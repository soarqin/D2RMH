/*
 * Copyright (c) 2021 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#include "plugin.h"

#include "d2r/d2rdefs.h"
#include "d2r/processmanager.h"
#include "ui/maprenderer.h"
#include "ui/window.h"
#include "cfg.h"
#include "util/util.h"

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include <windows.h>
#include <shlwapi.h>
#include <chrono>
#include <vector>
#include <queue>
#include <tuple>
#include <cstdint>

namespace plugin {

void PluginTextList::add(const char *text, uint32_t timeout, int fontSize) {
    auto wstr = util::utf8toucs4(text);
    for (auto ite = textList.begin(); ite != textList.end(); ++ite) {
        auto tout = util::getCurrTime() + std::chrono::milliseconds(timeout);
        if (ite->text == wstr && (fontSize < 0 || ite->fontSize == fontSize)) {
            if (fontSize < 0 && ite + 1 != textList.end()) {
                auto fsize = ite->fontSize;
                textList.erase(ite);
                textList.emplace_back(PluginText{wstr, tout, fsize});
            } else {
                ite->timeout = tout;
            }
            return;
        }
    }
    textList.emplace_back(PluginText{wstr, util::getCurrTime() + std::chrono::milliseconds(timeout),
                                     fontSize < 0 ? 0 : fontSize});
}

struct PluginCtx {
    sol::state lua;
    std::vector<std::tuple<std::chrono::milliseconds, sol::function, sol::function, bool>> plugins;
    using FuncPair = std::pair<std::chrono::steady_clock::time_point, size_t>;
    std::priority_queue<FuncPair, std::vector<FuncPair>, std::greater<>> timedRunning;
};

Plugin::Plugin(d2r::ProcessManager *process, ui::MapRenderer *renderer) :
    ctx_(new PluginCtx), d2rProcess_(process), mapRenderer_(renderer), window_(renderer->getRenderer().owner()) {
}

Plugin::~Plugin() {
    window_->clearHotkeys();
    delete ctx_;
}

void Plugin::load() {
    window_->clearHotkeys();
    delete ctx_;
    ctx_ = new PluginCtx;
    ctx_->lua = sol::state();
    auto &lua = ctx_->lua;
    lua.open_libraries();

    addCFunctions();
    WIN32_FIND_DATAW ffd = {};
    HANDLE find = FindFirstFileW(L"plugins\\*.lua", &ffd);
    if (find == INVALID_HANDLE_VALUE) {
        return;
    }
    do {
        if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            continue;
        }
        wchar_t filename[MAX_PATH] = L"plugins";
        PathAppendW(filename, ffd.cFileName);
        auto file = CreateFileW(filename,
                                GENERIC_READ,
                                FILE_SHARE_READ,
                                nullptr,
                                OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL,
                                nullptr);
        if (file == INVALID_HANDLE_VALUE) {
            continue;
        }
        std::string data;
        data.resize(ffd.nFileSizeLow);
        DWORD bytesRead;
        ReadFile(file, data.data(), ffd.nFileSizeLow, &bytesRead, nullptr);
        CloseHandle(file);
        sol::protected_function_result pfr = lua.safe_script(data, &sol::script_pass_on_error);
        if (!pfr.valid()) {
            sol::error err = pfr;
            window_->messageBox(util::utf8toucs4(err.what()).c_str(), filename, MB_ICONERROR);
        }
    } while (FindNextFileW(find, &ffd));
    FindClose(find);
}

void Plugin::run() {
    if (ctx_->timedRunning.empty()) { return; }
    auto now = util::getCurrTime();
    do {
        auto &pair = ctx_->timedRunning.top();
        if (now < pair.first) {
            break;
        }
        auto index = pair.second;
        auto &plugin = ctx_->plugins[index];
        auto dur = std::get<0>(plugin);
        auto next = pair.first + dur;
        if (next <= now) {
            next = now + dur;
        }
        ctx_->timedRunning.pop();
        ctx_->timedRunning.push(std::make_pair(next, index));
        if (std::get<3>(plugin)) {
            const auto &func = std::get<1>(plugin);
            if (func) { func(); }
        }
    } while (true);
}

void Plugin::onEnterGame() {
    for (auto &p: ctx_->plugins) {
        const auto &func = std::get<2>(p);
        if (func) { func(std::get<3>(p)); }
    }
}

void Plugin::addCFunctions() {
    auto &lua = ctx_->lua;
    lua.new_usertype<Cfg>(
        "Config",
        "show", &Cfg::show,
        "scale", &Cfg::scale
    );
    lua.new_usertype<d2r::Skill>(
        "Skill",
        "level", &d2r::Skill::skillLevel,
        "quantity", &d2r::Skill::quantity
    );
    lua.new_usertype<PluginTextList>(
        "TextList",
        "x", &PluginTextList::x,
        "y", &PluginTextList::y,
        "align", &PluginTextList::align,
        "valign", &PluginTextList::valign,
        "add", &PluginTextList::add,
        "clear", &PluginTextList::clear
    );
    lua.new_usertype<d2r::MapPlayer>(
        "Player",
        "act", &d2r::MapPlayer::act,
        "seed", &d2r::MapPlayer::seed,
        "difficulty", &d2r::MapPlayer::difficulty,
        "map", &d2r::MapPlayer::levelId,
        "pos_x", &d2r::MapPlayer::posX,
        "pos_y", &d2r::MapPlayer::posY,
        "name", &d2r::MapPlayer::name,
        "stats", &d2r::MapPlayer::stats
    );

    auto registerFunc = [this](uint32_t interval, const sol::function &func, const sol::function &toggleFunc, bool on) {
        auto index = ctx_->plugins.size();
        auto dur = std::chrono::milliseconds(interval);
        ctx_->plugins.emplace_back(dur, func, toggleFunc, on);
        ctx_->timedRunning.push(std::make_pair(util::getCurrTime() + dur, index));
        return index;
    };
    auto registerHotkeyForFunc = [this](const std::string &hotkey, int index) {
        window_->registerHotkey(hotkey, [this, index] {
            auto &on = std::get<3>(ctx_->plugins[index]);
            on = !on;
            const auto &func = std::get<2>(ctx_->plugins[index]);
            if (func) {
                func(on);
            }
        });
    };
    lua["register_plugin"] = sol::overload([registerFunc](uint32_t interval, const sol::function &func) {
                                               registerFunc(interval, func, sol::function(), true);
                                           },
                                           [registerFunc, registerHotkeyForFunc](const std::string &hotkey,
                                                                                 bool on,
                                                                                 uint32_t interval,
                                                                                 const sol::function &func) {
                                               auto index = registerFunc(interval, func, sol::function(), on);
                                               registerHotkeyForFunc(hotkey, index);
                                           },
                                           [registerFunc, registerHotkeyForFunc](const std::string &hotkey,
                                                                                 bool on,
                                                                                 uint32_t interval,
                                                                                 const sol::function &func,
                                                                                 const sol::function &toggleFunc) {
                                               auto index = registerFunc(interval, func, toggleFunc, on);
                                               registerHotkeyForFunc(hotkey, index);
                                           });
    lua["register_hotkey"] = [this](const std::string &hotkey, const sol::function &func) {
        window_->registerHotkey(hotkey, func);
    };
    lua["get_config"] = [] {
        return (Cfg *)cfg;
    };
    lua["flush_overlay"] = [this] {
        mapRenderer_->forceFlush();
    };
    lua["reload_config"] = [this] {
        loadCfg();
        auto &ren = mapRenderer_->getRenderer();
        ren.owner()->reloadConfig();
        if (cfg->fps > 0) {
            ren.limitFPS(cfg->fps);
        } else {
            render::Renderer::setSwapInterval(-cfg->fps);
        }
        mapRenderer_->reloadConfig();
    };

    lua["get_player"] = [this] {
        return d2rProcess_->currPlayer();
    };
    lua["get_skill"] = [this](uint16_t id) {
        return d2rProcess_->getSkill(id);
    };
    lua["kill_process"] = [this] {
        d2rProcess_->killProcess();
    };
    lua["create_text_list"] = [this](const std::string &str) {
        auto &res = mapRenderer_->getPluginText(str);
        res.textList.clear();
        return &res;
    };
    lua["remove_text_list"] = [this](const std::string &str) {
        mapRenderer_->removePluginText(str);
    };
    lua["message_box"] = [this](const std::string &str, uint32_t type) {
        window_->messageBox(util::utf8toucs4(str).c_str(), L"D2RMH", type);
    };
    lua["key_press"] = [this](const std::string &str) {
        auto hwnd = (HWND)d2rProcess_->hwnd();
        if (!hwnd) { return; }
        uint32_t mods;
        auto vkey = util::mapStringToVKey(str, mods);
        SendMessageA(hwnd, WM_KEYDOWN, vkey, 1);
        Sleep(5);
        SendMessageA(hwnd, WM_KEYUP, vkey, 0xC0000001u);
    };
    lua["mouse_move"] = [this](int x, int y) {
        auto hwnd = (HWND)d2rProcess_->hwnd();
        if (!hwnd) { return; }
        POINT pt = {x, y};
        ClientToScreen(hwnd, &pt);
        SetCursorPos(pt.x, pt.y);
    };
    lua["mouse_click"] = [this](int which) {
        auto hwnd = (HWND)d2rProcess_->hwnd();
        if (!hwnd) { return; }
        UINT msg, msg2;
        switch (which) {
        case 1:
            msg = WM_RBUTTONDOWN; msg2 = WM_RBUTTONUP;
            break;
        case 2:
            msg = WM_MBUTTONDOWN; msg2 = WM_MBUTTONUP;
            break;
        default:
            msg = WM_LBUTTONDOWN; msg2 = WM_LBUTTONUP;
            break;
        }
        SendMessageA(hwnd, msg, 0, 0);
        Sleep(5);
        SendMessageA(hwnd, msg2, 0, 0);
    };
    lua["delay"] = [this](uint32_t millisecs) {
        Sleep(millisecs);
    };
}

}

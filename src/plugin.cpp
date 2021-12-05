/*
 * Copyright (c) 2021 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#include "plugin.h"

#include "d2rprocess.h"
#include "util.h"

#include <sol/sol.hpp>

#include <windows.h>
#include <shlwapi.h>
#include <chrono>
#include <vector>
#include <queue>
#include <cstdint>

struct PluginCtx {
    sol::state lua;
    std::vector<std::pair<std::chrono::milliseconds, sol::function>> plugins;
    using FuncPair = std::pair<std::chrono::steady_clock::time_point, size_t>;
    std::priority_queue<FuncPair, std::vector<FuncPair>, std::greater<>> timedRunning;
};

Plugin::Plugin(D2RProcess *process): ctx_(new PluginCtx), d2rProcess_(process) {
}

Plugin::~Plugin() {
    delete ctx_;
}

void Plugin::load() {
    ctx_->lua = sol::state();
    ctx_->lua.open_libraries();
    ctx_->lua["register_plugin"] = [this](uint32_t interval, const sol::function &func) {
        auto index = ctx_->plugins.size();
        auto dur = std::chrono::milliseconds(interval);
        ctx_->plugins.emplace_back(dur, func);
        ctx_->timedRunning.push(std::make_pair(getCurrTime() + dur, index));
    };
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
        auto file = CreateFileW(filename, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (file == INVALID_HANDLE_VALUE) {
            continue;
        }
        std::string data;
        data.resize(ffd.nFileSizeLow);
        DWORD bytesRead;
        ReadFile(file, data.data(), ffd.nFileSizeLow, &bytesRead, nullptr);
        CloseHandle(file);
        ctx_->lua.script(data);
    } while (FindNextFileW(find, &ffd));
    FindClose(find);
}

void Plugin::run() {
    if (ctx_->timedRunning.empty()) { return; }
    auto now = getCurrTime();
    do {
        auto &pair = ctx_->timedRunning.top();
        if (now < pair.first) {
            break;
        }
        auto index = pair.second;
        auto &[dur, func] = ctx_->plugins[index];
        auto next = pair.first + dur;
        if (next <= now) {
            next = now + dur;
        }
        ctx_->timedRunning.pop();
        ctx_->timedRunning.push(std::make_pair(next, index));
        func();
    } while(true);
}

void Plugin::addCFunctions() {
    auto &lua = ctx_->lua;
    lua["get_player_stats"] = [this](sol::this_state s)->sol::object {
        sol::state_view lua(s);
        auto *plr = d2rProcess_->currPlayer();
        if (!plr) { return sol::make_object(lua, sol::lua_nil); }
        auto tbl = lua.create_table();
        int i = 1;
        for (auto v: plr->stats) {
            tbl[i++] = v;
        }
        return tbl;
    };
    lua["kill_process"] = [this] {
        d2rProcess_->killProcess();
    };
}

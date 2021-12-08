/*
 * Copyright (c) 2021 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#include "util.h"

#include <windows.h>
#include <map>

std::wstring utf8toucs4(const std::string &s) {
    std::wstring ws;
    wchar_t wc;
    size_t sz = s.length();
    for (size_t i = 0; i < sz;) {
        char c = s[i];
        if ((c & 0x80) == 0) {
            wc = c;
            ++i;
        } else if ((c & 0xE0) == 0xC0) {
            wc = (s[i] & 0x1F) << 6;
            wc |= (s[i + 1] & 0x3F);
            i += 2;
        } else if ((c & 0xF0) == 0xE0) {
            wc = (s[i] & 0xF) << 12;
            wc |= (s[i + 1] & 0x3F) << 6;
            wc |= (s[i + 2] & 0x3F);
            i += 3;
        } else if ((c & 0xF8) == 0xF0) {
            wc = (s[i] & 0x7) << 18;
            wc |= (s[i + 1] & 0x3F) << 12;
            wc |= (s[i + 2] & 0x3F) << 6;
            wc |= (s[i + 3] & 0x3F);
            i += 4;
        } else if ((c & 0xFC) == 0xF8) {
            wc = (s[i] & 0x3) << 24;
            wc |= (s[i] & 0x3F) << 18;
            wc |= (s[i] & 0x3F) << 12;
            wc |= (s[i] & 0x3F) << 6;
            wc |= (s[i] & 0x3F);
            i += 5;
        } else if ((c & 0xFE) == 0xFC) {
            wc = (s[i] & 0x1) << 30;
            wc |= (s[i] & 0x3F) << 24;
            wc |= (s[i] & 0x3F) << 18;
            wc |= (s[i] & 0x3F) << 12;
            wc |= (s[i] & 0x3F) << 6;
            wc |= (s[i] & 0x3F);
            i += 6;
        }
        ws += wc;
    }
    return ws;
}

std::vector<std::string> splitString(const std::string &str, char c) {
    typename std::string::size_type pos = 0;
    std::vector<std::string> result;
    while (true) {
        auto epos = str.find(c, pos);
        result.emplace_back(str.substr(pos, epos - pos));
        if (epos == std::string::npos) {
            break;
        }
        pos = epos + 1;
    }
    return result;
}

static std::chrono::steady_clock::time_point sCurrTime;
void updateTime() {
    sCurrTime = std::chrono::steady_clock::now();
}

std::chrono::steady_clock::time_point getCurrTime() {
    return sCurrTime;
}

uint32_t mapStringToVKey(const std::string &name, uint32_t &mods) {
    static const std::map<std::string, UINT> sModMap = {
        {"SHIFT", MOD_SHIFT},
        {"CONTROL", MOD_CONTROL},
        {"CTRL", MOD_CONTROL},
        {"ALT", MOD_ALT},
        {"WIN", MOD_WIN},
    };

    static const std::map<std::string, UINT> sVKeyMap = {
        {"LBUTTON", VK_LBUTTON},
        {"RBUTTON", VK_RBUTTON},
        {"CANCEL", VK_CANCEL},
        {"MBUTTON", VK_MBUTTON},
        {"XBUTTON1", VK_XBUTTON1},
        {"XBUTTON2", VK_XBUTTON2},
        {"BACK", VK_BACK},
        {"BACKSPACE", VK_BACK},
        {"TAB", VK_TAB},
        {"CLEAR", VK_CLEAR},
        {"RETURN", VK_RETURN},
        {"ENTER", VK_RETURN},
        {"PAUSE", VK_PAUSE},
        {"CAPITAL", VK_CAPITAL},
        {"CAPSLOCK", VK_CAPITAL},
        {"KANA", VK_KANA},
        {"HANGUL", VK_HANGUL},
        {"IME_ON", VK_IME_ON},
        {"JUNJA", VK_JUNJA},
        {"FINAL", VK_FINAL},
        {"HANJA", VK_HANJA},
        {"KANJI", VK_KANJI},
        {"IME_OFF", VK_IME_OFF},
        {"ESCAPE", VK_ESCAPE},
        {"CONVERT", VK_CONVERT},
        {"NONCONVERT", VK_NONCONVERT},
        {"ACCEPT", VK_ACCEPT},
        {"MODECHANGE", VK_MODECHANGE},
        {"SPACE", VK_SPACE},
        {"PRIOR", VK_PRIOR},
        {"NEXT", VK_NEXT},
        {"END", VK_END},
        {"HOME", VK_HOME},
        {"LEFT", VK_LEFT},
        {"UP", VK_UP},
        {"RIGHT", VK_RIGHT},
        {"DOWN", VK_DOWN},
        {"SELECT", VK_SELECT},
        {"PRINT", VK_PRINT},
        {"EXECUTE", VK_EXECUTE},
        {"SNAPSHOT", VK_SNAPSHOT},
        {"INSERT", VK_INSERT},
        {"DELETE", VK_DELETE},
        {"HELP", VK_HELP},
        {"0", 0x30},
        {"1", 0x31},
        {"2", 0x32},
        {"3", 0x33},
        {"4", 0x34},
        {"5", 0x35},
        {"6", 0x36},
        {"7", 0x37},
        {"8", 0x38},
        {"9", 0x39},
        {"A", 0x41},
        {"B", 0x42},
        {"C", 0x43},
        {"D", 0x44},
        {"E", 0x45},
        {"F", 0x46},
        {"G", 0x47},
        {"H", 0x48},
        {"I", 0x49},
        {"J", 0x4A},
        {"K", 0x4B},
        {"L", 0x4C},
        {"M", 0x4D},
        {"N", 0x4E},
        {"O", 0x4F},
        {"P", 0x50},
        {"Q", 0x51},
        {"R", 0x52},
        {"S", 0x53},
        {"T", 0x54},
        {"U", 0x55},
        {"V", 0x56},
        {"W", 0x57},
        {"X", 0x58},
        {"Y", 0x59},
        {"Z", 0x5A},
        {"APPS", VK_APPS},
        {"SLEEP", VK_SLEEP},
        {"NUMPAD0", VK_NUMPAD0},
        {"NUMPAD1", VK_NUMPAD1},
        {"NUMPAD2", VK_NUMPAD2},
        {"NUMPAD3", VK_NUMPAD3},
        {"NUMPAD4", VK_NUMPAD4},
        {"NUMPAD5", VK_NUMPAD5},
        {"NUMPAD6", VK_NUMPAD6},
        {"NUMPAD7", VK_NUMPAD7},
        {"NUMPAD8", VK_NUMPAD8},
        {"NUMPAD9", VK_NUMPAD9},
        {"NUM0", VK_NUMPAD0},
        {"NUM1", VK_NUMPAD1},
        {"NUM2", VK_NUMPAD2},
        {"NUM3", VK_NUMPAD3},
        {"NUM4", VK_NUMPAD4},
        {"NUM5", VK_NUMPAD5},
        {"NUM6", VK_NUMPAD6},
        {"NUM7", VK_NUMPAD7},
        {"NUM8", VK_NUMPAD8},
        {"NUM9", VK_NUMPAD9},
        {"MULTIPLY", VK_MULTIPLY},
        {"ADD", VK_ADD},
        {"SUBTRACT", VK_SUBTRACT},
        {"MINUS", VK_SUBTRACT},
        {"DECIMAL", VK_DECIMAL},
        {"DIVIDE", VK_DIVIDE},
        {"F1", VK_F1},
        {"F2", VK_F2},
        {"F3", VK_F3},
        {"F4", VK_F4},
        {"F5", VK_F5},
        {"F6", VK_F6},
        {"F7", VK_F7},
        {"F8", VK_F8},
        {"F9", VK_F9},
        {"F10", VK_F10},
        {"F11", VK_F11},
        {"F12", VK_F12},
        {"F13", VK_F13},
        {"F14", VK_F14},
        {"F15", VK_F15},
        {"F16", VK_F16},
        {"F17", VK_F17},
        {"F18", VK_F18},
        {"F19", VK_F19},
        {"F20", VK_F20},
        {"F21", VK_F21},
        {"F22", VK_F22},
        {"F23", VK_F23},
        {"F24", VK_F24},
        {"NUMLOCK", VK_NUMLOCK},
        {"SCROLL", VK_SCROLL},
        {"LSHIFT", VK_LSHIFT},
        {"RSHIFT", VK_RSHIFT},
        {"LCONTROL", VK_LCONTROL},
        {"RCONTROL", VK_RCONTROL},
        {"LMENU", VK_LMENU},
        {"RMENU", VK_RMENU},
        {"BROWSER_BACK", VK_BROWSER_BACK},
        {"BROWSER_FORWARD", VK_BROWSER_FORWARD},
        {"BROWSER_REFRESH", VK_BROWSER_REFRESH},
        {"BROWSER_STOP", VK_BROWSER_STOP},
        {"BROWSER_SEARCH", VK_BROWSER_SEARCH},
        {"BROWSER_FAVORITES", VK_BROWSER_FAVORITES},
        {"BROWSER_HOME", VK_BROWSER_HOME},
        {"VOLUME_MUTE", VK_VOLUME_MUTE},
        {"VOLUME_DOWN", VK_VOLUME_DOWN},
        {"VOLUME_UP", VK_VOLUME_UP},
        {"MEDIA_NEXT_TRACK", VK_MEDIA_NEXT_TRACK},
        {"MEDIA_PREV_TRACK", VK_MEDIA_PREV_TRACK},
        {"MEDIA_STOP", VK_MEDIA_STOP},
        {"MEDIA_PLAY_PAUSE", VK_MEDIA_PLAY_PAUSE},
        {"LAUNCH_MAIL", VK_LAUNCH_MAIL},
        {"LAUNCH_MEDIA_SELECT", VK_LAUNCH_MEDIA_SELECT},
        {"LAUNCH_APP1", VK_LAUNCH_APP1},
        {"LAUNCH_APP2", VK_LAUNCH_APP2},
        {"OEM_PLUS", VK_OEM_PLUS},
        {"OEM_COMMA", VK_OEM_COMMA},
        {"OEM_MINUS", VK_OEM_MINUS},
        {"OEM_PERIOD", VK_OEM_PERIOD},
        {";", VK_OEM_1},
        {"/", VK_OEM_2},
        {"~", VK_OEM_3},
        {"[", VK_OEM_4},
        {"\\", VK_OEM_5},
        {"]", VK_OEM_6},
        {"'", VK_OEM_7},
        {"PROCESSKEY", VK_PROCESSKEY},
        {"ATTN", VK_ATTN},
        {"CRSEL", VK_CRSEL},
        {"EXSEL", VK_EXSEL},
        {"EREOF", VK_EREOF},
        {"PLAY", VK_PLAY},
        {"ZOOM", VK_ZOOM},
        {"PA1", VK_PA1},
        {"OEM_CLEAR", VK_OEM_CLEAR},
    };

    mods = 0;
    std::string str = name;
    for (auto &c: str) { c = std::toupper(c); }
    auto sl = splitString(str, '+');
    if (sl.empty()) {
        return 0;
    }
    auto ite = sVKeyMap.find(sl.back());
    if (ite == sVKeyMap.end()) {
        return 0;
    }
    sl.pop_back();
    for (const auto &s: sl) {
        auto ite2 = sModMap.find(s);
        if (ite2 == sModMap.end()) {
            return 0;
        }
        mods |= ite2->second;
    }
    return ite->second;
}

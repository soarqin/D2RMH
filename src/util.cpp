/*
 * Copyright (c) 2021 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#include "util.h"

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

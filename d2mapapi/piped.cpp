/*
 * Copyright (c) 2021 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#include "d2map.h"
#include "session.h"

#include <windows.h>
#include <unordered_map>
#include <cstdint>

enum {
    SessionsCacheSize = 8,
};

int wmain(int argc, wchar_t *argv[]) {
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    if ((hStdout == INVALID_HANDLE_VALUE) || (hStdin == INVALID_HANDLE_VALUE)) {
        ExitProcess(1);
    }

    const auto *errstr = argc > 1 ? d2mapapi::d2Init(argv[1]) : "Usage: d2mapapi_piped <D2 Game Path>";
    if (errstr) {
        do {
            HKEY key;
            if (RegOpenKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\Blizzard Entertainment\\Diablo II", 0, KEY_READ, &key)
                == ERROR_SUCCESS) {
                wchar_t path[MAX_PATH];
                DWORD pathSize = sizeof(path);
                if (RegQueryValueExW(key, L"InstallPath", nullptr, nullptr, LPBYTE(path), &pathSize) == ERROR_SUCCESS) {
                    errstr = d2mapapi::d2Init(path);
                    if (!errstr) {
                        RegCloseKey(key);
                        break;
                    }
                }
                RegCloseKey(key);
            }
            MessageBoxA(nullptr, errstr, "d2mapapi", MB_OK | MB_ICONERROR);
            return -1;
        } while (false);
    }

    std::unordered_map<uint64_t, std::unique_ptr<d2mapapi::Session>> sessions;
    std::vector<uint64_t> sessionsOrder;
    for (;;) {
        struct Req {
            uint32_t seed;
            uint32_t difficulty;
            uint32_t levelId;
        };
        Req req = {};
        DWORD bytesRead, written;
        if (!ReadFile(hStdin, &req, sizeof(uint32_t) * 3, &bytesRead, nullptr)) {
            break;
        }
        auto key = uint64_t(req.seed) | (uint64_t(req.difficulty) << 32);
        auto &session = sessions[key];
        sessionsOrder.emplace_back(key);
        if (!session) {
            session = std::make_unique<d2mapapi::Session>();
            session->update(req.seed, req.difficulty);
            if (sessionsOrder.size() > SessionsCacheSize) {
                auto oldKey = sessionsOrder[0];
                sessionsOrder.erase(sessionsOrder.begin());
                sessions.erase(oldKey);
            }
        }
        const auto *map = session->getMap(req.levelId);
        std::string str;
        if (map) {
            str = map->encode();
        } else {
            str = R"({"error":"Invalid map id!"})";
        }
        auto sz = uint32_t(str.size());
        if (!WriteFile(hStdout, &sz, sizeof(uint32_t), &written, nullptr) ||
            !WriteFile(hStdout, str.c_str(), sz, &written, nullptr)) {
            break;
        }
    }
    return 0;
}

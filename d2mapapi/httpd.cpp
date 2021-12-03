/*
 * Copyright (c) 2021 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#include "simphttp.h"

#include "d2map.h"
#include "session.h"

#include <windows.h>
#include <unordered_map>
#include <memory>

enum {
    SessionsCacheSize = 8,
};

int wmain(int argc, wchar_t *argv[]) {
    const auto *errstr = argc > 1 ? d2mapapi::d2Init(argv[1]) : "Usage: d2mapapi_piped <D2 Game Path>";
    if (errstr) {
        do {
            HKEY key;
            if (RegOpenKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\Blizzard Entertainment\\Diablo II", 0, KEY_READ, &key) == ERROR_SUCCESS) {
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
    simphttp::Server server([&sessions, &sessionsOrder](auto &req, auto &res) {
        uint32_t seed = 0;
        uint8_t difficulty = 0;
        uint32_t levelId = 0;
        int indentation = 0;
        bool success = false;
        const char *url = req.url.c_str();
        do {
            if (*url != '/') break;
            ++url;
            char *next;
            seed = uint32_t(strtoul(url, &next, 0));
            if (url == next || *next != '/') break;

            url = next + 1;
            difficulty = uint32_t(strtoul(url, &next, 0));
            if (url == next || *next != '/') break;

            url = next + 1;
            levelId = uint32_t(strtoul(url, &next, 0));
            if (url == next || (*next != '/' && *next != 0)) break;

            if (*next == '/') {
                url = next + 1;
                indentation = int(strtoul(url, &next, 0));
                if (url == next && (*next == '/' || *next == 0)) {
                    indentation = 0;
                } else if (*next != '/' && *next != 0) {
                    break;
                }
            }
            success = true;
        } while (false);
        res.setStatus(200);
        res.setHeader("Connection", "keep-alive");
        res.setHeader("Content-Type", "application/json");
        if (!success) {
            const std::string errstr = R"({"error":"Invalid parameters!")";
            res.setHeader("Content-Length", std::to_string(errstr.size()));
            res.end(errstr);
            return;
        }
        auto key = uint64_t(seed) | (uint64_t(difficulty) << 32);
        auto &session = sessions[key];
        sessionsOrder.emplace_back(key);
        if (!session) {
            session = std::make_unique<d2mapapi::Session>();
            session->update(seed, difficulty);
            if (sessionsOrder.size() > SessionsCacheSize) {
                auto oldKey = sessionsOrder[0];
                sessionsOrder.erase(sessionsOrder.begin());
                sessions.erase(oldKey);
            }
        }
        const auto *map = session->getMap(levelId);
        if (map) {
            auto str = map->encode(indentation);
            res.setHeader("Content-Length", std::to_string(str.size()));
            res.end(str);
        } else {
            const std::string errstr = R"({"error":"Invalid map id!"})";
            res.setHeader("Content-Length", std::to_string(errstr.size()));
            res.end(errstr);
        }
    });
    server.listen("::", 8000);
    return 0;
}

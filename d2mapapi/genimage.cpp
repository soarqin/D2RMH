/*
 * Copyright (c) 2021 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#include "d2map.h"
#include "session.h"

#define STBIW_WINDOWS_UTF8
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

#include <windows.h>
#include <shlwapi.h>
#include <cstdio>
#include <cstdint>
#include <cwchar>

int wmain(int argc, wchar_t *argv[]) {
    if (argc < 5) {
        fprintf(stderr, "Usage: d2mapapi_gen_image [D2 Game Path] <seed> <difficulty> <map_id> <image filename>\n");
        return -1;
    }
    const wchar_t *wfilename = argv[argc > 5 ? 5 : 4];
    const wchar_t *ext = StrRChrW(wfilename, nullptr, L'.');
    int filetype = -1;
    if (ext) {
        if (!StrCmpIW(ext, L".png")) {
            filetype = 0;
        } else if (!StrCmpIW(ext, L".bmp")) {
            filetype = 1;
        } else if (!StrCmpIW(ext, L".tga")) {
            filetype = 2;
        } else if (!StrCmpIW(ext, L".jpg")) {
            filetype = 3;
        }
    }
    if (filetype < 0) {
        fprintf(stderr, "Supported file extension: png, bmp, tga, jpg\n");
        return -1;
    }
    const auto *errstr = argc > 5 ? d2mapapi::d2Init(argv[1]) : nullptr;
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
            fprintf(stderr, "[d2mapapi_gen_image v" D2MAPAPI_VERSION "] %s\n", errstr);
            return -1;
        } while (false);
    }
    std::uint32_t seed, difficulty, mapId;
    seed = std::uint32_t(wcstoul(argv[argc > 5 ? 2 : 1], nullptr, 0));
    difficulty = std::uint32_t(wcstoul(argv[argc > 5 ? 3 : 2], nullptr, 0));
    mapId = std::uint32_t(wcstoul(argv[argc > 5 ? 4 : 3], nullptr, 0));
    auto sess = std::make_unique<d2mapapi::Session>();
    sess->update(seed, difficulty);
    auto *map = sess->getMap(mapId);
    auto str = map->encode();
    auto *collmap = new d2mapapi::CollisionMap(str);

    auto [x0, y0, x1, y1] = collmap->crop;
    std::vector<uint8_t> vec;
    collmap->extractCellData<uint8_t>(vec, 0, 128);

    stbi_write_png_compression_level = 9;
    char filename[1024];
    stbiw_convert_wchar_to_utf8(filename, 1024, wfilename);
    switch (filetype) {
    case 0:
        stbi_write_png(filename, x1 - x0, y1 - y0, 1, vec.data(), x1 - x0);
        break;
    case 1:
        stbi_write_bmp(filename, x1 - x0, y1 - y0, 1, vec.data());
        break;
    case 2:
        stbi_write_tga(filename, x1 - x0, y1 - y0, 1, vec.data());
        break;
    case 3:
        stbi_write_jpg(filename, x1 - x0, y1 - y0, 1, vec.data(), 90);
        break;
    default:
        break;
    }
    return 0;
}

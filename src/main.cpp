/*
 * Copyright (c) 2021 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#include "cfg.h"

#include "renderer.h"
#include "window.h"
#include "maprenderer.h"
#include "util.h"
#include "os_structs.h"

#include "d2map.h"

int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    osInit();
    HANDLE evt = CreateEventW(nullptr, FALSE, FALSE, L"Global\\D2RMH_EVENT" );
    if (!evt) {
        return -1;
    }
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        CloseHandle(evt);
        MessageBoxW(nullptr, L"A D2RMH instance is already running!", L"D2RMH", MB_OK | MB_ICONERROR);
        return 0;
    }
    loadCfg();
    const auto *errstr = d2MapInit(cfg->d2Path.c_str());
    if (errstr) {
        do {
            HKEY key;
            if (RegOpenKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\Blizzard Entertainment\\Diablo II", 0, KEY_READ, &key) == ERROR_SUCCESS) {
                wchar_t path[MAX_PATH];
                DWORD pathSize = sizeof(path);
                if (RegQueryValueExW(key, L"InstallPath", nullptr, nullptr, LPBYTE(path), &pathSize) == ERROR_SUCCESS) {
                    errstr = d2MapInit(path);
                    if (!errstr) {
                        RegCloseKey(key);
                        break;
                    }
                }
                RegCloseKey(key);
            }
            MessageBoxA(nullptr, errstr, "D2RMH", MB_OK | MB_ICONERROR);
            return 0;
        } while (false);
    }
    loadData();

    Window wnd(100, 100, 500, 400);
    Renderer renderer(&wnd);
    if (cfg->fps > 0) {
        renderer.limitFPS(cfg->fps);
    } else {
        Renderer::setSwapInterval(-cfg->fps);
    }

    MapRenderer map(renderer);
    wnd.enableTrayMenu(true, (const wchar_t*)1, L"D2RMH", L"D2RMH is running.\nYou can close it from tray-icon popup menu.", L"D2RMH");
    wnd.addAboutMenu();
    wnd.addTrayMenuItem(L"Reload Config", -1, 0, [&wnd, &renderer, &map]() {
        loadCfg();
        wnd.reloadConfig();
        if (cfg->fps > 0) {
            renderer.limitFPS(cfg->fps);
        } else {
            Renderer::setSwapInterval(-cfg->fps);
        }
        map.reloadConfig();
    });
    wnd.addTrayMenuItem(L"Quit", -1, 0, [&wnd]() { wnd.quit(); });
    while (wnd.run()) {
        updateTime();
        renderer.prepare();
        map.update();
        renderer.begin();
        map.render();
        renderer.end();
    }
    return 0;
}

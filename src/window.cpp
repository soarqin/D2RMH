/*
 * Copyright (c) 2021 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#include "window.h"

#include "cfg.h"

#include <windows.h>
#include <stdexcept>
#include <map>

EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)
#define WM_TRAY_CALLBACK_MESSAGE (WM_USER + 1)

struct MenuItem {
    UINT id = 0;
    HMENU submenu = nullptr;
    std::function<void()> cb;
};

struct WindowCtx {
    HWND hwnd = nullptr;
    std::function<void(int, int)> sizeCB;
    NOTIFYICONDATAW nid = {};
    HMENU trayMenu = nullptr;
    bool running = false;
    UINT userMsg = WM_USER + 1;
    std::map<UINT, MenuItem> trayMenuInfo;
};

static LRESULT CALLBACK wndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_DESTROY: {
        auto *ctx = ((Window*)GetWindowLongPtr(hwnd, GWLP_USERDATA))->ctx();
        if (ctx->nid.cbSize) {
            Shell_NotifyIconW(NIM_DELETE, &ctx->nid);
        }
        ctx->running = false;
        return 0;
    }
    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;
    case WM_SIZE: {
        auto *wnd = (Window*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
        if (!wnd) { break; }
        auto *ctx = wnd->ctx();
        if (ctx->sizeCB) { ctx->sizeCB(LOWORD(lParam), HIWORD(lParam)); }
        break;
    }
    case WM_TRAY_CALLBACK_MESSAGE:
        if (lParam == WM_RBUTTONUP) {
            POINT p;
            GetCursorPos(&p);
            SetForegroundWindow(hwnd);
            auto *ctx = ((Window*)GetWindowLongPtr(hwnd, GWLP_USERDATA))->ctx();
            WORD cmd = TrackPopupMenu(ctx->trayMenu,
                                      TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY,
                                      p.x, p.y, 0, hwnd, nullptr);
            SendMessage(hwnd, WM_COMMAND, cmd, 0);
            return 0;
        }
        break;
    case WM_COMMAND: {
        auto *ctx = ((Window*)GetWindowLongPtr(hwnd, GWLP_USERDATA))->ctx();
        auto ite = ctx->trayMenuInfo.find(wParam);
        if (ite == ctx->trayMenuInfo.end()) {
            break;
        }
        ite->second.cb();
        return 0;
    }
    default:
        break;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

Window::Window(int x, int y, int width, int height): ctx_(new WindowCtx) {
    auto inst = HINST_THISCOMPONENT;
    auto icon = LoadIconW(inst, MAKEINTRESOURCEW(1));

    WNDCLASSEX wcx = {};

    wcx.cbSize = sizeof(wcx);
    wcx.style = CS_HREDRAW | CS_VREDRAW;
    wcx.lpfnWndProc = wndProc;
    wcx.hInstance = inst;
    wcx.hIcon = icon;
    wcx.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcx.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wcx.lpszClassName = "D2RMH";
    wcx.hIconSm = icon;
    RegisterClassEx(&wcx);
    DWORD exStyle = WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT | WS_EX_LAYERED;
    DWORD style = WS_POPUP;
    RECT rc = {x, y, x + width, y + height};
    AdjustWindowRectEx(&rc, style, FALSE, exStyle);

    ctx_->hwnd = CreateWindowExW(exStyle, L"D2RMH", L"D2RMH",
        style, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, inst, this);
    if (ctx_->hwnd == nullptr) {
        throw std::runtime_error("Unable to create window");
    }
    SetWindowLongPtr(ctx_->hwnd, GWLP_USERDATA, (LONG_PTR)this);
    SetLayeredWindowAttributes(ctx_->hwnd, 0, cfg->alpha, LWA_COLORKEY | LWA_ALPHA);
    ShowWindow(ctx_->hwnd, SW_SHOW);
    ctx_->running = true;
}

Window::~Window() {
    if (IsWindow(ctx_->hwnd)) {
        DestroyWindow(ctx_->hwnd);
    }
    delete ctx_;
}

void Window::enableTrayMenu(bool enable, const wchar_t *icon, const wchar_t *tip, const wchar_t *info, const wchar_t *infoTitle) {
    if (!enable) {
        if (ctx_->nid.cbSize) {
            Shell_NotifyIconW(NIM_DELETE, &ctx_->nid);
        }
        return;
    }
    HICON hicon;
    if (IS_INTRESOURCE(icon)) {
        hicon = LoadIconW(HINST_THISCOMPONENT, icon);
    } else {
        ExtractIconExW(icon, 0, nullptr, &hicon, 1);
    }
    memset(&ctx_->nid, 0, sizeof(ctx_->nid));
    ctx_->nid.cbSize = NOTIFYICONDATAW_V3_SIZE;
    ctx_->nid.uVersion = 3;
    ctx_->nid.hWnd = ctx_->hwnd;
    ctx_->nid.uID = 0;
    ctx_->nid.uFlags = NIF_ICON | NIF_MESSAGE;
    if (tip) {
        ctx_->nid.uFlags |= NIF_TIP;
        lstrcpyW(ctx_->nid.szTip, tip);
    }
    if (info && infoTitle) {
        ctx_->nid.uFlags |= NIF_INFO;
        lstrcpyW(ctx_->nid.szInfo, info);
        lstrcpyW(ctx_->nid.szInfoTitle, infoTitle);
        ctx_->nid.dwInfoFlags = NIIF_NONE | NIIF_NOSOUND
#ifdef NIIF_RESPECT_QUIET_TIME
            | NIIF_RESPECT_QUIET_TIME
#endif
            ;
    }
    ctx_->nid.hIcon = hicon;
    ctx_->nid.uCallbackMessage = WM_TRAY_CALLBACK_MESSAGE;
    Shell_NotifyIconW(NIM_ADD, &ctx_->nid);

    ctx_->trayMenu = CreatePopupMenu();
}

int Window::addTrayMenuItem(const wchar_t *name, int parent, unsigned flags, const std::function<void()> &cb) {
    auto &st = ctx_->trayMenuInfo[ctx_->userMsg];
    HMENU parentMenu;
    if (parent < 0) {
        parentMenu = ctx_->trayMenu;
    } else {
        auto ite = ctx_->trayMenuInfo.find(parent);
        if (ite == ctx_->trayMenuInfo.end() || !ite->second.submenu) {
            return -1;
        }
        parentMenu = ite->second.submenu;
    }
    if (lstrcmpW(name, L"-") == 0) {
        InsertMenuW(parentMenu, ctx_->userMsg, MF_SEPARATOR, TRUE, L"");
    } else {
        MENUITEMINFOW item = {};
        item.cbSize = sizeof(MENUITEMINFOW);
        item.fMask = MIIM_ID | MIIM_TYPE | MIIM_STATE | MIIM_DATA;
        item.fType = MFT_STRING;
        item.fState = 0;
        if (flags & TRAYMENU_SUBMENU) {
            item.fMask |= MIIM_SUBMENU;
            item.hSubMenu = CreatePopupMenu();
            st.submenu = item.hSubMenu;
        }
        if (flags & TRAYMENU_DISABLED) {
            item.fState |= MFS_DISABLED;
        }
        if (flags & TRAYMENU_CHECKED) {
            item.fState |= MFS_CHECKED;
        }
        item.wID = ctx_->userMsg;
        item.dwTypeData = (LPWSTR)name;
        item.dwItemData = (ULONG_PTR)&st;
        InsertMenuItemW(parentMenu, ctx_->userMsg, TRUE, &item);
    }
    st.id = ctx_->userMsg++;
    st.cb = cb;
    return st.id;
}

bool Window::run() const {
    if (!ctx_->running) {
        return false;
    }
    MSG msg = { };
    while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return true;
}

bool Window::running() const {
    return ctx_->running;
}

void Window::quit() {
    DestroyWindow(ctx_->hwnd);
    PostQuitMessage(0);
}

void Window::getDimension(int &w, int &h) {
    RECT rc;
    GetClientRect(ctx_->hwnd, &rc);
    w = rc.right - rc.left;
    h = rc.bottom - rc.top;
}

void *Window::hwnd() {
    return (void*)ctx_->hwnd;
}

void Window::move(int x, int y, int w, int h) {
    auto exStyle = GetWindowLong(ctx_->hwnd, GWL_EXSTYLE);
    auto style = GetWindowLong(ctx_->hwnd, GWL_STYLE);
    RECT rc = {x, y, x + w, y + h};
    AdjustWindowRectEx(&rc, style, FALSE, exStyle);
    MoveWindow(ctx_->hwnd, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, FALSE);
}

void Window::setSizeCallback(const std::function<void(int, int)> &cb) {
    ctx_->sizeCB = cb;
}
/*
 * Copyright (c) 2021 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#if defined(_MSC_VER)
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

#include "window.h"

#include "util/util.h"

#include <windows.h>
#include <versionhelpers.h>
#include <dwmapi.h>
#include <commctrl.h>
#include <stdexcept>
#include <map>
#include <tuple>
#include <string>

EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)
#define WM_TRAY_CALLBACK_MESSAGE (WM_USER + 1)

namespace ui {

struct MenuItem {
    UINT id = 0;
    HMENU submenu = nullptr;

    std::wstring name;
    int parent = 0;
    unsigned flags = 0;
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
    struct {
        bool enabled = false;
        const wchar_t *icon = nullptr;
        std::wstring tip, info, infoTitle;
    } trayMenuBase;

    std::map<int, std::tuple<UINT, UINT, std::function<void()>>> hotkeys;
    bool hotkeysEnabled = false;
};

static bool updateFramebufferTransparency(HWND hwnd) {
    BOOL composition, opaque;
    DWORD color;

    if (!IsWindowsVistaOrGreater())
        return false;

    if (FAILED(DwmIsCompositionEnabled(&composition)) || !composition)
        return false;

    if (IsWindows8OrGreater() ||
        (SUCCEEDED(DwmGetColorizationColor(&color, &opaque)) && !opaque)) {
        HRGN region = CreateRectRgn(0, 0, -1, -1);
        DWM_BLURBEHIND bb = {0};
        bb.dwFlags = DWM_BB_ENABLE | DWM_BB_BLURREGION;
        bb.hRgnBlur = region;
        bb.fEnable = TRUE;

        DwmEnableBlurBehindWindow(hwnd, &bb);
        MARGINS m = {-1};
        DeleteObject(region);
    } else {
        DWM_BLURBEHIND bb = {0};
        bb.dwFlags = DWM_BB_ENABLE;
        DwmEnableBlurBehindWindow(hwnd, &bb);
    }
    return true;
}

static LRESULT CALLBACK wndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static UINT taskbarRestartMsg;
    switch (uMsg) {
    case WM_CREATE: {
        taskbarRestartMsg = RegisterWindowMessageW(L"TaskbarCreated");
        break;
    }
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
    case WM_DWMCOMPOSITIONCHANGED:
    case WM_DWMCOLORIZATIONCOLORCHANGED: {
        auto *ctx = ((Window*)GetWindowLongPtr(hwnd, GWLP_USERDATA))->ctx();
        updateFramebufferTransparency(ctx->hwnd);
        return 0;
    }
    case WM_HOTKEY: {
        auto id = int(wParam);
        if (id < 0) { break; }
        auto *ctx = ((Window*)GetWindowLongPtr(hwnd, GWLP_USERDATA))->ctx();
        auto ite = ctx->hotkeys.find(id);
        if (ite == ctx->hotkeys.end()) { break; }
        const auto &func = std::get<2>(ite->second);
        if (func) { func(); }
        return 0;
    }
    default:
        if (uMsg == taskbarRestartMsg) {
            auto *win = (Window*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
            auto *ctx = win->ctx();
            if (!ctx->trayMenuBase.enabled) {
                break;
            }
            auto m = ctx->trayMenuInfo;
            win->destroyTrayMenu();
            win->enableTrayMenu(true, ctx->trayMenuBase.icon, ctx->trayMenuBase.tip.c_str(), ctx->trayMenuBase.info.c_str(), ctx->trayMenuBase.infoTitle.c_str());
            for (auto &p: m) {
                win->addTrayMenuItem(p.second.name.c_str(), p.second.parent, p.second.flags, p.second.cb);
            }
        }
        break;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

Window::Window(int x, int y, int width, int height): ctx_(new WindowCtx) {
    INITCOMMONCONTROLSEX iccex = { sizeof(INITCOMMONCONTROLSEX) };
    InitCommonControlsEx(&iccex);
    auto inst = HINST_THISCOMPONENT;
    auto icon = LoadIconW(inst, MAKEINTRESOURCEW(1));

    WNDCLASSEXW wcx = {};

    wcx.cbSize = sizeof(wcx);
    wcx.style = CS_HREDRAW | CS_VREDRAW;
    wcx.lpfnWndProc = wndProc;
    wcx.hInstance = inst;
    wcx.hIcon = icon;
    wcx.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcx.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wcx.lpszClassName = L"D2RMH";
    wcx.hIconSm = icon;
    RegisterClassExW(&wcx);
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
    SetLayeredWindowAttributes(ctx_->hwnd, 0, 0, 0);
    updateFramebufferTransparency(ctx_->hwnd);
    ShowWindow(ctx_->hwnd, SW_SHOW);
    ctx_->running = true;
}

Window::~Window() {
    clearHotkeys();
    if (IsWindow(ctx_->hwnd)) {
        DestroyWindow(ctx_->hwnd);
    }
    delete ctx_;
}

INT_PTR CALLBACK dialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CLOSE:
        EndDialog(hWnd, TRUE);
        break;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
            EndDialog(hWnd, TRUE);
            break;
        }
        break;
    case WM_NOTIFY: {
        auto *nmhdr = LPNMHDR(lParam);
        switch (nmhdr->code) {
        case NM_CLICK:
        case NM_RETURN: {
            if (nmhdr->idFrom == 1001) {
                auto pNMLink = PNMLINK(lParam);
                auto item = pNMLink->item;
                if (item.iLink == 0) {
                    ShellExecuteW(nullptr, L"open", item.szUrl, nullptr, nullptr, SW_SHOW);
                }
            }
            break;
        }
        }
        break;
    }
    case WM_INITDIALOG: {
        RECT rc, rc2;
        GetWindowRect(GetDesktopWindow(), &rc);
        GetWindowRect(hWnd, &rc2);
        auto width = rc2.right - rc2.left;
        auto height = rc2.bottom - rc2.top;
        MoveWindow(hWnd, (rc.right + rc.left - width) / 2, (rc.top + rc.bottom - height) / 2, width, height, FALSE);
        auto caption = GetDlgItem(hWnd, 1000);
        SetWindowTextW(caption, L"D2RMH " VERSION_STRING_FULL);
        auto font = HFONT(SendMessage(caption, WM_GETFONT, 0, 0));
        LOGFONTW lf = {};
        GetObjectW(font, sizeof(lf), &lf);
        lf.lfHeight *= 2; lf.lfWidth *= 2;
        auto font2 = CreateFontIndirectW(&lf);
        SendMessage(caption, WM_SETFONT, (WPARAM)font2, TRUE);
        break;
    }
    case WM_CTLCOLORSTATIC: {
        if (GetDlgCtrlID((HWND)lParam) == 1000) {
            SetTextColor((HDC)wParam, 0x881111);
            SetBkMode((HDC)wParam, TRANSPARENT);
            return (INT_PTR)::GetStockObject(NULL_BRUSH);
        }
        return FALSE;
    }
    default:
        return FALSE;
    }

    return TRUE;
}

void Window::enableTrayMenu(bool enable, const wchar_t *icon, const wchar_t *tip, const wchar_t *info, const wchar_t *infoTitle) {
    ctx_->trayMenuBase = {enable, icon, tip ? tip : L"", info ? info : L"", infoTitle ? infoTitle : L""};
    if (!enable) {
        if (ctx_->nid.cbSize) {
            Shell_NotifyIconW(NIM_DELETE, &ctx_->nid);
            memset(&ctx_->nid, 0, sizeof(ctx_->nid));
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
    if (tip && tip[0]) {
        ctx_->nid.uFlags |= NIF_TIP;
        lstrcpyW(ctx_->nid.szTip, tip);
    }
    if (info && info[0] && infoTitle && infoTitle[0]) {
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

void Window::addAboutMenu() {
    addTrayMenuItem(L"About", -1, 0, [] {
        DialogBox(HINST_THISCOMPONENT, MAKEINTRESOURCE(101), nullptr, dialogProc);
    });
}

int Window::addTrayMenuItem(const wchar_t *name, int parent, unsigned flags, const std::function<void()> &cb) {
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
    auto &st = ctx_->trayMenuInfo[ctx_->userMsg];
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

    st.name = name;
    st.parent = parent;
    st.flags = flags;
    st.cb = cb;
    return st.id;
}

void Window::destroyTrayMenu() {
    if (ctx_->nid.cbSize) {
        Shell_NotifyIconW(NIM_DELETE, &ctx_->nid);
        memset(&ctx_->nid, 0, sizeof(ctx_->nid));
    }
    ctx_->trayMenuBase.enabled = false;
    DestroyMenu(ctx_->trayMenu);
    ctx_->trayMenu = nullptr;
    ctx_->trayMenuInfo.clear();
    ctx_->userMsg = WM_USER + 1;
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

void Window::move(int x, int y, int w, int h) {
    auto exStyle = GetWindowLong(ctx_->hwnd, GWL_EXSTYLE);
    auto style = GetWindowLong(ctx_->hwnd, GWL_STYLE);
    RECT rc = {x, y, x + w, y + h};
    AdjustWindowRectEx(&rc, style, FALSE, exStyle);
    MoveWindow(ctx_->hwnd, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, FALSE);
}

void Window::reloadConfig() {
    clearHotkeys();
    SetLayeredWindowAttributes(ctx_->hwnd, 0, 0, 0);
    updateFramebufferTransparency(ctx_->hwnd);
}

void Window::enableHotkeys(bool enable) {
    if (enable == ctx_->hotkeysEnabled) { return; }
    ctx_->hotkeysEnabled = enable;
    if (enable) {
        for (auto &p: ctx_->hotkeys) {
            RegisterHotKey(ctx_->hwnd, p.first, std::get<0>(p.second) | MOD_NOREPEAT, std::get<1>(p.second));
        }
    } else {
        for (auto &p: ctx_->hotkeys) {
            UnregisterHotKey(ctx_->hwnd, p.first);
        }
    }
}

void Window::registerHotkey(const std::string &name, const std::function<void()> &cb) {
    if (!cb) { return; }
    UINT mods;
    UINT vkey = util::mapStringToVKey(name, mods);
    if (!vkey) { return; }
    for (int nid = 1; nid < 0xC000; ++nid) {
        if (ctx_->hotkeys.find(nid) != ctx_->hotkeys.end()) { continue; }
        ctx_->hotkeys[nid] = std::make_tuple(mods, vkey, cb);
        if (ctx_->hotkeysEnabled) {
            RegisterHotKey(ctx_->hwnd, nid, mods | MOD_NOREPEAT, vkey);
        }
        break;
    }
}

void Window::clearHotkeys() {
    for (auto &p: ctx_->hotkeys) {
        UnregisterHotKey(ctx_->hwnd, p.first);
    }
    ctx_->hotkeys.clear();
}

int Window::messageBox(const wchar_t *msg, const wchar_t *title, uint32_t type) {
    return MessageBoxW(ctx_->hwnd, msg, title, type);
}

void Window::setSizeCallback(const std::function<void(int, int)> &cb) {
    ctx_->sizeCB = cb;
}

void *Window::hwnd() {
    return (void*)ctx_->hwnd;
}

}

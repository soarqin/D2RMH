/*
 * Copyright (c) 2022 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#include "configdlg.h"
#include "cfg.h"
#include "util/util.h"
#include <windows.h>
#include <commctrl.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <cmath>

EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)

extern Cfg sCfg;

namespace ui {

enum ConfigItemType {
    CITInt32,
    CITUInt8,
    CITFloat,
    CITColor,
    CITString,
    CITWString,
    CITPath,
    CITWPath,
};

struct ConfigItem {
    const wchar_t *title;
    ConfigItemType type;
    void *data;
    /*   1-ComboBox
     * for Path:
     *   2-folder selection
     */
    uint32_t flag;
    /* for ComboBox: const wchar_t **values, each pair of wstring = (name, value), ends with NULL wstring
     * for Int and Float: min Value
     */
    void *param1;
    /* for String and Path: hints wstring for empty editbox
     * for Int and Float: max Value
     */
    void *param2;
};

static BOOL CALLBACK setFontCallback(HWND hwnd, LPARAM font) {
    SendMessage(hwnd, WM_SETFONT, (WPARAM)font, 0);
    EnumChildWindows(hwnd, setFontCallback, font);
    return TRUE;
}

static void fileSelFunc(HWND hwnd, UINT id, bool folderOnly) {
    IFileDialog *pfd = nullptr;
    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
    if (!SUCCEEDED(hr)) { return; }
    DWORD dwFlags = 0;
    pfd->GetOptions(&dwFlags);
    pfd->SetOptions(dwFlags | FOS_FORCESHOWHIDDEN | (folderOnly ? (FOS_PATHMUSTEXIST | FOS_PICKFOLDERS) : FOS_FILEMUSTEXIST) | FOS_FORCEFILESYSTEM);
    if (!folderOnly) {
        COMDLG_FILTERSPEC specs[] = {
            {L"TrueType Files", L"*.ttf;*.ttc"},
            {L"D2R DC6 Files", L"*.dc6"},
        };
        pfd->SetFileTypes(2, specs);
        pfd->SetFileTypeIndex(0);
        pfd->SetDefaultExtension(L"ttf");
    }
    WCHAR path[1024];
    GetWindowTextW(GetDlgItem(hwnd, id), path, 1024);
    if (!folderOnly) {
        PathRemoveFileSpecW(path);
    }
    IShellItem *shellItem;
    if (SUCCEEDED(SHCreateItemFromParsingName(path, nullptr, IID_PPV_ARGS(&shellItem)))) {
        pfd->SetDefaultFolder(shellItem);
        pfd->SetFolder(shellItem);
    }
    hr = pfd->Show(hwnd);
    if (SUCCEEDED(hr)) {
        IShellItem *psiResult;
        hr = pfd->GetResult(&psiResult);
        if (SUCCEEDED(hr)) {
            PWSTR pszFilePath = nullptr;
            hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH,
                                           &pszFilePath);
            if (SUCCEEDED(hr)) {
                SetWindowTextW(GetDlgItem(hwnd, id), pszFilePath);
                CoTaskMemFree(pszFilePath);
            }
            psiResult->Release();
        }
    }
    pfd->Release();
};

void ConfigDlg::setupConfigItems(void *hwndParent) {
    static const wchar_t *languageComboItems[] = {
        L"Use Game Language", L"",
        L"English [enUS]", L"enUS",
        L"Deutsch [deDE]", L"deDE",
        L"español(españa) [esES]", L"esES",
        L"français [frFR]", L"frFR",
        L"italiano [itIT]", L"itIT",
        L"한국인 [koKR]", L"koKR",
        L"Polskie [plPL]", L"plPL",
        L"español(México) [esMX]", L"esMX",
        L"日本語 [jaJP]", L"jaJP",
        L"Português [ptBR]", L"ptBR",
        L"русский [ruRU]", L"ruRU",
        L"繁體中文 [zhTW]", L"zhTW",
        L"简体中文 [zhCN]", L"zhCN",
        nullptr, nullptr,
    };
    static const wchar_t *d2rFontComboItems[] = {
        L"formal436bt.ttf (default for western languages)", L"formal436bt.ttf",
        L"exocetblizzardot-medium.otf", L"exocetblizzardot-medium.otf",
        L"irisl.ttf", L"irisl.ttf",
        L"kodia.ttf", L"kodia.ttf",
        L"philosopher-bolditalic.ttf", L"philosopher-bolditalic.ttf",
        L"blizzardglobal-v5_81.ttf (Default for koKR/ruRU/zhCN)", L"blizzardglobal-v5_81.ttf",
        L"blizzardglobaltcunicode.ttf (default for zhTW)", L"blizzardglobaltcunicode.ttf",
        L"arfangxinshuh7c95b5_eb_t.ttf (Support zhTW)", L"arfangxinshuh7c95b5_eb_t.ttf",
        L"bljap_v8_3.ttf (default for jaJP)", L"bljap_v8_3.ttf",
        L"ik4ll3.ttf (Support jaJP)", L"ik4ll3.ttf",
        L"tbgdb_0pp.ttf (Support jaJP)", L"tbgdb_0pp.ttf",
        nullptr, nullptr,
    };
#define MVAL(n) (void*)uintptr_t(n)
    const ConfigItem configItems[] = {
        {L"D2 Legacy Path", CITWPath, &sCfg.d2Path, 2},
        {L"Display Font", CITPath, &sCfg.fontFilePath, 1, MVAL(d2rFontComboItems), MVAL(L"(Leave empty to use default font from D2R)")},
        {L"Map Text Size", CITInt32, &sCfg.fontSize, 0, MVAL(6), MVAL(48)},
        {L"Message Text Size", CITInt32, &sCfg.msgFontSize, 0, MVAL(12), MVAL(72)},
        {L"Language", CITString, &sCfg.languageOrig, 1, MVAL(languageComboItems)},
        {L"Text Color", CITColor, &sCfg.textColor},
        {L"Map Scale", CITFloat, &sCfg.scale, 0, MVAL(10), MVAL(40)},
    };
#undef MVAL

    functions_.clear();

    /* Add controls to dialog */
    int x = 15, y = 15;
    uintptr_t id = 1001;
    HWND temp;
    HWND hwnd = HWND(hwndParent);

    for (auto &item: configItems) {
        switch (item.type) {
        case CITInt32:
        case CITUInt8:
        case CITFloat:
        case CITColor:
        case CITString:
        case CITWString:
            break;
        default:
            if (x != 15) {
                x = 15;
                y += 25;
            }
            break;
        }
        CreateWindowW(L"STATIC", item.title, WS_CHILD | WS_VISIBLE | SS_RIGHT, x, y + 1, 110, 17, hwnd, nullptr, nullptr, nullptr);
        switch (item.type) {
        case CITInt32:
        case CITUInt8: {
            auto val = item.type == CITInt32 ? *(int*)item.data : int(uint32_t(*(uint8_t*)item.data));
            CreateWindowW(L"EDIT", std::to_wstring(val).c_str(), ES_AUTOHSCROLL | ES_NUMBER | ES_RIGHT | WS_BORDER | WS_CHILD | WS_VISIBLE, x + 115, y, 80, 17, hwnd, HMENU(id), nullptr, nullptr);
            temp = CreateWindowW(UPDOWN_CLASSW, L"", UDS_ARROWKEYS | UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_SETBUDDYINT | WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hwnd, HMENU(id + 1), nullptr, nullptr);
            SendMessage(temp, UDM_SETRANGE, 0, MAKELONG(intptr_t(item.param2), intptr_t(item.param1)));
            SendMessage(temp, UDM_SETPOS, 0, *(int*)item.data);
            break;
        }
        case CITFloat: {
            auto val = *(float*)item.data;
            auto n = std::lround(val * 10.f);
            wchar_t nstr[16];
            snwprintf(nstr, 16, L"%g", val);
            CreateWindowW(L"EDIT", nstr, ES_AUTOHSCROLL | ES_RIGHT | WS_BORDER | WS_CHILD | WS_VISIBLE, x + 115, y, 80, 17, hwnd, HMENU(id), nullptr, nullptr);
            temp = CreateWindowW(UPDOWN_CLASSW, L"", UDS_ARROWKEYS | UDS_ALIGNRIGHT | UDS_AUTOBUDDY | WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hwnd, HMENU(id + 1), nullptr, nullptr);
            auto maxVal = intptr_t(item.param2), minVal = intptr_t(item.param1);
            SendMessage(temp, UDM_SETRANGE, 0, MAKELONG(maxVal, minVal));
            SendMessage(temp, UDM_SETPOS, 0, n);
            functions_[id] = [hwnd, id, maxVal, minVal](uintptr_t lParam) {
                wchar_t nstr[16];
                GetWindowTextW(GetDlgItem(hwnd, id), nstr, 16);
                nstr[15] = 0;
                auto n = std::lround(_wtof(nstr) * 10.f);
                if (n > maxVal) { n = maxVal; }
                else if (n < minVal) { n = minVal; }
                SendMessage(GetDlgItem(hwnd, id + 1), UDM_SETPOS, 0, n);
            };
            functions_[id + 1] = [hwnd, id, maxVal, minVal](uintptr_t lParam) {
                auto pdata = (LPNMUPDOWN)lParam;
                auto n = pdata->iPos + pdata->iDelta;
                if (n > maxVal) { n = maxVal; }
                else if (n < minVal) { n = minVal; }
                auto val = float(n) * 0.1f;
                wchar_t nstr[16];
                snwprintf(nstr, 16, L"%g", val);
                SetWindowTextW(GetDlgItem(hwnd, id), nstr);
            };
            break;
        }
        case CITColor: {
            temp = CreateWindowW(L"STATIC", L"", SS_NOTIFY | WS_CHILD | WS_VISIBLE, x + 116, y, 39, 17, hwnd, HMENU(id), nullptr, nullptr);
            auto color = *(uint32_t*)item.data;
            SetWindowLong(temp, GWL_USERDATA, color);
            functions_[id] = [hwnd, id, color](uintptr_t) {
                CHOOSECOLORW cc = {};
                cc.lStructSize = sizeof(cc);
                cc.hwndOwner = hwnd;
                cc.rgbResult = color & 0xFFFFFFu;
                static COLORREF custClr[16] = {
                    RGB(255,255,255),RGB(255,255,255),RGB(255,255,255),RGB(255,255,255),
                    RGB(255,255,255),RGB(255,255,255),RGB(255,255,255),RGB(255,255,255),
                    RGB(255,255,255),RGB(255,255,255),RGB(255,255,255),RGB(255,255,255),
                    RGB(255,255,255),RGB(255,255,255),RGB(255,255,255),RGB(255,255,255),
                };
                custClr[0] = cc.rgbResult;
                cc.lpCustColors = custClr;
                cc.Flags = CC_FULLOPEN | CC_RGBINIT;
                if (ChooseColorW(&cc)) {
                    auto child = GetDlgItem(hwnd, id);
                    auto orig = GetWindowLong(child, GWL_USERDATA);
                    SetWindowLong(child, GWL_USERDATA, (orig & 0xFF000000U) | cc.rgbResult);
                    RedrawWindow(child, nullptr, nullptr, RDW_INVALIDATE);
                }
            };
            CreateWindowW(L"STATIC", L"Alpha:", WS_CHILD | WS_VISIBLE | SS_RIGHT, x + 160, y + 1, 40, 17, hwnd, nullptr, nullptr, nullptr);
            CreateWindowW(L"EDIT", std::to_wstring(*(int*)item.data).c_str(), ES_AUTOHSCROLL | ES_NUMBER | ES_RIGHT | WS_BORDER | WS_CHILD | WS_VISIBLE, x + 200, y, 50, 17, hwnd, HMENU(id + 1), nullptr, nullptr);
            temp = CreateWindowW(UPDOWN_CLASSW, L"", UDS_ARROWKEYS | UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_SETBUDDYINT | WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hwnd, HMENU(id + 2), nullptr, nullptr);
            SendMessage(temp, UDM_SETRANGE, 0, MAKELONG(255, 0));
            SendMessage(temp, UDM_SETPOS, 0, color >> 24);
            break;
        }
        case CITWPath: {
            if (item.flag & 1) {
                temp = CreateWindowW(L"COMBOBOX", ((std::wstring*)item.data)->c_str(), CBS_DROPDOWN | WS_BORDER | WS_CHILD | WS_VISIBLE, x + 115, y - 1, 435, 19, hwnd, HMENU(id), nullptr, nullptr);
                if (item.param2) {
                    SendMessage(temp, EM_SETCUEBANNER, FALSE, LPARAM(item.param2));
                }
                int sel = -1;
                const auto str = util::utf8toucs4(*(std::string*)item.data);
                const auto **list = (const wchar_t **)item.param1;
                for (int i = 0; list[i]; i += 2) {
                    SendMessageW(temp, CB_ADDSTRING, 0, LPARAM(list[i]));
                    if (str == list[i + 1]) {
                        sel = i >> 1;
                    }
                }
                SendMessage(temp, CB_SETCURSEL, sel, 0);
                if (item.param2) {
                    SendMessage(temp, CB_SETCUEBANNER, FALSE, LPARAM(item.param2));
                }
            } else {
                temp = CreateWindowW(L"EDIT", ((std::wstring*)item.data)->c_str(), ES_AUTOHSCROLL | WS_BORDER | WS_CHILD | WS_VISIBLE, x + 115, y, 435, 17, hwnd, HMENU(id), nullptr, nullptr);
                if (item.param2) {
                    SendMessage(temp, EM_SETCUEBANNER, FALSE, LPARAM(item.param2));
                }
            }
            CreateWindowW(L"BUTTON", L"..", BS_FLAT | WS_CHILD | WS_VISIBLE, x + 550, y - 1, 20, 19, hwnd, HMENU(id + 1), nullptr, nullptr);
            auto flag = item.flag;
            functions_[id+1] = [hwnd, id, flag](uintptr_t) {
                fileSelFunc(hwnd, id, (flag & 2U) != 0);
            };
            break;
        }
        case CITPath: {
            if (item.flag & 1) {
                temp = CreateWindowW(L"COMBOBOX", ((std::wstring*)item.data)->c_str(), CBS_DROPDOWN | WS_BORDER | WS_CHILD | WS_VISIBLE, x + 115, y - 1, 435, 19, hwnd, HMENU(id), nullptr, nullptr);
                if (item.param2) {
                    SendMessage(temp, EM_SETCUEBANNER, FALSE, LPARAM(item.param2));
                }
                int sel = -1;
                const auto &str = *(std::wstring*)item.data;
                const auto **list = (const wchar_t **)item.param1;
                for (int i = 0; list[i]; i += 2) {
                    SendMessageW(temp, CB_ADDSTRING, 0, LPARAM(list[i]));
                    if (str == list[i + 1]) {
                        sel = i >> 1;
                    }
                }
                SendMessage(temp, CB_SETCURSEL, sel, 0);
                if (item.param2) {
                    SendMessage(temp, CB_SETCUEBANNER, FALSE, LPARAM(item.param2));
                }
            } else {
                temp = CreateWindowW(L"EDIT", util::utf8toucs4(*(std::string*)item.data).c_str(), ES_AUTOHSCROLL | WS_BORDER | WS_CHILD | WS_VISIBLE, x + 115, y, 435, 17, hwnd, HMENU(id), nullptr, nullptr);
                if (item.param2) {
                    SendMessage(temp, EM_SETCUEBANNER, FALSE, LPARAM(item.param2));
                }
            }
            CreateWindowW(L"BUTTON", L"..", BS_FLAT | WS_CHILD | WS_VISIBLE, x + 550, y - 1, 20, 19, hwnd, HMENU(id + 1), nullptr, nullptr);
            auto flag = item.flag;
            functions_[id+1] = [hwnd, id, flag](uintptr_t) {
                fileSelFunc(hwnd, id, (flag & 2U) != 0);
            };
            break;
        }
        case CITWString: {
            if (item.flag & 1) {
                temp = CreateWindowW(L"COMBOBOX", L"", CBS_DROPDOWNLIST | WS_BORDER | WS_CHILD | WS_VISIBLE, x + 115, y - 1, 155, 19, hwnd, HMENU(id), nullptr, nullptr);
                const auto **list = (const wchar_t **)item.param1;
                int sel = 0;
                const auto &str = *(std::wstring*)item.data;
                for (int i = 0; list[i]; i += 2) {
                    SendMessageW(temp, CB_ADDSTRING, 0, LPARAM(list[i]));
                    if (str == list[i + 1]) {
                        sel = i >> 1;
                    }
                }
                SendMessage(temp, CB_SETCURSEL, sel, 0);
                break;
            }
            temp = CreateWindowW(L"EDIT", ((std::wstring*)item.data)->c_str(), ES_AUTOHSCROLL | WS_BORDER | WS_CHILD | WS_VISIBLE, x + 115, y, 155, 17, hwnd, HMENU(id), nullptr, nullptr);
            if (item.param2) {
                SendMessage(temp, EM_SETCUEBANNER, FALSE, LPARAM(item.param2));
            }
            break;
        }
        case CITString: {
            if (item.flag & 1) {
                temp = CreateWindowW(L"COMBOBOX", L"", CBS_DROPDOWNLIST | WS_BORDER | WS_CHILD | WS_VISIBLE, x + 115, y - 1, 155, 19, hwnd, HMENU(id), nullptr, nullptr);
                const auto **list = (const wchar_t **)item.param1;
                int sel = 0;
                const auto str = util::utf8toucs4(*(std::string*)item.data);
                for (int i = 0; list[i]; i += 2) {
                    SendMessageW(temp, CB_ADDSTRING, 0, LPARAM(list[i]));
                    if (str == list[i + 1]) {
                        sel = i >> 1;
                    }
                }
                SendMessage(temp, CB_SETCURSEL, sel, 0);
                break;
            }
            temp = CreateWindowW(L"EDIT", util::utf8toucs4(*(std::string*)item.data).c_str(), ES_AUTOHSCROLL | WS_BORDER | WS_CHILD | WS_VISIBLE, x + 115, y, 155, 17, hwnd, HMENU(id), nullptr, nullptr);
            if (item.param2) {
                SendMessage(temp, EM_SETCUEBANNER, FALSE, LPARAM(item.param2));
            }
            break;
        }
        default:
            break;
        }
        switch (item.type) {
        case CITInt32:
        case CITUInt8:
        case CITFloat:
        case CITColor:
        case CITString:
        case CITWString:
            if (x == 15) {
                x += 285;
            } else {
                x = 15;
                y += 25;
            }
            break;
        default:
            x = 15;
            y += 25;
            break;
        }
        id += 3;
    }

    if (x != 15) {
        y += 25;
    }
    /* Calculate window size and position */
    int w = 600, h = y + 10 + 30;
    RECT rc, rc2 = {0, 0, w, h};
    AdjustWindowRectEx(&rc2, GetWindowLong(hwnd, GWL_STYLE), FALSE, GetWindowLong(hwnd, GWL_EXSTYLE));
    auto width = rc2.right - rc2.left;
    auto height = rc2.bottom - rc2.top;
    GetWindowRect(GetDesktopWindow(), &rc);
    MoveWindow(hwnd, (rc.right + rc.left - width) / 2, (rc.top + rc.bottom - height) / 2, width, height, FALSE);

    /* Move OK and Cancel button to correct position */
    auto ok = GetDlgItem(hwnd, IDOK);
    auto cancel = GetDlgItem(hwnd, IDCANCEL);
    MoveWindow(ok, w / 2 - 140, h - 35, 120, 20, FALSE);
    MoveWindow(cancel, w / 2 + 20, h - 35, 120, 20, FALSE);

    /* Set font to all controls */
    auto font = GetStockObject(DEFAULT_GUI_FONT);
    EnumChildWindows(hwnd, setFontCallback, (LPARAM)font);
}

INT_PTR CALLBACK configDialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_INITDIALOG:
        SetWindowLongPtr(hWnd, GWLP_USERDATA, lParam);
        ((ConfigDlg*)lParam)->setupConfigItems(hWnd);
        break;
    case WM_CLOSE:
        EndDialog(hWnd, FALSE);
        break;
    case WM_CTLCOLORSTATIC: {
        if (GetDlgCtrlID((HWND)lParam) == 1000) {
            SetBkMode((HDC)wParam, TRANSPARENT);
            return (INT_PTR)::GetStockObject(NULL_BRUSH);
        }
        if (GetWindowLong((HWND)lParam, GWL_STYLE) & SS_NOTIFY) {
            static std::map<uint32_t, HBRUSH> brushes;
            auto &b = brushes[GetDlgCtrlID((HWND)lParam)];
            if (b) { DeleteObject(b); }
            b = CreateSolidBrush(GetWindowLong((HWND)lParam, GWL_USERDATA) & 0xFFFFFFu);
            return (INT_PTR)b;
        }
        return FALSE;
    }
    case WM_COMMAND: {
        auto id = LOWORD(wParam);
        switch (id) {
        case IDOK:
            EndDialog(hWnd, TRUE);
            break;
        case IDCANCEL:
            EndDialog(hWnd, FALSE);
            break;
        default:
            switch (HIWORD(wParam)) {
            case BN_CLICKED:
            case EN_CHANGE:
                ((ConfigDlg *)GetWindowLongPtr(hWnd, GWLP_USERDATA))->call(id, lParam);
                break;
            default:
                break;
            }
        }
        break;
    }
    case WM_NOTIFY: {
        switch (((LPNMHDR)lParam)->code) {
        case UDN_DELTAPOS:
            ((ConfigDlg *)GetWindowLongPtr(hWnd, GWLP_USERDATA))->call(((LPNMHDR)lParam)->idFrom, lParam);
            break;
        default:
            break;
        }
        break;
    }
    default:
        return FALSE;
    }
    return TRUE;
}

bool ConfigDlg::run() {
    return DialogBoxParamW(HINST_THISCOMPONENT, MAKEINTRESOURCEW(102), nullptr, configDialogProc, uintptr_t(this)) != FALSE;
}

}

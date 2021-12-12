#include "d2map.h"

#include "d2ptrs.h"
#include "offset.h"

#include <iostream>
#include <windows.h>

namespace d2mapapi {

d2client_struct D2Client;

const char *d2Init(const wchar_t *dir) {
    wchar_t szPath[MAX_PATH] = {0};
    GetCurrentDirectoryW(MAX_PATH, szPath);
    if (dir[0] != 0 && dir[lstrlenW(dir) - 1] != '\\') {
        wchar_t dira[MAX_PATH];
        lstrcpyW(dira, dir);
        lstrcatW(dira, L"\\");
        SetCurrentDirectoryW(dira);
    } else {
        SetCurrentDirectoryW(dir);
    }

    memset(&D2Client, 0, sizeof(d2client_struct));
    if (!defineOffsets()) {
        return "Diablo II Legacy: Failed to load DLLs!";
    }
    auto version = getD2Version();

    *p_STORM_MPQHashTable = 0;
    if (version == D2_111b || version == D2_112a) {
        D2Client.u112.dwInit = 1;
        D2Client.u112.fpInit = (uint32_t)D2ClientInterface;
    } else {
        D2Client.u113.dwInit = 1;
        D2Client.u113.fpInit = (uint32_t)D2ClientInterface;
    }

    FOG_10021("D2");
    FOG_10101(1, 0);
    FOG_10089(1);

    if (!FOG_10218()) {
        return "Diablo II Legacy: Initialize Failed!";
    }

    if (!D2WIN_10086() || !D2WIN_10005(0, 0, 0, &D2Client)) {
        return "Diablo II Legacy: Couldn't load MPQ files.\nPlease make sure you have a full install of Diablo II and copy the D2XMUSIC.MPQ and D2XVIDEO.MPQ from the Expansion CD";
    }

    D2LANG_Init(0, "ENG", 0);

    if (!D2COMMON_InitDataTables(0, 0, 0)) {
        return "Diablo II Legacy: Couldn't initialize sqptDataTable!";
    }

    D2CLIENT_InitGameMisc();

    SetCurrentDirectoryW(szPath);
    return nullptr;
}

Level *__fastcall getLevel(Act *act, uint32_t levelno) {
    auto d2Ver = getD2Version();
    for (Level *pLevel = act->pMisc(d2Ver)->pLevelFirst(d2Ver); pLevel; pLevel = pLevel->pNextLevel(d2Ver))
        if (pLevel->dwLevelNo(d2Ver) == levelno)
            return (Level*)pLevel;
    return D2COMMON_GetLevel(act->pMisc(d2Ver), levelno);
}

#if defined(_MSC_VER)
void __declspec(naked) D2CLIENT_InitGameMisc(void)
{
    __asm
    {
        PUSH ECX
        PUSH EBP
        PUSH ESI
        PUSH EDI
        JMP D2CLIENT_InitGameMisc_I
        RETN
    }
}
#else
void __attribute__((naked)) D2CLIENT_InitGameMisc() {
    asm volatile (
    "push %%ecx\n"
    "push %%ebp\n"
    "push %%esi\n"
    "push %%edi\n"
    "jmp *%0\n"
    "ret"
    :
    : "r"(D2CLIENT_InitGameMisc_I)
    );
}
#endif

uint32_t D2ClientInterface() {
    if (getD2Version() == D2_112a) {
        return D2Client.u112.dwInit;
    }
    return D2Client.u113.dwInit;
}

}

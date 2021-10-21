#include "d2map.h"

#include "d2ptrs.h"
#include "offset.h"

#include <iostream>
#include <windows.h>

d2client_struct D2Client;

char D2_DIR[MAX_PATH] = "";

void init(const char *dir) {
    snprintf(D2_DIR, sizeof(D2_DIR), "%s\\", dir);

    char szPath[MAX_PATH] = {0};
    GetCurrentDirectory(MAX_PATH, szPath);
    memset(&D2Client, 0, sizeof(d2client_struct));
    SetCurrentDirectory(D2_DIR);
    if (!defineOffsets()) {
        std::cout << "Game Server Initialize Failed!" << std::endl;
        ExitProcess(0);
    }

    *p_STORM_MPQHashTable = 0;
    D2Client.dwInit = 1;
    D2Client.fpInit = (uint32_t)D2ClientInterface;

    FOG_10021("D2");
    FOG_10101(1, 0);
    FOG_10089(1);

    if (!FOG_10218()) {
        std::cout << "Game Server Initialize Failed!" << std::endl;
        ExitProcess(0);
    }

    if (!D2WIN_10086() || !D2WIN_10005(0, 0, 0, &D2Client)) {
        std::cout
            << "Couldn't load Diablo 2 MPQ files. Please make sure you have a full install of Diablo II and copy the D2XMUSIC.MPQ and D2XVIDEO.MPQ from the Expansion CD"
            << std::endl;
        ExitProcess(0);
    }

    D2LANG_10008(0, "ENG", 0);

    if (!D2COMMON_InitDataTables(0, 0, 0)) {
        std::cout << "Couldn't initialize sqptDataTable!" << std::endl;
        ExitProcess(0);
    }

    D2CLIENT_InitGameMisc();

    SetCurrentDirectory(szPath);

}

Level *__fastcall getLevel(ActMisc *misc, uint32_t levelno) {
    for (Level *pLevel = misc->pLevelFirst; pLevel; pLevel = pLevel->pNextLevel)
        if (pLevel->dwLevelNo == levelno)
            return pLevel;

    return D2COMMON_GetLevel(misc, levelno);
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
    return D2Client.dwInit;
}

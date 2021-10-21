#define _DEFINE_VARS

#include "offset.h"

#include "d2ptrs.h"

#include <iostream>
#include <windows.h>

bool defineOffsets() {
    void *ptrToLoad[] = {
        &D2CLIENT_InitGameMisc_I,
        &p_STORM_MPQHashTable,
        &D2CLIENT_LoadAct_1,
        &D2CLIENT_LoadAct_2,
        &D2COMMON_AddRoomData,
        &D2COMMON_RemoveRoomData,
        &D2COMMON_GetLevel,
        &D2COMMON_InitLevel,
        &D2COMMON_LoadAct,
        &D2COMMON_UnloadAct,
        &FOG_10021,
        &FOG_10101,
        &FOG_10089,
        &FOG_10218,
        &D2WIN_10086,
        &D2WIN_10005,
        &D2LANG_10008,
        &D2COMMON_InitDataTables,
    };
    for (auto *ptr: ptrToLoad) {
        auto *p = (DWORD*)ptr;
        *p = getDllOffset(*p);
        fflush(stdout);
        if (!*p) { return false; }
    }
    return true;
}

uint32_t GetDllOffset(const char *DllName, int Offset) {
    try {
        HMODULE hMod = GetModuleHandle(DllName);

        if (!hMod)
            hMod = LoadLibrary(DllName);

        if (!hMod)
            return 0;

        if (Offset < 0)
            return (DWORD)GetProcAddress(hMod, (LPCSTR)(-Offset));

        return ((DWORD)hMod) + Offset;
    }
    catch (...) {
        std::cout << "getDllOffset" << std::endl;
    }

    return 0;
}

uint32_t getDllOffset(int num) {
    static const char *dlls[] = {"D2Client.DLL", "D2Common.DLL", "D2Gfx.DLL", "D2Lang.DLL",
                                 "D2Win.DLL", "D2Net.DLL", "D2Game.DLL", "D2Launch.DLL", "Fog.DLL", "BNClient.DLL",
                                 "Storm.DLL", "D2Cmp.DLL"};
    if ((num & 0xff) > 12)
        return 0;
    return GetDllOffset(dlls[num & 0xff], num >> 8);
}

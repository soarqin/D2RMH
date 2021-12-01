#include "offset.h"

#define _DEFINE_VARS
#include "d2ptrs.h"

#include <iostream>
#include <windows.h>

namespace d2mapapi {

bool defineOffsets() {
    const struct {
        const char *dll;
        void *data;
        int32_t ordinal;
    } offsets[] = {
        {"STORM.DLL", &p_STORM_MPQHashTable, 0x53120},
        {"D2CLIENT.DLL", &D2CLIENT_LoadAct_1, 0x62AA0},
        {"D2CLIENT.DLL", &D2CLIENT_LoadAct_2, 0x62760},
        {"D2CLIENT.DLL", &D2CLIENT_InitGameMisc_I, 0x4454B},
        {"D2COMMON.DLL", &D2COMMON_AddRoomData, -10401},
        {"D2COMMON.DLL", &D2COMMON_RemoveRoomData, -11099},
        {"D2COMMON.DLL", &D2COMMON_GetLevel, -10207},

        {"D2COMMON.DLL", &D2COMMON_InitLevel, -10322},
        {"D2COMMON.DLL", &D2COMMON_LoadAct, -10951},
        {"D2COMMON.DLL", &D2COMMON_UnloadAct, -10868},

        {"FOG.DLL", &FOG_10021, -10021},
        {"FOG.DLL", &FOG_10101, -10101},
        {"FOG.DLL", &FOG_10089, -10089},
        {"FOG.DLL", &FOG_10218, -10218},

        {"D2WIN.DLL", &D2WIN_10086, -10086},
        {"D2WIN.DLL", &D2WIN_10005, -10005},

        {"D2LANG.DLL", &D2LANG_10008, -10008},
        {"D2COMMON.DLL", &D2COMMON_InitDataTables, -10943},
    };
    for (const auto &off: offsets) {
        HMODULE hMod = GetModuleHandle(off.dll);
        if (!hMod) {
            hMod = LoadLibrary(off.dll);
        }
        if (!hMod) {
            return false;
        }
        uintptr_t addr;
        if (off.ordinal < 0) {
            addr = (uintptr_t)GetProcAddress(hMod, (const char *)-off.ordinal);
        } else {
            addr = (uintptr_t)hMod + off.ordinal;
        }
        if (!addr) {
            return false;
        }
        *(uintptr_t*)off.data = addr;
    }
    return true;
}

}

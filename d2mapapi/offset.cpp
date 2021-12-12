#include "offset.h"

#define _DEFINE_VARS
#include "d2ptrs.h"
#include "crc32.h"

#include <iostream>
#include <windows.h>
#include <sys/stat.h>
#include <sys/types.h>

namespace d2mapapi {

static D2Version d2version = D2_113c;

D2Version getD2Version() {
    return d2version;
}

bool defineOffsets() {
    const struct DllOffset {
        const char *dll;
        void *data;
        int32_t ordinal;
    } offsets_111b[] = {
        {"STORM.DLL", &p_STORM_MPQHashTable, 0x54D30},
        {"D2CLIENT.DLL", &D2CLIENT_LoadAct_1, 0x52F40},
        {"D2CLIENT.DLL", &D2CLIENT_LoadAct_2, 0x52C00},
        {"D2CLIENT.DLL", &D2CLIENT_InitGameMisc_I, 0x32E5B},
        {"D2COMMON.DLL", &D2COMMON_AddRoomData, -10787},
        {"D2COMMON.DLL", &D2COMMON_RemoveRoomData, -10672},
        {"D2COMMON.DLL", &D2COMMON_GetLevel, -11058},

        {"D2COMMON.DLL", &D2COMMON_InitLevel, -10741},
        {"D2COMMON.DLL", &D2COMMON_LoadAct, 0x264A0}, /* -10669 */
        {"D2COMMON.DLL", &D2COMMON_UnloadAct, -10651},

        {"FOG.DLL", &FOG_10021, -10021},
        {"FOG.DLL", &FOG_10101, -10101},
        {"FOG.DLL", &FOG_10089, -10089},
        {"FOG.DLL", &FOG_10218, -10218},

        {"D2WIN.DLL", &D2WIN_10086, -10030},
        {"D2WIN.DLL", &D2WIN_10005, -10051},

        {"D2LANG.DLL", &D2LANG_Init, -10009},
        {"D2COMMON.DLL", &D2COMMON_InitDataTables, -10506},
        {nullptr},
    }, offsets_112a[] = {
        {"STORM.DLL", &p_STORM_MPQHashTable, 0x55358},
        {"D2CLIENT.DLL", &D2CLIENT_LoadAct_1, 0x409E0},
        {"D2CLIENT.DLL", &D2CLIENT_LoadAct_2, 0x406A0},
        {"D2CLIENT.DLL", &D2CLIENT_InitGameMisc_I, 0x7CD2B},
        {"D2COMMON.DLL", &D2COMMON_AddRoomData, -10184},
        {"D2COMMON.DLL", &D2COMMON_RemoveRoomData, -11009},
        {"D2COMMON.DLL", &D2COMMON_GetLevel, -11020},

        {"D2COMMON.DLL", &D2COMMON_InitLevel, -10721},
        {"D2COMMON.DLL", &D2COMMON_LoadAct, 0x56780}, /* -10588 */
        {"D2COMMON.DLL", &D2COMMON_UnloadAct, -10710},

        {"FOG.DLL", &FOG_10021, -10021},
        {"FOG.DLL", &FOG_10101, -10101},
        {"FOG.DLL", &FOG_10089, -10089},
        {"FOG.DLL", &FOG_10218, -10218},

        {"D2WIN.DLL", &D2WIN_10086, -10059},
        {"D2WIN.DLL", &D2WIN_10005, -10073},

        {"D2LANG.DLL", &D2LANG_Init, -10003},
        {"D2COMMON.DLL", &D2COMMON_InitDataTables, -10797},
        {nullptr},
    }, offsets_113c[] = {
        {"STORM.DLL", &p_STORM_MPQHashTable, 0x53120},
        {"D2CLIENT.DLL", &D2CLIENT_LoadAct_1, 0x62AA0},
        {"D2CLIENT.DLL", &D2CLIENT_LoadAct_2, 0x62760},
        {"D2CLIENT.DLL", &D2CLIENT_InitGameMisc_I, 0x4454B},
        {"D2COMMON.DLL", &D2COMMON_AddRoomData, -10401},
        {"D2COMMON.DLL", &D2COMMON_RemoveRoomData, -11099},
        {"D2COMMON.DLL", &D2COMMON_GetLevel, -10207},

        {"D2COMMON.DLL", &D2COMMON_InitLevel, -10322},
        {"D2COMMON.DLL", &D2COMMON_LoadAct, 0x3CB30},
        {"D2COMMON.DLL", &D2COMMON_UnloadAct, -10868},

        {"FOG.DLL", &FOG_10021, -10021},
        {"FOG.DLL", &FOG_10101, -10101},
        {"FOG.DLL", &FOG_10089, -10089},
        {"FOG.DLL", &FOG_10218, -10218},

        {"D2WIN.DLL", &D2WIN_10086, -10086},
        {"D2WIN.DLL", &D2WIN_10005, -10005},

        {"D2LANG.DLL", &D2LANG_Init, -10008},
        {"D2COMMON.DLL", &D2COMMON_InitDataTables, -10943},
        {nullptr},
    }, offsets_113d[] = {
        {"STORM.DLL", &p_STORM_MPQHashTable, 0x52A60},
        {"D2CLIENT.DLL", &D2CLIENT_LoadAct_1, 0x737F0},
        {"D2CLIENT.DLL", &D2CLIENT_LoadAct_2, 0x2B420},
        {"D2CLIENT.DLL", &D2CLIENT_InitGameMisc_I, 0x4559B},
        {"D2COMMON.DLL", &D2COMMON_AddRoomData, 0x24990},
        {"D2COMMON.DLL", &D2COMMON_RemoveRoomData, 0x24930},
        {"D2COMMON.DLL", &D2COMMON_GetLevel, 0x6D440},

        {"D2COMMON.DLL", &D2COMMON_InitLevel, 0x6DDF0},
        {"D2COMMON.DLL", &D2COMMON_LoadAct, 0x24810},
        {"D2COMMON.DLL", &D2COMMON_UnloadAct, 0x24590},

        {"FOG.DLL", &FOG_10021, -10021},
        {"FOG.DLL", &FOG_10101, -10101},
        {"FOG.DLL", &FOG_10089, -10089},
        {"FOG.DLL", &FOG_10218, -10218},

        {"D2WIN.DLL", &D2WIN_10086, -10174},
        {"D2WIN.DLL", &D2WIN_10005, -10072},

        {"D2LANG.DLL", &D2LANG_Init, -10009},
        {"D2COMMON.DLL", &D2COMMON_InitDataTables, -10081},
        {nullptr},
    }, *offsets = nullptr;
    const struct DllSizeToVersion {
        uint32_t gameCrc32;
        uint32_t stormCrc32;
        const DllOffset *offsets;
        D2Version version;
    } sizeMap[] = {
        { 0x8fd3f392, 0xb6390775, offsets_111b, D2_111b },
        { 0xab566eaa, 0xe5b0f351, offsets_112a, D2_112a },
        { 0xea2f0e6e, 0x5711a8b4, offsets_113c, D2_113c },
        { 0xb3d69c47, 0xbdb6784e, offsets_113d, D2_113d },
    };
    uint32_t gameCrc32, stormCrc32;
    auto crc32File = [](const wchar_t *filename)->uint32_t {
        auto file = CreateFileW(filename, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (file == INVALID_HANDLE_VALUE) {
            return 0;
        }
        std::string data;
        auto size = GetFileSize(file, nullptr);
        data.resize(size);
        DWORD bytesRead;
        ReadFile(file, data.data(), size, &bytesRead, nullptr);
        CloseHandle(file);
        return crc::crc32(data.data(), size);
    };
    /* Do CRC check for game.exe first, fallback to Storm.dll */
    gameCrc32 = crc32File(L"Game.exe");
    stormCrc32 = gameCrc32 ? 0 : crc32File(L"Storm.dll");
    for (auto &sm: sizeMap) {
        if (gameCrc32 == sm.gameCrc32 || stormCrc32 == sm.stormCrc32) {
            offsets = sm.offsets;
            d2version = sm.version;
            break;
        }
    }
    if (!offsets) {
        return false;
    }
    for (const auto *off = offsets; off->dll; off++) {
        HMODULE hMod = GetModuleHandle(off->dll);
        if (!hMod) {
            hMod = LoadLibrary(off->dll);
        }
        if (!hMod) {
            return false;
        }
        uintptr_t addr;
        if (off->ordinal < 0) {
            addr = (uintptr_t)GetProcAddress(hMod, (const char *)-off->ordinal);
        } else {
            addr = (uintptr_t)hMod + off->ordinal;
        }
        if (!addr) {
            return false;
        }
        *(uintptr_t*)off->data = addr;
    }
    return true;
}

}

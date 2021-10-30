#pragma once

#include "d2structs.h"

#ifdef _DEFINE_VARS

enum {DLLNO_D2CLIENT, DLLNO_D2COMMON, DLLNO_D2GFX, DLLNO_D2LANG, DLLNO_D2WIN, DLLNO_D2NET, DLLNO_D2GAME, DLLNO_D2LAUNCH, DLLNO_FOG, DLLNO_BNCLIENT, DLLNO_STORM, DLLNO_D2CMP, DLLNO_D2MULTI, DLLNO_D2SOUND};

#define DLLOFFSET(a1,b1)         ((DLLNO_##a1)|((b1)<<8))
#define FUNCPTR(d1,v1,t1,t2,o1)  typedef t1 d1##_##v1##_t t2; d1##_##v1##_t *d1##_##v1 = (d1##_##v1##_t *)DLLOFFSET(d1,o1);
#define VARPTR(d1,v1,t1,o1)      typedef t1 d1##_##v1##_t;    d1##_##v1##_t *p_##d1##_##v1 = (d1##_##v1##_t *)DLLOFFSET(d1,o1);
#define ASMPTR(d1,v1,o1)         uint32_t d1##_##v1 = DLLOFFSET(d1,o1);

#else

#define FUNCPTR(d1, v1, t1, t2, o1)  typedef t1 d1##_##v1##_t t2; extern d1##_##v1##_t *d1##_##v1;
#define VARPTR(d1, v1, t1, o1)       typedef t1 d1##_##v1##_t;    extern d1##_##v1##_t *p_##d1##_##v1;
#define ASMPTR(d1, v1, o1)           extern uint32_t d1##_##v1;

#endif

FUNCPTR(D2CLIENT, InitGameMisc_I, void __stdcall, (uint32_t Dummy1, uint32_t Dummy2, uint32_t Dummy3), 0x4454B) // Updated
VARPTR(STORM, MPQHashTable, uint32_t, 0x53120) // Updated
ASMPTR(D2CLIENT, LoadAct_1, 0x62AA0) // Updated
ASMPTR(D2CLIENT, LoadAct_2, 0x62760) // Updated
FUNCPTR(D2COMMON,
        AddRoomData,
        void __stdcall,
        (Act * ptAct, int LevelId, int Xpos, int Ypos, Room1 * pRoom),
        -10401)//Updated  // 1.12 -10184
FUNCPTR(D2COMMON,
        RemoveRoomData,
        void __stdcall,
        (Act * ptAct, int LevelId, int Xpos, int Ypos, Room1 * pRoom),
        -11099)//Updated // 1.12 -11009
FUNCPTR(D2COMMON, GetLevel, Level * __fastcall, (ActMisc * pMisc, uint32_t dwLevelNo), -10207)//Updated // 1.12 -11020

FUNCPTR(D2COMMON, InitLevel, void __stdcall, (Level * pLevel), -10322)//Updated // 1.12 -10721
FUNCPTR(D2COMMON,
        LoadAct,
        Act* __stdcall,
        (uint32_t ActNumber, uint32_t Seed, uint32_t Unk, void *pGame, uint32_t Difficulty, void *pMempool, uint32_t TownLevelId, uint32_t Func_1, uint32_t Func_2),
        -10951)//Updated 1.13 0x3CB30 // 1.12  0x56780
FUNCPTR(D2COMMON, UnloadAct, void __stdcall, (Act * pAct), -10868) //Updated // 1.12 -10710

FUNCPTR(FOG, 10021, void __fastcall, (const char *szProg), -10021) // 1.12 & 1.13
FUNCPTR(FOG, 10101, uint32_t __fastcall, (uint32_t _1, uint32_t _2), -10101) // 1.12 & 1.13
FUNCPTR(FOG, 10089, uint32_t __fastcall, (uint32_t _1), -10089) // 1.12 & 1.13
FUNCPTR(FOG, 10218, uint32_t __fastcall, (void), -10218) // 1.12 & 1.13

FUNCPTR(D2WIN, 10086, uint32_t __fastcall, (void), -10086) // Updated
FUNCPTR(D2WIN, 10005, uint32_t __fastcall, (uint32_t _1, uint32_t _2, uint32_t _3, d2client_struct * pD2Client), -10005) //Updated

FUNCPTR(D2LANG, 10008, uint32_t __fastcall, (uint32_t _1, const char *_2, uint32_t _3), -10008) //Updated
FUNCPTR(D2COMMON, InitDataTables, uint32_t __stdcall, (uint32_t _1, uint32_t _2, uint32_t _3), -10943)//Updated //1.12 -10797

#undef FUNCPTR
#undef VARPTR
#undef ASMPTR

#pragma once

#include "d2structs.h"

#if defined(_DEFINE_VARS)
#define D2EXTERN
#else
#define D2EXTERN extern
#endif

namespace d2mapapi {

D2EXTERN uint32_t *p_STORM_MPQHashTable;
D2EXTERN uint32_t D2CLIENT_LoadAct_1;
D2EXTERN uint32_t D2CLIENT_LoadAct_2;
D2EXTERN void (__stdcall* D2CLIENT_InitGameMisc_I)(uint32_t Dummy1, uint32_t Dummy2, uint32_t Dummy3);
D2EXTERN void (__stdcall* D2COMMON_AddRoomData)(Act * ptAct, int LevelId, int Xpos, int Ypos, Room1 *pRoom);
D2EXTERN void (__stdcall* D2COMMON_RemoveRoomData)(Act * ptAct, int LevelId, int Xpos, int Ypos, Room1 *pRoom);
D2EXTERN Level * (__fastcall* D2COMMON_GetLevel)(ActMisc * pMisc, uint32_t dwLevelNo);

D2EXTERN void (__stdcall* D2COMMON_InitLevel)(Level * pLevel);
D2EXTERN Act* (__stdcall* D2COMMON_LoadAct)(uint32_t ActNumber, uint32_t Seed, uint32_t Unk, void *pGame, uint32_t Difficulty, void *pMempool, uint32_t TownLevelId, uint32_t Func_1, uint32_t Func_2);
D2EXTERN void (__stdcall* D2COMMON_UnloadAct)(Act * pAct);

D2EXTERN void (__fastcall* FOG_10021)(const char *szProg);
D2EXTERN uint32_t (__fastcall* FOG_10101)(uint32_t Dummy1, uint32_t Dummy2);
D2EXTERN uint32_t (__fastcall* FOG_10089)(uint32_t Dummy1);
D2EXTERN uint32_t (__fastcall* FOG_10218)(void);

D2EXTERN uint32_t (__fastcall* D2WIN_10086)(void);
D2EXTERN uint32_t (__fastcall* D2WIN_10005)(uint32_t Dummy1, uint32_t Dummy2, uint32_t Dummy3, d2client_struct * pD2Client);

D2EXTERN uint32_t (__fastcall* D2LANG_Init)(uint32_t Dummy1, const char *_2, uint32_t Dummy3);
D2EXTERN uint32_t (__stdcall* D2COMMON_InitDataTables)(uint32_t Dummy1, uint32_t Dummy2, uint32_t Dummy3);

}

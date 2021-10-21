#pragma once

#include <cstdint>

struct UnitAny;
struct Room1;
struct Room2;
struct Level;
struct Act;
struct ActMisc;
struct OverheadMsg;

#pragma pack(push)
#pragma pack(1)

struct d2client_struct //Updated 1.13
{
    uint32_t dwInit;                    //0x00
    uint8_t _1[0x20D - 4];                //0x04
    uint32_t fpInit;                    //0x20D

};

#pragma pack(pop)

struct CollMap { //1.13?
    uint32_t dwPosGameX;                //0x00
    uint32_t dwPosGameY;                //0x04
    uint32_t dwSizeGameX;                //0x08
    uint32_t dwSizeGameY;                //0x0C
    uint32_t dwPosRoomX;                //0x10
    uint32_t dwPosRoomY;                //0x14
    uint32_t dwSizeRoomX;                //0x18
    uint32_t dwSizeRoomY;                //0x1C
    uint16_t *pMapStart;                //0x20
    uint16_t *pMapEnd;                    //0x22
};

#pragma pack(push)
#pragma pack(1)

//1.13c - RoomTile - McGod
struct RoomTile {
    Room2 *pRoom2;            //0x00
    RoomTile *pNext;          //0x04
    uint32_t _2[2];            //0x08
    uint32_t *nNum;             //0x10
};

struct QuestInfo { //1.13?
    void *pBuffer;                    //0x00
    uint32_t _1;                        //0x04
};

struct Waypoint { //1.13?
    uint8_t flags;                        //0x00
};

struct PlayerData { //1.13?
    char szName[0x10];                //0x00
    QuestInfo *pNormalQuest;        //0x10
    QuestInfo *pNightmareQuest;        //0x14
    QuestInfo *pHellQuest;            //0x18
    Waypoint *pNormalWaypoint;        //0x1c
    Waypoint *pNightmareWaypoint;    //0x20
    Waypoint *pHellWaypoint;        //0x24
};

//1.13c - PresetUnit - McGod
struct PresetUnit {
    uint32_t _1;                  //0x00
    uint32_t dwTxtFileNo;            //0x04
    uint32_t dwPosX;               //0x08
    PresetUnit *pPresetNext;      //0x0C
    uint32_t _3;                  //0x10
    uint32_t dwType;               //0x14
    uint32_t dwPosY;               //0x18
};

//1.13c - Level - McGod
struct Level {
    uint32_t _1[4];         //0x00
    Room2 *pRoom2First;      //0x10
    uint32_t _2[2];         //0x14
    uint32_t dwPosX;         //0x1C
    uint32_t dwPosY;         //0x20
    uint32_t dwSizeX;         //0x24
    uint32_t dwSizeY;         //0x28
    uint32_t _3[96];         //0x2C
    Level *pNextLevel;      //0x1AC
    uint32_t _4;            //0x1B0
    ActMisc *pMisc;         //0x1B4
    uint32_t _5[6];         //0x1BC
    uint32_t dwLevelNo;      //0x1D0
};

//1.13c - Room2 - McGod
struct Room2 {
    uint32_t _1[2];         //0x00
    Room2 **pRoom2Near;      //0x08
    uint32_t _2[6];         //0x0C
    Room2 *pRoom2Next;      //0x24
    uint32_t dwRoomFlags;      //0x28
    uint32_t dwRoomsNear;      //0x2C
    Room1 *pRoom1;         //0x30
    uint32_t dwPosX;         //0x34
    uint32_t dwPosY;         //0x38
    uint32_t dwSizeX;         //0x3C
    uint32_t dwSizeY;         //0x40
    uint32_t _3;            //0x44
    uint32_t dwPresetType;      //0x48
    RoomTile *pRoomTiles;   //0x4C
    uint32_t _4[2];         //0x50
    Level *pLevel;         //0x58
    PresetUnit *pPreset;   //0x5C
};

#pragma pack(pop)

//1.13c - Room1 - McGod   
struct Room1 {
    Room1 **pRoomsNear;    //0x00
    uint32_t _1[3];         //0x04
    Room2 *pRoom2;         //0x10
    uint32_t _2[3];         //0x14
    CollMap *Coll;         //0x20
    uint32_t dwRoomsNear;      //0x24
    uint32_t _3[9];         //0x28
    uint32_t dwPosX;         //0x4C
    uint32_t dwPosY;         //0x50
    uint32_t dwSizeX;         //0x54
    uint32_t dwSizeY;         //0x58
    uint32_t _4[6];         //0x5C
    UnitAny *pUnitFirst;   //0x74
    uint32_t _5;            //0x78
    Room1 *pRoomNext;      //0x7C
};

//1.13c - ActMisc - McGod
struct ActMisc {
    uint32_t _1[37];         //0x00
    uint32_t dwStaffTombLevel; //0x94
    uint32_t _2[245];         //0x98
    Act *pAct;            //0x46C
    uint32_t _3[3];         //0x470
    Level *pLevelFirst;      //0x47C
};

//1.13c - Act - McGod   
struct Act {
    uint32_t _1[3];         //0x00
    uint32_t dwMapSeed;      //0x0C
    Room1 *pRoom1;         //0x10
    uint32_t dwAct;         //0x14
    uint32_t _2[12];         //0x18
    ActMisc *pMisc;         //0x48
};

struct Path { //1.13?
    uint16_t xOffset;                    //0x00
    uint16_t xPos;                        //0x02
    uint16_t yOffset;                    //0x04
    uint16_t yPos;                        //0x06
    uint32_t _1[2];                    //0x08
    uint16_t xTarget;                    //0x10
    uint16_t yTarget;                    //0x12
    uint32_t _2[2];                    //0x14
    Room1 *pRoom1;                    //0x1C
    Room1 *pRoomUnk;                //0x20
    uint32_t _3[3];                    //0x24
    UnitAny *pUnit;                    //0x30
    uint32_t dwFlags;                    //0x34
    uint32_t _4;                        //0x38
    uint32_t dwPathType;                //0x3C
    uint32_t dwPrevPathType;            //0x40
    uint32_t dwUnitSize;                //0x44
    uint32_t _5[4];                    //0x48
    UnitAny *pTargetUnit;            //0x58
    uint32_t dwTargetType;                //0x5C
    uint32_t dwTargetId;                //0x60
    uint8_t bDirection;                //0x64
};

struct ItemPath { //1.13?
    uint32_t _1[3];                    //0x00
    uint32_t dwPosX;                    //0x0C
    uint32_t dwPosY;                    //0x10
    //Use Path for the rest
};

struct Stat { //1.13?
    uint16_t wSubIndex;                    //0x00
    uint16_t wStatIndex;                //0x02
    uint32_t dwStatValue;                //0x04
};

struct StatList { //1.13?
    uint32_t _1[9];                    //0x00
    Stat *pStat;                    //0x24
    uint16_t wStatCount1;                //0x28
    uint16_t wStatCount2;                //0x2A
    uint32_t _2[2];                    //0x2C
    uint8_t *_3;                        //0x34
    uint32_t _4;                        //0x38
    StatList *pNext;                //0x3C
};

struct Inventory1 { //1.13?
    uint32_t dwSignature;                //0x00
    uint8_t *bGame1C;                    //0x04
    UnitAny *pOwner;                //0x08
    UnitAny *pFirstItem;            //0x0C
    UnitAny *pLastItem;                //0x10
    uint32_t _1[2];                    //0x14
    uint32_t dwLeftItemUid;            //0x1C
    UnitAny *pCursorItem;            //0x20
    uint32_t dwOwnerId;                //0x24
    uint32_t dwItemCount;                //0x28
};

struct Light { //1.13?
    uint32_t _1[3];                    //0x00
    uint32_t dwType;                    //0x0C
    uint32_t _2[7];                    //0x10
    uint32_t dwStaticValid;            //0x2C
    int *pnStaticMap;                //0x30
};

struct SkillInfo { //1.13?
    uint16_t wSkillId;                    //0x00
};

struct Skill { //1.13?
    SkillInfo *pSkillInfo;            //0x00
    Skill *pNextSkill;                //0x04
    uint32_t _1[8];                    //0x08
    uint32_t dwSkillLevel;                //0x28
    uint32_t _2[2];                    //0x2C
    uint32_t dwFlags;                    //0x30
};

struct Info { //1.13?
    uint8_t *pGame1C;                    //0x00
    Skill *pFirstSkill;                //0x04
    Skill *pLeftSkill;                //0x08
    Skill *pRightSkill;                //0x0C
};

struct ItemData { //1.13?
    uint32_t dwQuality;                //0x00
    uint32_t _1[2];                    //0x04
    uint32_t dwItemFlags;                //0x0C 1 = Owned by player, 0xFFFFFFFF = Not owned
    uint32_t _2[2];                    //0x10
    uint32_t dwFlags;                    //0x18
    uint32_t _3[3];                    //0x1C
    uint32_t dwQuality2;                //0x28
    uint32_t dwItemLevel;                //0x2C
    uint32_t _4[2];                    //0x30
    uint16_t wPrefix;                    //0x38
    uint16_t _5[2];                        //0x3A
    uint16_t wSuffix;                    //0x3E
    uint32_t _6;                        //0x40
    uint8_t BodyLocation;                //0x44
    uint8_t ItemLocation;                //0x45 Non-body/belt location (Body/Belt == 0xFF)
    uint8_t _7;                        //0x46
    uint16_t _8;                        //0x47
    uint32_t _9[4];                    //0x48
    Inventory1 *pOwnerInventory;    //0x5C
    uint32_t _10;                        //0x60
    UnitAny *pNextInvItem;            //0x64
    uint8_t _11;                        //0x68
    uint8_t NodePage;                    //0x69 Actual location, this is the most reliable by far
    uint16_t _12;                        //0x6A
    uint32_t _13[6];                    //0x6C
    UnitAny *pOwner;                //0x84
};

struct MonsterData { //1.13
    uint8_t _1[22];                    //0x00
    struct {
        uint8_t fUnk: 1;
        uint8_t fNormal: 1;
        uint8_t fChamp: 1;
        uint8_t fBoss: 1;
        uint8_t fMinion: 1;
    };                //0x16
    uint8_t _2[5]; //[6]
    uint8_t anEnchants[9];                //0x1C
    uint16_t wUniqueNo;                    //0x26
    uint32_t _5;                        //0x28
    struct {
        wchar_t wName[28];
    };                                //0x2C
};

struct ObjectTxt { //1.13?
    char szName[0x40];                //0x00
    wchar_t wszName[0x40];            //0x40
    uint8_t _1[4];                        //0xC0
    uint8_t nSelectable0;                //0xC4
    uint8_t _2[0x87];                    //0xC5
    uint8_t nOrientation;                //0x14C
    uint8_t _2b[0x19];                    //0x14D
    uint8_t nSubClass;                    //0x166
    uint8_t _3[0x11];                    //0x167
    uint8_t nParm0;                    //0x178
    uint8_t _4[0x39];                    //0x179
    uint8_t nPopulateFn;                //0x1B2
    uint8_t nOperateFn;                //0x1B3
    uint8_t _5[8];                        //0x1B4
    uint32_t nAutoMap;                    //0x1BB
};

struct ObjectData { //1.13?
    ObjectTxt *pTxt;                //0x00
    uint32_t Type;                        //0x04 (0x0F would be a Exp Shrine)
    uint32_t _1[8];                    //0x08
    char szOwner[0x10];                //0x28
};

struct ObjectPath { //1.13?
    Room1 *pRoom1;                    //0x00
    uint32_t _1[2];                    //0x04
    uint32_t dwPosX;                    //0x0C
    uint32_t dwPosY;                    //0x10
    //Leaving rest undefined, use Path
};

struct UnitAny { //1.13?
    uint32_t dwType;                    //0x00
    uint32_t dwTxtFileNo;                //0x04
    uint32_t _1;                        //0x08
    uint32_t dwUnitId;                    //0x0C
    uint32_t dwMode;                    //0x10
    union {
        PlayerData *pPlayerData;
        ItemData *pItemData;
        MonsterData *pMonsterData;
        ObjectData *pObjectData;
        //TileData *pTileData doesn't appear to exist anymore
    };                                //0x14
    uint32_t dwAct;                    //0x18
    Act *pAct;                        //0x1C
    uint32_t dwSeed[2];                //0x20
    uint32_t _2;                        //0x28
    union {
        Path *pPath;
        ItemPath *pItemPath;
        ObjectPath *pObjectPath;
    };                                //0x2C
    uint32_t _3[5];                    //0x30
    uint32_t dwGfxFrame;                //0x44
    uint32_t dwFrameRemain;            //0x48
    uint16_t wFrameRate;                //0x4C
    uint16_t _4;                        //0x4E
    uint8_t *pGfxUnk;                    //0x50
    uint32_t *pGfxInfo;                //0x54
    uint32_t _5;                        //0x58
    StatList *pStats;                //0x5C
    Inventory1 *pInventory;            //0x60
    Light *ptLight;                    //0x64
    uint32_t _6[9];                    //0x68
    uint16_t wX;                        //0x8C
    uint16_t wY;                        //0x8E
    uint32_t _7;                        //0x90
    uint32_t dwOwnerType;                //0x94
    uint32_t dwOwnerId;                //0x98
    uint32_t _8[2];                    //0x9C
    OverheadMsg *pOMsg;                //0xA4
    Info *pInfo;                    //0xA8
    uint32_t _9[6];                    //0xAC
    uint32_t dwFlags;                    //0xC4
    uint32_t dwFlags2;                    //0xC8
    uint32_t _10[5];                    //0xCC
    UnitAny *pChangedNext;            //0xE0
    UnitAny *pRoomNext;                //0xE4
    UnitAny *pListNext;                //0xE8 -> 0xD8
};

#pragma pack(push)
#pragma pack(1)

typedef struct {
    uint32_t dwNPCClassId;
    uint32_t dwEntryAmount;
    uint16_t wEntryId1;
    uint16_t wEntryId2;
    uint16_t wEntryId3;
    uint16_t wEntryId4;
    uint16_t _1;
    uint32_t dwEntryFunc1;
    uint32_t dwEntryFunc2;
    uint32_t dwEntryFunc3;
    uint32_t dwEntryFunc4;
    uint8_t _2[5];
} NPCMenu;

struct OverheadMsg {
    uint32_t _1;
    uint32_t dwTrigger;
    uint32_t _2[2];
    char Msg[232];
};

#pragma pack(pop)

struct D2MSG {
    void *myHWND;
    char lpBuf[256];
};

struct InventoryLayout {
    uint8_t SlotWidth;
    uint8_t SlotHeight;
    uint8_t unk1;
    uint8_t unk2;
    uint32_t Left;
    uint32_t Right;
    uint32_t Top;
    uint32_t Bottom;
    uint8_t SlotPixelWidth;
    uint8_t SlotPixelHeight;
};

struct MpqTable {

};

struct sgptDataTable {
    MpqTable *pPlayerClass;
    uint32_t dwPlayerClassRecords;
    MpqTable *pBodyLocs;
    uint32_t dwBodyLocsRecords;
    MpqTable *pStorePage;
    uint32_t dwStorePageRecords;
    MpqTable *pElemTypes;
};
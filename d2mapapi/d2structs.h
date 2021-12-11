#pragma once

#include <cstdint>

namespace d2mapapi {

struct UnitAny;
struct Room1_112;
struct Room1_113;
struct Room2_112;
struct Room2_113;
struct Level112;
struct Level113;
struct Act112;
struct Act113;
struct ActMisc112;
struct ActMisc113;

union RoomTile;
union PresetUnit;
union Room1;
union Room2;
union Level;
union Act;
union ActMisc;

#pragma pack(push)
#pragma pack(1)

struct d2client_struct_112 {
    uint32_t dwInit;                    //0x00
    uint8_t _1[0x20C-4];                //0x04
    uint32_t fpInit;                    //0x20C
};

struct d2client_struct_113 {
    uint32_t dwInit;                    //0x00
    uint8_t _1[0x20D - 4];              //0x04
    uint32_t fpInit;                    //0x20D
};

struct CollMap {
    uint32_t dwPosGameX;                //0x00
    uint32_t dwPosGameY;                //0x04
    uint32_t dwSizeGameX;               //0x08
    uint32_t dwSizeGameY;               //0x0C
    uint32_t dwPosRoomX;                //0x10
    uint32_t dwPosRoomY;                //0x14
    uint32_t dwSizeRoomX;               //0x18
    uint32_t dwSizeRoomY;               //0x1C
    uint16_t *pMapStart;                //0x20
    uint16_t *pMapEnd;                  //0x24
};

struct RoomTile112 {
    uint32_t *nNum;                     //0x00
    Room2 *pRoom2;                      //0x04
    uint32_t _1[2];                     //0x08
    RoomTile *pNext;                    //0x10
};

struct RoomTile113 {
    Room2 *pRoom2;                      //0x00
    RoomTile *pNext;                    //0x04
    uint32_t _2[2];                     //0x08
    uint32_t *nNum;                     //0x10
};

struct PresetUnit112 {
    uint32_t dwTxtFileNo;               //0x00
    uint32_t _1[2];                     //0x04
    uint32_t dwPosX;                    //0x0C
    uint32_t _2;                        //0x10
    uint32_t dwPosY;                    //0x14
    PresetUnit *pPresetNext;            //0x18
    uint32_t dwType;                    //0x1C
};

struct PresetUnit113 {
    uint32_t _1;                        //0x00
    uint32_t dwTxtFileNo;               //0x04
    uint32_t dwPosX;                    //0x08
    PresetUnit *pPresetNext;            //0x0C
    uint32_t _3;                        //0x10
    uint32_t dwType;                    //0x14
    uint32_t dwPosY;                    //0x18
};

struct Level112 {
    uint8_t _1[0x50];                   //0x00
    uint32_t dwSeed[2];                 //0x50
    uint32_t _2;                        //0x58
    Level *pNextLevel;                  //0x5C
    uint32_t _56;                       //0x60
    ActMisc *pMisc;                     //0x64
    uint32_t _3;                        //0x68
    uint32_t dwPosX;                    //0x6C
    uint32_t dwPosY;                    //0x70
    uint32_t dwSizeX;                   //0x74
    uint32_t dwSizeY;                   //0x78
    uint32_t _4[6];                     //0x7C
    uint32_t dwLevelNo;                 //0x94
    uint32_t _5[0x61];                  //0x98
    Room2 *pRoom2First;                 //0x21C
};

struct Level113 {
    uint32_t _1[4];                     //0x00
    Room2 *pRoom2First;                 //0x10
    uint32_t _2[2];                     //0x14
    uint32_t dwPosX;                    //0x1C
    uint32_t dwPosY;                    //0x20
    uint32_t dwSizeX;                   //0x24
    uint32_t dwSizeY;                   //0x28
    uint32_t _3[96];                    //0x2C
    Level *pNextLevel;                  //0x1AC
    uint32_t _4;                        //0x1B0
    ActMisc *pMisc;                     //0x1B4
    uint32_t _5[6];                     //0x1BC
    uint32_t dwLevelNo;                 //0x1D0
};

struct Room2_112 {
    Level *pLevel;                      //0x00
    uint32_t _1;                        //0x04
    uint32_t dwRoomsNear;               //0x08
    RoomTile *pRoomTiles;               //0x0C
    Room2 **pRoom2Near;                 //0x10
    uint32_t _3[6];                     //0x14
    uint32_t dwPosX;                    //0x2C
    uint32_t dwPosY;                    //0x30
    uint32_t dwSizeX;                   //0x34
    uint32_t dwSizeY;                   //0x38
    uint32_t *pType2Info;               //0x3C
    uint32_t _4[0x20];                  //0x40
    uint32_t dwPresetType;              //0xC0
    PresetUnit *pPreset;                //0xC4
    uint32_t _5[0x3];                   //0xC8
    Room2 *pRoom2Next;                  //0xD4
    Room1 *pRoom1;                      //0xD8
};

struct Room2_113 {
    uint32_t _1[2];                     //0x00
    Room2 **pRoom2Near;                 //0x08
    uint32_t _2[6];                     //0x0C
    Room2 *pRoom2Next;                  //0x24
    uint32_t dwRoomFlags;               //0x28
    uint32_t dwRoomsNear;               //0x2C
    Room1 *pRoom1;                      //0x30
    uint32_t dwPosX;                    //0x34
    uint32_t dwPosY;                    //0x38
    uint32_t dwSizeX;                   //0x3C
    uint32_t dwSizeY;                   //0x40
    uint32_t _3;                        //0x44
    uint32_t dwPresetType;              //0x48
    RoomTile *pRoomTiles;               //0x4C
    uint32_t _4[2];                     //0x50
    Level *pLevel;                      //0x58
    PresetUnit *pPreset;                //0x5C
};

struct Room1_112 {
    Room1 **pRoomsNear;                 //0x00
    uint32_t _1[2];                     //0x04
    uint32_t dwSeed[2];                 //0x0C
    uint32_t _2;                        //0x14
    uint32_t dwXStart;                  //0x18
    uint32_t dwYStart;                  //0x1C
    uint32_t dwXSize;                   //0x20
    uint32_t dwYSize;                   //0x24
    uint32_t _3[0x4];                   //0x28
    Room1 *pRoomNext;                   //0x38
    uint32_t _4;                        //0x3C
    UnitAny *pUnitFirst;                //0x40
    uint32_t _5[3];                     //0x44
    CollMap *Coll;                      //0x50
    uint32_t _6[0x7];                   //0x54
    Room2 *pRoom2;                      //0x70
    uint32_t _7;                        //0x74
    uint32_t dwRoomsNear;               //0x78
};

struct Room1_113 {
    Room1 **pRoomsNear;                 //0x00
    uint32_t _1[3];                     //0x04
    Room2 *pRoom2;                      //0x10
    uint32_t _2[3];                     //0x14
    CollMap *Coll;                      //0x20
    uint32_t dwRoomsNear;               //0x24
    uint32_t _3[9];                     //0x28
    uint32_t dwPosX;                    //0x4C
    uint32_t dwPosY;                    //0x50
    uint32_t dwSizeX;                   //0x54
    uint32_t dwSizeY;                   //0x58
    uint32_t _4[6];                     //0x5C
    UnitAny *pUnitFirst;                //0x74
    uint32_t _5;                        //0x78
    Room1 *pRoomNext;                   //0x7C
};

struct ActMisc112 {
    uint32_t _1;                        //0x00
    Act *pAct;                          //0x04
    uint32_t _2[238];                   //0x3BC
    uint32_t dwStaffTombLevel;          //0x3C0
    uint32_t _3[43];                    //0x470
    Level *pLevelFirst;
};

struct ActMisc113 {
    uint32_t _1[37];                    //0x00
    uint32_t dwStaffTombLevel;          //0x94
    uint32_t _2[245];                   //0x98
    Act *pAct;                          //0x46C
    uint32_t _3[3];                     //0x470
    Level *pLevelFirst;                 //0x47C
};

struct Act112 {
    uint8_t _1[0x34];                   //0x00
    Room1 *pRoom1;                      //0x34
    ActMisc *pMisc;                     //0x38
    uint32_t _2[2];                     //0x40
    uint32_t dwAct;                     //0x44
};

struct Act113 {
    uint32_t _1[3];                     //0x00
    uint32_t dwMapSeed;                 //0x0C
    Room1 *pRoom1;                      //0x10
    uint32_t dwAct;                     //0x14
    uint32_t _2[12];                    //0x18
    ActMisc *pMisc;                     //0x48
};

#define GETTER_BY_VER(n) \
inline decltype(u113.n) n(bool is112a) { return is112a ? u112.n : u113.n; }

union d2client_struct {
    d2client_struct_112 u112;
    d2client_struct_113 u113;
};

union RoomTile {
    RoomTile112 u112;
    RoomTile113 u113;

    GETTER_BY_VER(pNext)
    GETTER_BY_VER(nNum)
    GETTER_BY_VER(pRoom2)
};

union PresetUnit {
    PresetUnit112 u112;
    PresetUnit113 u113;

    GETTER_BY_VER(pPresetNext)
    GETTER_BY_VER(dwType)
    GETTER_BY_VER(dwTxtFileNo)
    GETTER_BY_VER(dwPosX)
    GETTER_BY_VER(dwPosY)
};

union Room1 {
    Room1_112 u112;
    Room1_113 u113;

    GETTER_BY_VER(Coll)
};

union Room2 {
    Room2_112 u112;
    Room2_113 u113;

    GETTER_BY_VER(pRoom2Next)
    GETTER_BY_VER(pRoom1)
    GETTER_BY_VER(dwPosX)
    GETTER_BY_VER(dwPosY)
    GETTER_BY_VER(dwRoomsNear)
    GETTER_BY_VER(pRoom2Near)
    GETTER_BY_VER(pLevel)
    GETTER_BY_VER(dwSizeX)
    GETTER_BY_VER(dwSizeY)
    GETTER_BY_VER(pPreset)
    GETTER_BY_VER(pRoomTiles)
};

union Level {
    Level112 u112;
    Level113 u113;
    GETTER_BY_VER(pRoom2First)
    GETTER_BY_VER(dwLevelNo)
    GETTER_BY_VER(dwPosX)
    GETTER_BY_VER(dwPosY)
    GETTER_BY_VER(dwSizeX)
    GETTER_BY_VER(dwSizeY)
    GETTER_BY_VER(pNextLevel)
};

union ActMisc {
    ActMisc112 u112;
    ActMisc113 u113;
    GETTER_BY_VER(pLevelFirst)
};

union Act {
    Act112 u112;
    Act113 u113;
    GETTER_BY_VER(pMisc)
};

#pragma pack(pop)

}

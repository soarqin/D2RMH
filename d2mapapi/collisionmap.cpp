#include "collisionmap.h"

#include "d2map.h"
#include "d2ptrs.h"

#include <iostream>
#include <vector>

constexpr int unit_type_npc = 1;
constexpr int unit_type_object = 2;
constexpr int unit_type_tile = 5;

CollisionMap::CollisionMap(Act *act, unsigned int areaId) {
    this->act_ = act;
    this->areaId_ = areaId;
}

bool CollisionMap::build() {

    Level *pLevel = getLevel(act_->pMisc, areaId_);

    if (!pLevel) { return false; }
    if (!pLevel->pRoom2First) {
        D2COMMON_InitLevel(pLevel);
    }

    if (pLevel->pRoom2First) {
        levelOrigin.x = pLevel->dwPosX * 5;
        levelOrigin.y = pLevel->dwPosY * 5;

        const int width = pLevel->dwSizeX * 5;
        const int height = pLevel->dwSizeY * 5;
        map = std::vector<int16_t>(width * height, -1);
        totalWidth = width; totalHeight = height;

        for (Room2 *pRoom2 = pLevel->pRoom2First; pRoom2; pRoom2 = pRoom2->pRoom2Next) {
            bool bAdded = false;

            if (!pRoom2->pRoom1) {
                bAdded = true;
                D2COMMON_AddRoomData(act_, pLevel->dwLevelNo, pRoom2->dwPosX, pRoom2->dwPosY, NULL);
            }

            // levels near
            for (uint32_t i = 0; i < pRoom2->dwRoomsNear; i++) {
                if (pLevel->dwLevelNo != pRoom2->pRoom2Near[i]->pLevel->dwLevelNo) {
                    const auto originX = static_cast<int>(pRoom2->pRoom2Near[i]->pLevel->dwPosX * 5);
                    const auto originY = static_cast<int>(pRoom2->pRoom2Near[i]->pLevel->dwPosY * 5);
                    const auto origin = Point{originX, originY};
                    const auto newLevelWidth = pRoom2->pRoom2Near[i]->pLevel->dwSizeX * 5;
                    const auto newLevelHeight = pRoom2->pRoom2Near[i]->pLevel->dwSizeY * 5;

                    auto &adjacentLevel = adjacentLevels[pRoom2->pRoom2Near[i]->pLevel->dwLevelNo];
                    adjacentLevel.levelOrigin = origin;
                    adjacentLevel.width = newLevelWidth;
                    adjacentLevel.height = newLevelHeight;
                }
            }

            // add collision data
            if (pRoom2->pRoom1 && pRoom2->pRoom1->Coll) {
                const int x = pRoom2->pRoom1->Coll->dwPosGameX - levelOrigin.x;
                const int y = pRoom2->pRoom1->Coll->dwPosGameY - levelOrigin.y;
                const int cx = pRoom2->pRoom1->Coll->dwSizeGameX;
                const int cy = pRoom2->pRoom1->Coll->dwSizeGameY;
                const int nLimitX = x + cx;
                const int nLimitY = y + cy;

                uint16_t *p = pRoom2->pRoom1->Coll->pMapStart;
                if (cropX < 0 || x < cropX) cropX = x;
                if (cropY < 0 || y < cropY) cropY = y;
                if (cropX2 < 0 || nLimitX > cropX2) cropX2 = nLimitX;
                if (cropY2 < 0 || nLimitY > cropY2) cropY2 = nLimitY;
                for (int j = y; j < nLimitY; j++) {
                    int index = j * width + x;
                    for (int i = x; i < nLimitX; i++) {
                        map[index++] = *p++;
                    }
                }
            }

            // add unit data
            for (PresetUnit *pPresetUnit = pRoom2->pPreset; pPresetUnit; pPresetUnit = pPresetUnit->pPresetNext) {
                // npcs
                if (pPresetUnit->dwType == unit_type_npc) {
                    const auto npcX = static_cast<int>(pRoom2->dwPosX * 5 + pPresetUnit->dwPosX);
                    const auto npcY = static_cast<int>(pRoom2->dwPosY * 5 + pPresetUnit->dwPosY);
                    npcs[pPresetUnit->dwTxtFileNo].push_back(Point{npcX, npcY});
                }

                // objects
                if (pPresetUnit->dwType == unit_type_object) {
                    const auto objectX = static_cast<int>(pRoom2->dwPosX * 5 + pPresetUnit->dwPosX);
                    const auto objectY = static_cast<int>(pRoom2->dwPosY * 5 + pPresetUnit->dwPosY);
                    objects[pPresetUnit->dwTxtFileNo].push_back(Point{objectX, objectY});
                }

                // level exits
                if (pPresetUnit->dwType == unit_type_tile) {
                    for (RoomTile *pRoomTile = pRoom2->pRoomTiles; pRoomTile; pRoomTile = pRoomTile->pNext) {
                        if (*pRoomTile->nNum == pPresetUnit->dwTxtFileNo) {
                            const auto exitX = static_cast<int>(pRoom2->dwPosX * 5 + pPresetUnit->dwPosX);
                            const auto exitY = static_cast<int>(pRoom2->dwPosY * 5 + pPresetUnit->dwPosY);

                            adjacentLevels[pRoomTile->pRoom2->pLevel->dwLevelNo].exits.push_back(Point{exitX, exitY});
                        }
                    }
                }
            }

            if (bAdded) {
                D2COMMON_RemoveRoomData(act_, pLevel->dwLevelNo, pRoom2->dwPosX, pRoom2->dwPosY, NULL);
            }
        }
    }
    return true;
}
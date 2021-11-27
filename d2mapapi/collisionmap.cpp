#include "collisionmap.h"

#include "d2map.h"
#include "d2ptrs.h"

#include <iostream>
#include <set>
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

        std::set<Level*> nearLevels;
        for (Room2 *pRoom2 = pLevel->pRoom2First; pRoom2; pRoom2 = pRoom2->pRoom2Next) {
            bool bAdded = false;

            if (!pRoom2->pRoom1) {
                bAdded = true;
                D2COMMON_AddRoomData(act_, pLevel->dwLevelNo, pRoom2->dwPosX, pRoom2->dwPosY, NULL);
            }

            // levels near
            for (uint32_t i = 0; i < pRoom2->dwRoomsNear; i++) {
                if (pLevel->dwLevelNo != pRoom2->pRoom2Near[i]->pLevel->dwLevelNo) {
                    nearLevels.insert(pRoom2->pRoom2Near[i]->pLevel);
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

                            auto &al = adjacentLevels[pRoomTile->pRoom2->pLevel->dwLevelNo];
                            al.isWrap = true;
                            al.exits.push_back(Point{exitX, exitY});
                        }
                    }
                }
            }

            if (bAdded) {
                D2COMMON_RemoveRoomData(act_, pLevel->dwLevelNo, pRoom2->dwPosX, pRoom2->dwPosY, NULL);
            }
        }
        for (auto *level: nearLevels) {
            const auto originX = static_cast<int>(level->dwPosX * 5);
            const auto originY = static_cast<int>(level->dwPosY * 5);
            const auto origin = Point{originX, originY};
            const int newLevelWidth = level->dwSizeX * 5;
            const int newLevelHeight = level->dwSizeY * 5;

            auto &adjacentLevel = adjacentLevels[level->dwLevelNo];
            adjacentLevel.levelOrigin = origin;
            adjacentLevel.width = newLevelWidth;
            adjacentLevel.height = newLevelHeight;
            /* we have exits already */
            if (!adjacentLevel.exits.empty()) { continue; }
            /* caculate the path to the new level */
            if (width > 5) {
                if (origin.x < levelOrigin.x && origin.x + newLevelWidth == levelOrigin.x) {
                    /* Check near level on left-side */
                    int startY = -1, endY = -1;
                    int h0 = std::max(origin.y - levelOrigin.y, 0);
                    int h1 = std::min(origin.y + newLevelHeight - levelOrigin.y, height);
                    int index = h0 * width;
                    for (int i = h0; i < h1; ++i) {
                        if ((map[index] & 1) || (map[index + 5] & 1)) {
                            if (startY >= 0) {
                                endY = i;
                                break;
                            }
                        } else {
                            if (startY < 0) {
                                startY = i;
                            }
                        }
                        index += width;
                    }
                    adjacentLevel.exits.emplace_back(Point{levelOrigin.x, levelOrigin.y + (startY + endY) / 2});
                }
                if (origin.x > levelOrigin.x && origin.x == levelOrigin.x + width) {
                    /* Check near level on right-side */
                    int startY = -1, endY = -1;
                    int h0 = std::max(origin.y - levelOrigin.y, 0);
                    int h1 = std::min(origin.y + newLevelHeight - levelOrigin.y, height);
                    int index = h0 * width + (width - 1);
                    for (int i = h0; i < h1; ++i) {
                        if ((map[index] & 1) || (map[index - 5] & 1)) {
                            if (startY >= 0) {
                                endY = i;
                                break;
                            }
                        } else {
                            if (startY < 0) {
                                startY = i;
                            }
                        }
                        index += width;
                    }
                    adjacentLevel.exits.emplace_back(Point{levelOrigin.x + width - 1, levelOrigin.y + (startY + endY) / 2});
                }
            }
            if (height > 5) {
                if (origin.y < levelOrigin.y && origin.y + newLevelHeight == levelOrigin.y) {
                    /* Check near level on upside */
                    int startX = -1, endX = -1;
                    int w0 = std::max(origin.x - levelOrigin.x, 0);
                    int w1 = std::min(origin.x + newLevelWidth - levelOrigin.x, width);
                    for (int i = w0; i < w1; ++i) {
                        if ((map[i] & 1) || (map[i + 5 * width] & 1)) {
                            if (startX >= 0) {
                                endX = i;
                                break;
                            }
                        } else {
                            if (startX < 0) {
                                startX = i;
                            }
                        }
                    }
                    adjacentLevel.exits.emplace_back(Point{levelOrigin.x + (startX + endX) / 2, levelOrigin.y});
                }
                if (origin.y > levelOrigin.y && origin.y == levelOrigin.y + height) {
                    /* Check near level on downside */
                    int startX = -1, endX = -1;
                    int w0 = std::max(origin.x - levelOrigin.x, 0);
                    int w1 = std::min(origin.x + newLevelWidth - levelOrigin.x, width);
                    int index = width * (height - 1) + w0;
                    for (int i = w0; i < w1; ++i) {
                        if ((map[index] & 1) || map[index - 5 * width] & 1) {
                            if (startX >= 0) {
                                endX = i;
                                break;
                            }
                        } else {
                            if (startX < 0) {
                                startX = i;
                            }
                        }
                        ++index;
                    }
                    adjacentLevel.exits.emplace_back(Point{levelOrigin.x + (startX + endX) / 2, levelOrigin.y + height - 1});
                }
            }
        }
    }
    return true;
}

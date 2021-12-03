/*
 * Copyright (c) 2021 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#include "mapdata.h"

#include "d2map.h"
#include "d2ptrs.h"

#include <set>

namespace d2mapapi {


constexpr int unit_type_npc = 1;
constexpr int unit_type_object = 2;
constexpr int unit_type_tile = 5;

MapData::MapData(Act *act, unsigned int areaId) : CollisionMap(areaId) {
    Level *pLevel = getLevel(act->pMisc, areaId);

    if (!pLevel) { return; }
    if (!pLevel->pRoom2First) {
        D2COMMON_InitLevel(pLevel);
    }

    if (pLevel->pRoom2First) {
        offset.x = pLevel->dwPosX * 5;
        offset.y = pLevel->dwPosY * 5;

        const int width = pLevel->dwSizeX * 5;
        const int height = pLevel->dwSizeY * 5;
        auto map = std::vector<int16_t>(width * height, -1);
        size.width = width;
        size.height = height;

        std::set<Level *> nearLevels;
        for (Room2 *pRoom2 = pLevel->pRoom2First; pRoom2; pRoom2 = pRoom2->pRoom2Next) {
            bool bAdded = false;

            if (!pRoom2->pRoom1) {
                bAdded = true;
                D2COMMON_AddRoomData(act, pLevel->dwLevelNo, pRoom2->dwPosX, pRoom2->dwPosY, nullptr);
            }

            // levels near
            for (uint32_t i = 0; i < pRoom2->dwRoomsNear; i++) {
                if (pLevel->dwLevelNo != pRoom2->pRoom2Near[i]->pLevel->dwLevelNo) {
                    nearLevels.insert(pRoom2->pRoom2Near[i]->pLevel);
                }
            }

            // add collision data
            if (pRoom2->pRoom1 && pRoom2->pRoom1->Coll) {
                const int x = pRoom2->pRoom1->Coll->dwPosGameX - offset.x;
                const int y = pRoom2->pRoom1->Coll->dwPosGameY - offset.y;
                const int cx = pRoom2->pRoom1->Coll->dwSizeGameX;
                const int cy = pRoom2->pRoom1->Coll->dwSizeGameY;
                const int nLimitX = x + cx;
                const int nLimitY = y + cy;

                uint16_t *p = pRoom2->pRoom1->Coll->pMapStart;
                if (crop.x0 < 0 || x < crop.x0) crop.x0 = x;
                if (crop.y0 < 0 || y < crop.y0) crop.y0 = y;
                if (crop.x1 < 0 || nLimitX > crop.x1) crop.x1 = nLimitX;
                if (crop.y1 < 0 || nLimitY > crop.y1) crop.y1 = nLimitY;
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

                            auto &al = exits[pRoomTile->pRoom2->pLevel->dwLevelNo];
                            al.isPortal = true;
                            al.offsets.push_back(Point{exitX, exitY});
                        }
                    }
                }
            }

            if (bAdded) {
                D2COMMON_RemoveRoomData(act, pLevel->dwLevelNo, pRoom2->dwPosX, pRoom2->dwPosY, nullptr);
            }
        }
        for (auto *level: nearLevels) {
            const auto originX = static_cast<int>(level->dwPosX * 5);
            const auto originY = static_cast<int>(level->dwPosY * 5);
            const auto origin = Point{originX, originY};
            const int newLevelWidth = level->dwSizeX * 5;
            const int newLevelHeight = level->dwSizeY * 5;

            auto &adjacentLevel = exits[level->dwLevelNo];
            /* we have exits already */
            if (!adjacentLevel.offsets.empty()) { continue; }
            /* caculate the path to the new level */
            if (width > 5) {
                if (origin.x < offset.x && origin.x + newLevelWidth == offset.x) {
                    /* Check near level on left-side */
                    int startY = -1, endY = -1;
                    int h0 = std::max(origin.y - offset.y, 0);
                    int h1 = std::min(origin.y + newLevelHeight - offset.y, height);
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
                    adjacentLevel.offsets.emplace_back(Point{offset.x, offset.y + (startY + endY) / 2});
                }
                if (origin.x > offset.x && origin.x == offset.x + width) {
                    /* Check near level on right-side */
                    int startY = -1, endY = -1;
                    int h0 = std::max(origin.y - offset.y, 0);
                    int h1 = std::min(origin.y + newLevelHeight - offset.y, height);
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
                    adjacentLevel.offsets.emplace_back(Point{offset.x + width - 1, offset.y + (startY + endY) / 2});
                }
            }
            if (height > 5) {
                if (origin.y < offset.y && origin.y + newLevelHeight == offset.y) {
                    /* Check near level on upside */
                    int startX = -1, endX = -1;
                    int w0 = std::max(origin.x - offset.x, 0);
                    int w1 = std::min(origin.x + newLevelWidth - offset.x, width);
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
                    adjacentLevel.offsets.emplace_back(Point{offset.x + (startX + endX) / 2, offset.y});
                }
                if (origin.y > offset.y && origin.y == offset.y + height) {
                    /* Check near level on downside */
                    int startX = -1, endX = -1;
                    int w0 = std::max(origin.x - offset.x, 0);
                    int w1 = std::min(origin.x + newLevelWidth - offset.x, width);
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
                    adjacentLevel.offsets.emplace_back(Point{offset.x + (startX + endX) / 2, offset.y + height - 1});
                }
            }
        }

        /* run length encoding map data */
        mapData.clear();
        for (int j = crop.y0; j < crop.y1; ++j) {
            int index = j * width + crop.x0;
            bool lastIsWalkable = false;
            int count = 0;
            for (int i = crop.x0; i < crop.x1; ++i) {
                bool walkable = !(map[index++] & 1);
                if (walkable == lastIsWalkable) {
                    ++count;
                    continue;
                }
                mapData.emplace_back(count);
                count = 1;
                lastIsWalkable = walkable;
            }
            mapData.emplace_back(count);
            mapData.emplace_back(-1);
        }
    }
    built = true;
}

}

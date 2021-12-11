/*
 * Copyright (c) 2021 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#include "mapdata.h"

#include "offset.h"
#include "d2map.h"
#include "d2ptrs.h"

#include <algorithm>
#include <vector>
#include <tuple>

namespace d2mapapi {

constexpr int unit_type_npc = 1;
constexpr int unit_type_object = 2;
constexpr int unit_type_tile = 5;

MapData::MapData(Act *act, unsigned int areaId) : CollisionMap(areaId) {
    bool is112a = getD2Version() == D2_112a;
    Level *pLevel = getLevel(act, areaId);

    if (!pLevel) { return; }

    unsigned int currLevelNo;
    unsigned int currPosX;
    unsigned int currPosY;
    unsigned int width, height;
    if (!pLevel->pRoom2First(is112a)) {
        D2COMMON_InitLevel(pLevel);
    }
    built = true;
    if (!pLevel->pRoom2First(is112a)) { return; }
    currLevelNo = pLevel->dwLevelNo(is112a);
    currPosX = pLevel->dwPosX(is112a);
    currPosY = pLevel->dwPosY(is112a);
    width = pLevel->dwSizeX(is112a) * 5;
    height = pLevel->dwSizeY(is112a) * 5;
    offset.x = currPosX * 5;
    offset.y = currPosY * 5;

    auto map = std::vector<int16_t>(width * height, -1);
    size.width = width;
    size.height = height;

    std::vector<std::tuple<uint32_t, int, int, int>> sides;
    for (Room2 *pRoom2 = pLevel->pRoom2First(is112a); pRoom2; pRoom2 = pRoom2->pRoom2Next(is112a)) {
        bool bAdded = false;
        auto roomPosX = pRoom2->dwPosX(is112a);
        auto roomPosY = pRoom2->dwPosY(is112a);

        if (!pRoom2->pRoom1(is112a)) {
            bAdded = true;
            D2COMMON_AddRoomData(act, pLevel->dwLevelNo(is112a), roomPosX, roomPosY, nullptr);
        }

        /* Check near levels' walkable rect (we check 2 pixels from edge)
         * side: 0-left 1-right 2-top 3-bottom
         */
        for (uint32_t i = 0; i < pRoom2->dwRoomsNear(is112a); i++) {
            int side = -1;
            auto *pRoom2Near = pRoom2->pRoom2Near(is112a)[i];
            auto nearLevelNo = pRoom2Near->pLevel(is112a)->dwLevelNo(is112a);
            if (currLevelNo == nearLevelNo) { continue; }
            auto nearPosX = pRoom2Near->dwPosX(is112a);
            auto nearPosY = pRoom2Near->dwPosY(is112a);
            auto nearSizeX = pRoom2Near->dwSizeX(is112a);
            auto nearSizeY = pRoom2Near->dwSizeY(is112a);
            auto roomSizeX = pRoom2->dwSizeX(is112a);
            auto roomSizeY = pRoom2->dwSizeY(is112a);
            if (nearPosX + nearSizeX == roomPosX && nearPosY == roomPosY) {
                side = 0;
            } else if (nearPosX == roomPosX + roomSizeX && nearPosY == roomPosY) {
                side = 1;
            } else if (nearPosY + nearSizeY == roomPosY && nearPosX == roomPosX) {
                side = 2;
            } else if (nearPosY == roomPosY + roomSizeY && nearPosX == roomPosX) {
                side = 3;
            }
            if (side < 0) { continue; }
            bool bAddedNear = false;
            if (!pRoom2Near->pRoom1(is112a)) {
                D2COMMON_AddRoomData(act, pRoom2Near->pLevel(is112a)->dwLevelNo(is112a), nearPosX, nearPosY, nullptr);
                bAddedNear = true;
            }
            int sideStart = -1;
            Room1 *room1;
            CollMap *coll;
            if ((room1 = pRoom2Near->pRoom1(is112a)) && (coll = room1->Coll(is112a))) {
                uint16_t *p = coll->pMapStart;
                auto w = coll->dwSizeGameX, h = coll->dwSizeGameY;
                switch (side) {
                case 0:
                    p += w - 1;
                    for (int z = 0; z < h; z++) {
                        if ((*p & 1) || (*(p-1) & 1)) {
                            if (sideStart >= 0) {
                                sides.emplace_back(nearLevelNo, side, coll->dwPosGameY + sideStart, coll->dwPosGameY + z);
                                sideStart = -1;
                            }
                        } else {
                            if (sideStart < 0) {
                                sideStart = z;
                            }
                        }
                        p += w;
                    }
                    break;
                case 1:
                    for (int z = 0; z < h; z++) {
                        if ((*p & 1) || (*(p+1) & 1)) {
                            if (sideStart >= 0) {
                                sides.emplace_back(nearLevelNo, side, coll->dwPosGameY + sideStart, coll->dwPosGameY + z);
                                sideStart = -1;
                            }
                        } else {
                            if (sideStart < 0) {
                                sideStart = z;
                            }
                        }
                        p += w;
                    }
                    break;
                case 2:
                    p += w * (h - 1);
                    for (int z = 0; z < w; z++) {
                        if ((*p & 1) || (*(p-w) & 1)) {
                            if (sideStart >= 0) {
                                sides.emplace_back(nearLevelNo, side, coll->dwPosGameX + sideStart, coll->dwPosGameX + z);
                                sideStart = -1;
                            }
                        } else {
                            if (sideStart < 0) {
                                sideStart = z;
                            }
                        }
                        p++;
                    }
                    break;
                case 3:
                    for (int z = 0; z < w; z++) {
                        if ((*p & 1) || (*(p+w) & 1)) {
                            if (sideStart >= 0) {
                                sides.emplace_back(nearLevelNo, side, coll->dwPosGameX + sideStart, coll->dwPosGameX + z);
                                sideStart = -1;
                            }
                        } else {
                            if (sideStart < 0) {
                                sideStart = z;
                            }
                        }
                        p++;
                    }
                    break;
                default: break;
                }
                if (sideStart >= 0) {
                    if (side == 2 || side == 3) {
                        sides.emplace_back(nearLevelNo, side, coll->dwPosGameX + sideStart, coll->dwPosGameX + w);
                    } else {
                        sides.emplace_back(nearLevelNo, side, coll->dwPosGameY + sideStart, coll->dwPosGameY + h);
                    }
                }
            }
            if (bAddedNear) {
                D2COMMON_RemoveRoomData(act, pRoom2Near->pLevel(is112a)->dwLevelNo(is112a), nearPosX, nearPosY, nullptr);
            }
        }

        // add collision data
        Room1 *room1;
        CollMap *coll;
        if ((room1 = pRoom2->pRoom1(is112a)) && (coll = room1->Coll(is112a))) {
            const int x = coll->dwPosGameX - offset.x;
            const int y = coll->dwPosGameY - offset.y;
            const int cx = coll->dwSizeGameX;
            const int cy = coll->dwSizeGameY;
            const int nLimitX = x + cx;
            const int nLimitY = y + cy;

            uint16_t *p = coll->pMapStart;
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
        for (PresetUnit *pPresetUnit = pRoom2->pPreset(is112a); pPresetUnit; pPresetUnit = pPresetUnit->pPresetNext(is112a)) {
            // npcs
            auto type = pPresetUnit->dwType(is112a);
            if (type == unit_type_npc) {
                const auto npcX = static_cast<int>(roomPosX * 5 + pPresetUnit->dwPosX(is112a));
                const auto npcY = static_cast<int>(roomPosY * 5 + pPresetUnit->dwPosY(is112a));
                npcs[pPresetUnit->dwTxtFileNo(is112a)].push_back(Point{npcX, npcY});
            }

            // objects
            if (type == unit_type_object) {
                const auto objectX = static_cast<int>(roomPosX * 5 + pPresetUnit->dwPosX(is112a));
                const auto objectY = static_cast<int>(roomPosY * 5 + pPresetUnit->dwPosY(is112a));
                objects[pPresetUnit->dwTxtFileNo(is112a)].push_back(Point{objectX, objectY});
            }

            // level exits
            if (type == unit_type_tile) {
                auto txtFileNo = pPresetUnit->dwTxtFileNo(is112a);
                auto presetPosX = pPresetUnit->dwPosX(is112a);
                auto presetPosY = pPresetUnit->dwPosY(is112a);
                for (RoomTile *pRoomTile = pRoom2->pRoomTiles(is112a); pRoomTile; pRoomTile = pRoomTile->pNext(is112a)) {
                    if (*pRoomTile->nNum(is112a) == txtFileNo) {
                        const auto exitX = static_cast<int>(roomPosX * 5 + presetPosX);
                        const auto exitY = static_cast<int>(roomPosY * 5 + presetPosY);

                        auto &al = exits[pRoomTile->pRoom2(is112a)->pLevel(is112a)->dwLevelNo(is112a)];
                        al.isPortal = true;
                        al.offsets.emplace_back(Point{exitX, exitY});
                    }
                }
            }
        }

        if (bAdded) {
            D2COMMON_RemoveRoomData(act, currLevelNo, roomPosX, roomPosY, nullptr);
        }
    }

    std::sort(sides.begin(), sides.end());
    std::vector<std::tuple<uint32_t, int, int, int>> realSides;
    uint32_t lastNearLevelNo = 0;
    int lastSide = -1, start = -1, end = -1;
    for (auto [nearLevelNo, side, sideStart, sideEnd]: sides) {
/*
        fprintf(stderr, "O: %u %d %d %d\n", nearLevelNo, side, sideStart, sideEnd);
        fflush(stderr);
*/
        if (lastNearLevelNo != nearLevelNo || lastSide != side) {
            if (start >= 0) {
                realSides.emplace_back(lastNearLevelNo, lastSide, start, end);
            }
            lastSide = side;
            lastNearLevelNo = nearLevelNo;
            start = -1;
            end = -1;
        }
        if (start == -1) {
            start = sideStart;
            end = sideEnd;
        } else if (sideStart == end) {
            end = sideEnd;
        } else {
            realSides.emplace_back(lastNearLevelNo, lastSide, start, end);
            start = sideStart;
            end = sideEnd;
        }
    }
    if (start >= 0) {
        realSides.emplace_back(lastNearLevelNo, lastSide, start, end);
    }
    sides.clear();
    for (auto [nearLevelNo, side, sideStart, sideEnd]: realSides) {
/*
        fprintf(stderr, "R: %u %d %d %d\n", nearLevelNo, side, sideStart, sideEnd);
        fflush(stderr);
*/
        int sStart = -1;
        switch (side) {
        case 0: {
            sideStart -= offset.y;
            if (sideStart < crop.y0) sideStart = crop.y0;
            sideEnd -= offset.y;
            if (sideEnd > crop.y1) sideEnd = crop.y1;
            if (sideStart == sideEnd) break;
            auto w = crop.x1 - crop.x0;
            auto *p = map.data() + (sideStart - crop.y0) * w;
            for (auto z = sideStart; z < sideEnd; ++z) {
                if ((*p & 1) || (*(p+1) & 1)) {
                    if (sStart >= 0) {
                        sides.emplace_back(nearLevelNo, side, sStart, z);
                        sStart = -1;
                    }
                } else {
                    if (sStart < 0) {
                        sStart = z;
                    }
                }
                p += w;
            }
            break;
        }
        case 1: {
            sideStart -= offset.y;
            if (sideStart < crop.y0) sideStart = crop.y0;
            sideEnd -= offset.y;
            if (sideEnd > crop.y1) sideEnd = crop.y1;
            if (sideStart == sideEnd) break;
            auto w = crop.x1 - crop.x0;
            auto *p = map.data() + (sideStart - crop.y0) * w + (w - 1);
            for (auto z = sideStart; z < sideEnd; ++z) {
                if ((*p & 1) || (*(p-1) & 1)) {
                    if (sStart >= 0) {
                        sides.emplace_back(nearLevelNo, side, sStart, z);
                        sStart = -1;
                    }
                } else {
                    if (sStart < 0) {
                        sStart = z;
                    }
                }
                p += w;
            }
            break;
        }
        case 2: {
            sideStart -= offset.x;
            if (sideStart < crop.x0) sideStart = crop.x0;
            sideEnd -= offset.x;
            if (sideEnd > crop.x1) sideEnd = crop.x1;
            if (sideStart == sideEnd) break;
            auto w = crop.x1 - crop.x0;
            auto *p = map.data() + (sideStart - crop.x0);
            for (auto z = sideStart; z < sideEnd; ++z) {
                if ((*p & 1) || (*(p+w) & 1)) {
                    if (sStart >= 0) {
                        sides.emplace_back(nearLevelNo, side, sStart, z);
                        sStart = -1;
                    }
                } else {
                    if (sStart < 0) {
                        sStart = z;
                    }
                }
                p++;
            }
            break;
        }
        case 3: {
            sideStart -= offset.x;
            if (sideStart < crop.x0) sideStart = crop.x0;
            sideEnd -= offset.x;
            if (sideEnd > crop.x1) sideEnd = crop.x1;
            if (sideStart == sideEnd) break;
            auto w = crop.x1 - crop.x0;
            auto *p = map.data() + w * (crop.y1 - crop.y0 - 1) + (sideStart - crop.x0);
            for (auto z = sideStart; z < sideEnd; ++z) {
                if ((*p & 1) || (*(p-w) & 1)) {
                    if (sStart >= 0) {
                        sides.emplace_back(nearLevelNo, side, sStart, z);
                        sStart = -1;
                    }
                } else {
                    if (sStart < 0) {
                        sStart = z;
                    }
                }
                p++;
            }
            break;
        }
        default:
            break;
        }
        if (sStart >= 0) {
            sides.emplace_back(nearLevelNo, side, sStart, sideEnd);
        }
    }
    for (auto [nearLevelNo, side, sideStart, sideEnd]: sides) {
        if (sideStart + 2 >= sideEnd) { continue; }
/*
        fprintf(stderr, "F: %u %d %d %d\n", nearLevelNo, side, sideStart, sideEnd);
        fflush(stderr);
*/
        switch (side) {
        case 0:
            exits[nearLevelNo].offsets.emplace_back(Point{offset.x + crop.x0, offset.y + (sideStart + sideEnd) / 2});
            break;
        case 1:
            exits[nearLevelNo].offsets.emplace_back(Point{offset.x + crop.x1 - 1, offset.y + (sideStart + sideEnd) / 2});
            break;
        case 2:
            exits[nearLevelNo].offsets.emplace_back(Point{offset.x + (sideStart + sideEnd) / 2, offset.y + crop.y0});
            break;
        case 3:
            exits[nearLevelNo].offsets.emplace_back(Point{offset.x + (sideStart + sideEnd) / 2, offset.y + crop.y1 - 1});
            break;
        default:
            break;
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

}

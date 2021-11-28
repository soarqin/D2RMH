/*
 * Copyright (c) 2021 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#include "data.h"

#include "d2rdefs.h"
#include "ini.h"
#include "util.h"

#include <algorithm>
#include <cstring>
#include <cstdlib>

static Data sgamedata;
const Data *gamedata = &sgamedata;

/* itemFilters[id][quality-1][ethereal][sockets] = style | (color << 4) | (sound << 8) */
static uint16_t itemFilters[1000][8][2][7] = {};

static inline uint32_t strToUInt(const std::string &str, bool searchItemCode) {
    if (!searchItemCode) {
        return uint32_t(strtoul(str.c_str(), nullptr, 0));
    }
    auto ite = sgamedata.itemIdByCode.find(str);
    if (ite != sgamedata.itemIdByCode.end()) {
        return ite->second;
    }
    uint32_t id = 0;
    for (auto c: str) {
        if (c < '0' || c > '9') { return 0; }
        id = id * 10 + (c - '0');
    }
    return id;
}

static inline std::vector<std::pair<uint32_t, uint32_t>> calcRanges(const std::string &str, bool searchItemCode, uint32_t maxValue) {
    std::vector<std::pair<uint32_t, uint32_t>> result;
    auto vec = splitString(str, ',');
    for (const auto &v: vec) {
        auto vec2 = splitString(v, '-');
        if (vec2.empty()) {
            continue;
        }
        auto startPos = strToUInt(vec2[0], searchItemCode);
        uint32_t endPos;
        if (vec2.size() > 1) {
            endPos = strToUInt(vec2[1], searchItemCode);
            if (endPos < startPos) endPos = startPos;
        } else {
            endPos = startPos;
        }
        if (startPos >= maxValue) {
            startPos = 0;
            endPos = maxValue - 1;
        } else {
            if (endPos >= maxValue) endPos = maxValue - 1;
        }
        result.emplace_back(startPos, endPos);
    }
    return result;
}

static void loadItemFilter(const char *key, const char *value) {
    auto res = uint16_t(strtoul(value, nullptr, 0));
    if (res) {
        auto *sub = strchr(value, ',');
        if (sub) {
            char *endptr = nullptr;
            res |= uint16_t(std::min(15ul, strtoul(sub + 1, &endptr, 0)) << 4);
            if (endptr && *endptr == ',') {
                res |= uint16_t(std::min(255ul, strtoul(endptr + 1, nullptr, 0)) << 8);
            }
        }
    }

    std::vector<std::pair<uint32_t, uint32_t>> ranges[4] = {};
    const uint32_t maxValues[4] = {1000, 8, 2, 7};
    size_t index = 0;
    while (true) {
        auto pos = strcspn(key, "+*#");
        ranges[index] = calcRanges(std::string(key, pos), index == 0, maxValues[index]);
        if (!key[pos]) { break; }
        switch (key[pos]) {
        case '+': index = 1; break;
        case '#': index = 2; break;
        case '*': index = 3; break;
        default: break;
        }
        key += pos + 1;
    }
    for (int i = 0; i < 4; ++i) {
        if (ranges[i].empty()) {
            ranges[i].emplace_back(0, maxValues[i] - 1);
        }
    }
    for (auto r0: ranges[0]) {
        for (auto i = r0.first; i <= r0.second; ++i) {
            for (auto r1: ranges[1]) {
                for (auto j = r1.first; j <= r1.second; ++j) {
                    for (auto r2: ranges[2]) {
                        for (auto k = r2.first; k <= r2.second; ++k) {
                            for (auto r3: ranges[3]) {
                                for (auto l = r3.first; l <= r3.second; ++l) {
                                    itemFilters[i][j][k][l] = res;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void loadData() {
    int section = -1;
    ini_parse("D2RMH_data.ini", [](void* user, const char* section,
                                   const char* name, const char* value)->int {
        auto *isec = (int*)user;
        if (!name) {
            if (!strcmp(section, "guides")) { *isec = 0; }
            else if (!strcmp(section, "levels")) { *isec = 1; }
            else if (!strcmp(section, "objects")) { *isec = 2; }
            else if (!strcmp(section, "npcs")) { *isec = 3; }
            else if (!strcmp(section, "monsters")) { *isec = 4; }
            else if (!strcmp(section, "shrines")) { *isec = 5; }
            else if (!strcmp(section, "superuniques")) { *isec = 6; }
            else if (!strcmp(section, "items")) { *isec = 7; }
            else if (!strcmp(section, "mercnames")) { *isec = 8; }
            else if (!strcmp(section, "strings")) { *isec = 9; }
            else { *isec = -1; }
            return 1;
        }
        switch (*isec) {
        case 0: {
            int from = strtol(name, nullptr, 0);
            int to;
            if (value[0] == '+') {
                to = strtol(value + 1, nullptr, 0) | 0x10000;
            } else if (value[0] == '-') {
                to = strtol(value + 1, nullptr, 0) | 0x20000;
            } else {
                to = strtol(value, nullptr, 0);
            }
            sgamedata.guides[from].insert(to);
            break;
        }
        case 1: {
            auto id = strtoul(name, nullptr, 0);
            if (id >= sgamedata.levels.size()) {
                sgamedata.levels.resize(id + 1);
            }
            sgamedata.levels[id] = { value, nullptr };
            break;
        }
        case 2: case 3: {
            auto sl = splitString(value, '|');
            if (sl.size() < 2) { break; }
            EObjType t = TypeNone;
            if (sl[0] == "Waypoint") { t = TypeWayPoint; }
            else if (sl[0] == "Quest") { t = TypeQuest; }
            else if (sl[0] == "Portal") { t = TypePortal; }
            else if (sl[0] == "Chest") { t = TypeChest; }
            else if (sl[0] == "Shrine") { t = TypeShrine; }
            else if (sl[0] == "Well") { t = TypeWell; }
            else if (sl[0] == "Door") { t = TypeDoor; }
            if (*isec == 2) {
                float x = 0, y = 0;
                bool isDoor = t == TypeDoor;
                if (isDoor || sl.size() > 2) {
                    auto sl2 = splitString(sl[isDoor ? 1 : 2], ',');
                    if (sl2.size() > 1) {
                        x = std::strtof(sl2[0].c_str(), nullptr);
                        y = std::strtof(sl2[1].c_str(), nullptr);
                    }
                }
                sgamedata.objects[0][strtol(name, nullptr, 0)] = {t, sl[1], nullptr, x, y};
            } else {
                sgamedata.objects[1][strtol(name, nullptr, 0)] = {t, sl[1], nullptr, 2.f, 2.f};
            }
            break;
        }
        case 4: {
            auto id = strtoul(name, nullptr, 0);
            if (id >= sgamedata.monsters.size()) {
                sgamedata.monsters.resize(id + 1);
            }
            const char *pos = strchr(value, '|');
            if (pos) {
                sgamedata.monsters[id] = { std::string(value, pos), uint8_t(strtoul(pos + 1, nullptr, 0)), nullptr };
            } else {
                sgamedata.monsters[id] = { value, false, nullptr };
            }
            break;
        }
        case 5: {
            auto id = strtoul(name, nullptr, 0);
            if (id >= sgamedata.shrines.size()) {
                sgamedata.shrines.resize(id + 1);
            }
            sgamedata.shrines[id] = { value, nullptr };
            break;
        }
        case 6: {
            auto id = strtoul(name, nullptr, 0);
            if (id >= sgamedata.superUniques.size()) {
                sgamedata.superUniques.resize(id + 1);
            }
            sgamedata.superUniques[id] = { value, nullptr };
            break;
        }
        case 7: {
            auto id = strtoul(name, nullptr, 0);
            if (id >= sgamedata.items.size()) {
                sgamedata.items.resize(id + 1);
            }
            const char *pos = strchr(value, '|');
            if (pos) {
                sgamedata.items[id] = { pos + 1, nullptr };
                sgamedata.itemIdByCode[std::string(value, pos)] = id;
            }
            break;
        }
        case 8: {
            auto id = strtoul(name, nullptr, 0);
            if (id >= 65535) { break; }
            if (id >= sgamedata.mercNames.size()) { sgamedata.mercNames.resize(id + 1); }
            sgamedata.mercNames[id].first = value;
            break;
        }
        case 9: {
            const char *pos = strchr(name, '[');
            if (!pos) { break; }
            auto index = strtoul(pos + 1, nullptr, 0);
            if (index < 0 || index >= 13) { break; }
            char realname[256];
            auto ssize = pos - name;
            memcpy(realname, name, ssize);
            realname[ssize] = 0;
            sgamedata.strings[realname][index] = utf8toucs4(value);
            break;
        }
        default:
            break;
        }
        return 1;
    }, &section);
    section = -1;
    ini_parse("D2RMH_item.ini", [](void* user, const char* section,
                                   const char* name, const char* value)->int {
        auto *isec = (int*)user;
        if (!name) {
            if (!strcmp(section, "items")) { *isec = 0; }
            else { *isec = -1; }
            return 1;
        }
        switch (*isec) {
        case 0: {
            loadItemFilter(name, value);
            break;
        }
        default:
            break;
        }
        return 1;
    }, &section);

    for (auto *vec: {&sgamedata.levels, &sgamedata.shrines, &sgamedata.superUniques, &sgamedata.items}) {
        for (auto &p: *vec) {
            auto ite = sgamedata.strings.find(p.first);
            if (ite == sgamedata.strings.end()) { continue; }
            p.second = &ite->second;
        }
    }
    for (auto &p: sgamedata.monsters) {
        auto ite = sgamedata.strings.find(std::get<0>(p));
        if (ite == sgamedata.strings.end()) { continue; }
        std::get<2>(p) = &ite->second;
    }
    for (auto &vec: sgamedata.objects) {
        for (auto &p: vec) {
            auto ite = sgamedata.strings.find(std::get<1>(p.second));
            if (ite == sgamedata.strings.end()) { continue; }
            std::get<2>(p.second) = &ite->second;
        }
    }
    for (auto &p: sgamedata.mercNames) {
        if (p.first.empty()) { continue; }
        auto ite = sgamedata.strings.find(p.first);
        if (ite == sgamedata.strings.end()) {
            p.second = nullptr;
        } else {
            p.second = &ite->second;
        }
    }
}

uint16_t filterItem(const UnitAny *unit, const ItemData *item, uint32_t sockets) {
    if (sockets > 6) { return 0; }
    auto id = unit->txtFileNo;
    if (id >= 1000) { return 0; }
    auto quality = item->quality - 1;
    if (quality >= 8) { return 0; }
    auto ethereal = (item->itemFlags & 0x00400000) ? 1 : 0;
    return itemFilters[id][quality][ethereal][sockets];
}

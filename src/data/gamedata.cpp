/*
 * Copyright (c) 2021 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#include "gamedata.h"

#include "viewstream.h"
#include "d2txt.h"
#include "d2r/storage.h"
#include "d2r/d2rdefs.h"
#include "util/util.h"

#include "ini.h"
#include "json/json.hpp"

#include <algorithm>
#include <cstring>
#include <cstdlib>

namespace data {

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

static inline std::vector<std::pair<uint32_t, uint32_t>> calcRanges(const std::string &str,
                                                                    bool searchItemCode,
                                                                    uint32_t maxValue) {
    std::vector<std::pair<uint32_t, uint32_t>> result;
    auto vec = util::splitString(str, ',');
    for (const auto &v: vec) {
        auto vec2 = util::splitString(v, '-');
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
        case '+': index = 1;
            break;
        case '#': index = 2;
            break;
        case '*': index = 3;
            break;
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

inline void assignJsonLngArray(Data::LngString &arr, nlohmann::basic_json<> &j) {
#define ASSIGN(n) arr[LNG_##n] = util::utf8toucs4(j[#n].get<std::string>())
    ASSIGN(enUS);
    ASSIGN(zhTW);
    ASSIGN(deDE);
    ASSIGN(esES);
    ASSIGN(frFR);
    ASSIGN(itIT);
    ASSIGN(koKR);
    ASSIGN(plPL);
    ASSIGN(esMX);
    ASSIGN(jaJP);
    ASSIGN(ptBR);
    ASSIGN(ruRU);
    ASSIGN(zhCN);
#undef ASSIGN
}

inline void loadJsonLng(std::unordered_map<std::string, Data::LngString> &jlng, d2r::Storage &storage, const char *filename) {
    std::vector<uint8_t> mem;
    if (storage.readFile(filename, mem)) {
        std::string_view sv(reinterpret_cast<const char*>(mem.data()), mem.size());
        view_istream<char> stm(sv);
        nlohmann::json j;
        stm >> j;
        for (auto j2: j) {
            auto key = j2["Key"].get<std::string>();
            auto &arr = jlng[key];
            assignJsonLngArray(arr, j2);
        }
    }
}

inline void loadJsonLngById(std::unordered_map<uint32_t, Data::LngString> &jlng, d2r::Storage &storage, const char *filename) {
    std::vector<uint8_t> mem;
    if (storage.readFile(filename, mem)) {
        std::string_view sv(reinterpret_cast<const char*>(mem.data()), mem.size());
        view_istream<char> stm(sv);
        nlohmann::json j;
        stm >> j;
        for (auto j2: j) {
            auto key = j2["id"].get<uint32_t>();
            auto &arr = jlng[key];
            assignJsonLngArray(arr, j2);
        }
    }
}

inline void loadTxt(D2TXT &txt, d2r::Storage &storage, const char *filename) {
    std::vector<uint8_t> mem;
    if (storage.readFile(filename, mem)) {
        txt.load(mem.data(), mem.size());
    }
}

template<typename T>
inline Data::LngString *moveStrings(std::unordered_map<T, Data::LngString> &to, std::unordered_map<T, Data::LngString> &from, const T &key) {
    auto ite = from.find(key);
    if (ite == from.end()) {
        return nullptr;
    }
    auto &result = to[key];
    result = std::move(ite->second);
    from.erase(ite);
    return &result;
}

inline EObjType objTypeFromString(const std::string &key) {
    if (key == "Waypoint") { return TypeWayPoint; }
    if (key == "Quest") { return TypeQuest; }
    if (key == "Portal") { return TypePortal; }
    if (key == "Chest") { return TypeChest; }
    if (key == "Shrine") { return TypeShrine; }
    if (key == "Well") { return TypeWell; }
    if (key == "Door") { return TypeDoor; }
    return TypeNone;
}

inline void loadD2RData() {
    struct UserParseData {
        /* -1-not used  0-guides  1-useful_names  2-useful_objects  3-name_replace */
        int section = -1;
        std::map<int, std::pair<std::string, std::string>> usefulObjectOp, usefulObjects, usefulNpcs;
        std::map<std::string, std::set<std::string>> guides;
    } parseData;
    ini_parse("D2RMH_gamedata.ini", [](void* user, const char* section, const char* name, const char* value)->int {
        auto *pd = (UserParseData*)user;
        if (!name) {
            if (!strcmp(section, "guides")) {
                pd->section = 0;
            } else if (!strcmp(section, "useful_object_op")) {
                pd->section = 1;
            } else if (!strcmp(section, "useful_objects")) {
                pd->section = 2;
            } else if (!strcmp(section, "useful_npcs")) {
                pd->section = 3;
            } else {
                pd->section = -1;
            }
            return 1;
        }
        switch (pd->section) {
        case 0: {
            if (strtoul(value, nullptr, 0) == 0) {
                break;
            }
            if (strncmp(name, "guide[", 6) != 0) {
                break;
            }
            const char *start = name + 6;
            const char *token = strchr(name, ']');
            if (token == nullptr) { break; }
            const char *start2 = strchr(token + 1, '[');
            if (start2 == nullptr) { break; }
            ++start2;
            const char *token2 = strchr(start2, ']');
            if (token2 == nullptr) { break; }
            pd->guides[std::string(start, token)].insert(std::string(start2, token2));
            break;
        }
        case 1: {
            const char *token = strchr(value, ',');
            pd->usefulObjectOp[strtol(name, nullptr, 0)] = token == nullptr ? std::make_pair(value, "")
                                                                            : std::make_pair(std::string(value, token), token + 1);
            break;
        }
        case 2: {
            const char *token = strchr(value, ',');
            pd->usefulObjects[strtol(name, nullptr, 0)] = token == nullptr ? std::make_pair(value, "")
                                                                           : std::make_pair(std::string(value, token), token + 1);
            break;
        }
        case 3: {
            const char *token = strchr(value, ',');
            pd->usefulNpcs[strtol(name, nullptr, 0)] = token == nullptr ? std::make_pair(value, "")
                                                                        : std::make_pair(std::string(value, token), token + 1);
            break;
        }
        default:
            break;
        }
        return 1;
    }, &parseData);

    std::unordered_map<std::string, Data::LngString> names;
    std::unordered_map<uint32_t, Data::LngString> mercNames;
    loadJsonLng(names, d2r::storage, "data:data/local/lng/strings/item-names.json");
    loadJsonLng(names, d2r::storage, "data:data/local/lng/strings/item-runes.json");
    loadJsonLng(names, d2r::storage, "data:data/local/lng/strings/item-nameaffixes.json");
    loadJsonLng(names, d2r::storage, "data:data/local/lng/strings/item-modifiers.json");
    loadJsonLng(names, d2r::storage, "data:data/local/lng/strings/levels.json");
    loadJsonLng(names, d2r::storage, "data:data/local/lng/strings/monsters.json");
    loadJsonLng(names, d2r::storage, "data:data/local/lng/strings/npcs.json");
    loadJsonLng(names, d2r::storage, "data:data/local/lng/strings/objects.json");
    loadJsonLng(names, d2r::storage, "data:data/local/lng/strings/shrines.json");
    loadJsonLng(names, d2r::storage, "data:data/local/lng/strings/ui.json");
    loadJsonLngById(mercNames, d2r::storage, "data:data/local/lng/strings/mercenaries.json");
    sgamedata.strings.erase("dummy");
    sgamedata.strings.erase("Dummy");
    sgamedata.strings.erase("unused");

    D2TXT levelTxt, objTxt, monTxt, shrineTxt, superuTxt;
    D2TXT itemTxt[3];
    loadTxt(levelTxt, d2r::storage, "data:data/global/excel/levels.txt");
    loadTxt(objTxt, d2r::storage, "data:data/global/excel/objects.txt");
    loadTxt(monTxt, d2r::storage, "data:data/global/excel/monstats.txt");
    loadTxt(shrineTxt, d2r::storage, "data:data/global/excel/shrines.txt");
    loadTxt(superuTxt, d2r::storage, "data:data/global/excel/superuniques.txt");
    loadTxt(itemTxt[0], d2r::storage, "data:data/global/excel/weapons.txt");
    loadTxt(itemTxt[1], d2r::storage, "data:data/global/excel/armor.txt");
    loadTxt(itemTxt[2], d2r::storage, "data:data/global/excel/misc.txt");

    std::map<std::wstring, int> levelIdByName;

    for (const auto *s: {"strCreateGameNormalText", "strCreateGameNightmareText", "strCreateGameHellText"}) {
        std::string key = s;
        moveStrings(sgamedata.strings, names, key);
    }
    auto idx0 = levelTxt.colIndexByName("Id");
    auto idx1 = levelTxt.colIndexByName("LevelName");
    auto rows = levelTxt.rows();
    for (size_t i = 0; i < rows; ++i) {
        auto id = levelTxt.value(i, idx0).second;
        const auto &key = levelTxt.value(i, idx1).first;
        const auto *arr = moveStrings(sgamedata.strings, names, key);
        if (arr) {
            levelIdByName[(*arr)[0]] = id;
            if (id >= sgamedata.levels.size()) { sgamedata.levels.resize(id + 1); }
            sgamedata.levels[id] = {key, arr};
        }
    }

    idx0 = objTxt.colIndexByName("*ID");
    idx1 = objTxt.colIndexByName("Name");
    auto idx2 = objTxt.colIndexByName("OperateFn");
    auto idx3 = objTxt.colIndexByName("SizeX");
    auto idx4 = objTxt.colIndexByName("SizeY");
    auto idx5 = objTxt.colIndexByName("IsDoor");
    rows = objTxt.rows();
    for (size_t i = 0; i < rows; ++i) {
        auto id = objTxt.value(i, idx0).second;
        if (objTxt.value(i, idx5).second) {
            sgamedata.objects[0][id] = {TypeDoor, "", nullptr,
                                        float(std::max(1, objTxt.value(i, idx3).second)),
                                        float(std::max(1, objTxt.value(i, idx4).second))};
            continue;
        }
        auto op = objTxt.value(i, idx2).second;
        auto kite = parseData.usefulObjectOp.find(op);
        EObjType tp;
        std::string key;
        if (kite == parseData.usefulObjectOp.end()) {
            auto ite = parseData.usefulObjects.find(id);
            if (ite == parseData.usefulObjects.end()) { continue; }
            key = ite->second.second;
            tp = objTypeFromString(ite->second.first);
        } else {
            key = kite->second.second;
            tp = objTypeFromString(kite->second.first);
        }
        if (key.empty()) { key = objTxt.value(i, idx1).first; }
        const auto *arr = moveStrings(sgamedata.strings, names, key);
        if (arr) {
            auto minValue = tp == TypeWayPoint ? 3 : 2;
            auto w = float(std::max(minValue, objTxt.value(i, idx3).second));
            auto h = float(std::max(minValue, objTxt.value(i, idx4).second));
            sgamedata.objects[0][id] = {tp, key, arr, w, h};
        }
    }

    idx0 = monTxt.colIndexByName("*hcIdx");
    idx1 = monTxt.colIndexByName("NameStr");
    idx2 = monTxt.colIndexByName("npc");
    idx3 = monTxt.colIndexByName("Align");
    idx4 = monTxt.colIndexByName("inTown");
    idx5 = monTxt.colIndexByName("enabled");
    rows = monTxt.rows();
    for (size_t i = 0; i < rows; ++i) {
        auto id = monTxt.value(i, idx0).second;
        auto ite = parseData.usefulNpcs.find(id);
        const Data::LngString *arr = nullptr;
        if (ite != parseData.usefulNpcs.end()) {
            std::string key = ite->second.second;
            if (key.empty()) { key = monTxt.value(i, idx1).first; }
            arr = moveStrings(sgamedata.strings, names, key);
            if (arr) {
                sgamedata.objects[1][id] = {objTypeFromString(ite->second.first), key, arr, 2.f, 2.f};
            }
        }
        if (monTxt.value(i, idx5).second) {
            auto key = monTxt.value(i, idx1).first;
            if (!arr) { arr = moveStrings(sgamedata.strings, names, key); }
            uint8_t monType = monTxt.value(i, idx2).second ? 1 :
                              monTxt.value(i, idx3).second ? 2 :
                              monTxt.value(i, idx4).second ? 1 : 0;
            if (id >= sgamedata.monsters.size()) { sgamedata.monsters.resize(id + 1); }
            sgamedata.monsters[id] = {arr ? key : "", monType, arr};
        }
    }

    idx0 = shrineTxt.colIndexByName("Code");
    idx1 = shrineTxt.colIndexByName("StringName");
    rows = shrineTxt.rows();
    for (size_t i = 0; i < rows; ++i) {
        auto id = shrineTxt.value(i, idx0).second;
        auto key = shrineTxt.value(i, idx1).first;
        const auto *arr = moveStrings(sgamedata.strings, names, key);
        if (arr) {
            if (id >= sgamedata.shrines.size()) { sgamedata.shrines.resize(id + 1); }
            sgamedata.shrines[id] = {key, arr};
        }
    }

    idx0 = superuTxt.colIndexByName("hcIdx");
    idx1 = superuTxt.colIndexByName("Name");
    rows = superuTxt.rows();
    for (size_t i = 0; i < rows; ++i) {
        auto id = superuTxt.value(i, idx0).second;
        auto key = superuTxt.value(i, idx1).first;
        const auto *arr = moveStrings(sgamedata.strings, names, key);
        if (arr) {
            if (id >= sgamedata.superUniques.size()) { sgamedata.superUniques.resize(id + 1); }
            sgamedata.superUniques[id] = {key, arr};
        }
    }

    uint32_t itemIndex = 0;
    for (const auto &item : itemTxt) {
        idx0 = item.colIndexByName("name");
        idx1 = item.colIndexByName("code");
        idx2 = item.colIndexByName("namestr");
        rows = item.rows();
        for (size_t i = 0; i < rows; ++i) {
            const auto &name = item.value(i, idx0).first;
            if (name == "Expansion") { continue; }
            const auto &code = item.value(i, idx1).first;
            const auto &key = item.value(i, idx2).first;
            const auto *arr = moveStrings(sgamedata.strings, names, key);
            if (arr) {
                if (itemIndex >= sgamedata.items.size()) { sgamedata.items.resize(itemIndex + 1); }
                sgamedata.items[itemIndex] = {key, arr};
                sgamedata.itemIdByCode[key] = itemIndex;
            }
            itemIndex++;
        }
    }

    for (auto &p: mercNames) {
        auto ite = mercNames.find(p.first);
        if (ite == mercNames.end()) {
            continue;
        }
        if (p.first >= sgamedata.mercNames.size()) { sgamedata.mercNames.resize(p.first + 1); }
        sgamedata.mercNames[p.first] = std::move(ite->second);
    }
    mercNames.clear();

    for (auto &p: parseData.guides) {
        int id;
        if (p.first[0] >= '0' && p.first[0] <= '9') {
            id = strtol(p.first.c_str(), nullptr, 0);
        } else {
            auto ite = levelIdByName.find(util::utf8toucs4(p.first));
            if (ite == levelIdByName.end()) {
                continue;
            }
            id = ite->second;
        }
        for (auto &s: p.second) {
            if (s.empty()) { continue; }
            if (s[0] == '-') {
                sgamedata.guides[id].insert(0x20000 | strtoul(s.data() + 1, nullptr, 0));
            } else if (s[0] == '+') {
                sgamedata.guides[id].insert(0x10000 | strtoul(s.data() + 1, nullptr, 0));
            } else if (s[0] >= '0' && s[0] <= '9'){
                sgamedata.guides[id].insert(strtoul(s.data() + 1, nullptr, 0));
            }
            auto ite2 = levelIdByName.find(util::utf8toucs4(s));
            if (ite2 == levelIdByName.end()) {
                continue;
            }
            sgamedata.guides[id].insert(ite2->second);
        }
    }
}

void loadData() {
    loadD2RData();
    int section = -1;
    ini_parse("D2RMH_item.ini", [](void *user, const char *section,
                                   const char *name, const char *value) -> int {
        auto *isec = (int *)user;
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
        default:break;
        }
        return 1;
    }, &section);
}

uint16_t filterItem(const d2r::UnitAny *unit, const d2r::ItemData *item, uint32_t sockets) {
    if (sockets > 6) { return 0; }
    auto id = unit->txtFileNo;
    if (id >= 1000) { return 0; }
    auto quality = item->quality - 1;
    if (quality >= 8) { return 0; }
    auto ethereal = (item->itemFlags & 0x00400000) ? 1 : 0;
    return itemFilters[id][quality][ethereal][sockets];
}

}

/*
 * Copyright (c) 2021 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#include "processdata.h"

#include "d2rdefs.h"
#include "cfg.h"

#include "util/ntprocess.h"
#include "data/gamedata.h"

#include <windows.h>

namespace d2r {

extern wchar_t enchantStrings[256][4];
extern wchar_t auraStrings[187][4];
extern wchar_t immunityStrings[6][4];
extern uint8_t statsMapping[size_t(StatId::TotalCount)];

enum {
    InventoryPanelOffset = 0x01,
    CharacterPanelOffset = 0x02,
    SkillFloatSelOffset = 0x03,
    SkillTreePanelOffset = 0x04,
    ChatMenuOffset = 0x08,
    SystemMenuOffset = 0x09,
    InGameMapOffset = 0x0A,
    QuestPanelOffset = 0x0E,
    WaypointPanelOffset = 0x13,
    PartyPanelOffset = 0x15,
    MercenaryOffset = 0x1E,
};

#define READ(a, v) util::readMemory64(handle, (a), sizeof(v), &(v))
#define READN(a, v, n) util::readMemory64(handle, (a), (n), (v))

ProcessData::ProcessData() :
    mapObjects(1024) {
    mapMonsters.reserve(1024);
    mapItems.reserve(1024);
}

ProcessData::~ProcessData() {
    if (handle) {
        UnhookWinEvent(HWINEVENTHOOK(hook));
        hook = nullptr;
        CloseHandle(handle);
    }
}

void ProcessData::resetData() {
    if (handle) {
        UnhookWinEvent(HWINEVENTHOOK(hook));
        hook = nullptr;
        CloseHandle(handle);
        handle = nullptr;
    }

    hwnd = nullptr;

    baseAddr = 0;
    baseSize = 0;

    hashTableBaseAddr = 0;
    uiBaseAddr = 0;
    isExpansionAddr = 0;
    rosterDataAddr = 0;
    gameInfoAddr = 0;

    mapEnabled = 0;
    panelEnabled = 0;
    focusedPlayer = 0;
    currPlayer = nullptr;

    gameName.clear();
    gamePass.clear();
    region.clear();
    season.clear();

    realTombLevelId = 0;
    superUniqueTombLevelId = 0;

    mapPlayers.clear();
    mapMonsters.clear();
    mapObjects.clear();
    mapItems.clear();
    knownItems.clear();
}

void ProcessData::updateData() {
    uint8_t lastDifficulty;
    uint32_t lastSeed, lastAct, lastLevelId;
    if (currPlayer) {
        lastDifficulty = currPlayer->difficulty;
        lastSeed = currPlayer->seed;
        lastAct = currPlayer->act;
        lastLevelId = currPlayer->levelId;
    } else {
        lastDifficulty = uint8_t(-1);
        lastSeed = 0;
        lastAct = uint32_t(-1);
        lastLevelId = uint32_t(-1);
    }

    mapPlayers.clear();
    if (cfg->showMonsters) {
        mapMonsters.clear();
    }
    if (cfg->showItems) {
        mapItems.clear();
    }

    /* Address from MapAssist: read obfuscated map seed */
    uint64_t mapSeedPtr;
    if (!READ(mapSeedAddr, mapSeedPtr)) {
        return;
    }
    uint32_t seedCheck;
    if (!READ(mapSeedPtr + 0x110, seedCheck)) {
        return;
    }
    if (!READ(mapSeedPtr + (seedCheck ? 0x840 : 0x10C0), currSeed)) {
        return;
    }

    readRosters();
    readUnitHashTable(hashTableBaseAddr, [this, lastDifficulty, lastSeed, lastAct, lastLevelId](const UnitAny &unit) {
        if (!unit.unitId || !unit.actPtr || !unit.inventoryPtr) { return; }
        uint64_t token;
        if (unit.unitId != focusedPlayer) {
            /* --START-- remove this if using readRoomUnits() */
            /* mode is 17 when this is a corpse */
            if (unit.mode != 17) { return; }
            DrlgAct act;
            if (!READ(unit.actPtr, act)) { return; }
            auto &player = mapPlayers[unit.unitId];
            player.skillPtr = unit.skillPtr;
            player.name[0] = 0;
            READ(unit.unionPtr, player.name);
            player.levelChanged = false;
            player.act = act.actId;
            player.seed = currSeed;
            READ(act.miscPtr + 0x830, player.difficulty);
            player.stats.fill(0);
            readPlayerStats(unit, [&player](uint16_t statId, int32_t value) {
                if (statId > 15) {
                    return;
                }
                player.stats[statId] = value;
            });
            DynamicPath path;
            if (!READ(unit.pathPtr, path)) { return; }
            player.posX = path.posX;
            player.posY = path.posY;
            DrlgRoom1 room1;
            if (!READ(path.room1Ptr, room1)) { return; }
            DrlgRoom2 room2;
            if (!READ(room1.room2Ptr, room2)) { return; }
            if (!READ(room2.levelPtr + 0x1F8, player.levelId)) { return; }
            /* --END-- remove this if using readRoomUnits() */
            return;
        }
        DrlgAct act;
        auto &player = mapPlayers[unit.unitId];
        player.skillPtr = unit.skillPtr;
        player.name[0] = 0;
        READ(unit.unionPtr, player.name);
        player.stats.fill(0);
        readPlayerStats(unit, [&player](uint16_t statId, int32_t value) {
            if (statId > 15) {
                return;
            }
            player.stats[statId] = value;
        });
        if (!READ(unit.actPtr, act)) { return; }
        READ(act.miscPtr + 0x830, player.difficulty);
        player.levelChanged = false;
        player.seed = currSeed;
        if (lastDifficulty != player.difficulty || lastSeed != currSeed) {
            readGameInfo();
            player.levelChanged = true;
            knownItems.clear();
        }
        player.act = act.actId;
        if (lastAct != act.actId) {
            player.levelChanged = true;
        }
        if (player.levelChanged) {
            /* get real TalTomb level id */
            READ(act.miscPtr + 0x120, realTombLevelId);
            READ(act.miscPtr + 0x874, superUniqueTombLevelId);
        }
        DynamicPath path;
        if (!READ(unit.pathPtr, path)) { return; }
        player.posX = path.posX;
        player.posY = path.posY;
        DrlgRoom1 room1;
        if (!READ(path.room1Ptr, room1)) { return; }
        DrlgRoom2 room2;
        if (!READ(room1.room2Ptr, room2)) { return; }
        uint32_t levelId;
        if (!READ(room2.levelPtr + 0x1F8, levelId)) { return; }

        player.levelId = levelId;
        if (levelId != lastLevelId) {
            player.levelChanged = true;
        }
        if (player.levelChanged) {
            if (cfg->showObjects) {
                mapObjects.clear();
            }
        }
/*  this is another way of reading all units from memory, we may need this if we need to see units on neighbour levels
        std::unordered_set<uint64_t> rset(256);
        rset.insert(path.room1Ptr);
        readRoomUnits(room1, rset);
*/
    });
    if (!focusedPlayer) {
        return;
    }
    uint8_t mem[0x28];
    READ(uiBaseAddr, mem);
    mapEnabled = mem[InGameMapOffset];
    uint32_t enableBits = 0;
    if (mem[InventoryPanelOffset]) { enableBits |= 0x01; }
    if (mem[CharacterPanelOffset]) { enableBits |= 0x02; }
    if (mem[SkillTreePanelOffset]) { enableBits |= 0x04; }
    if (mem[SystemMenuOffset]) { enableBits |= 0x08; }
    if (mem[QuestPanelOffset]) { enableBits |= 0x10; }
    if (mem[PartyPanelOffset]) { enableBits |= 0x20; }
    if (mem[MercenaryOffset]) { enableBits |= 0x40; }
    if (mem[WaypointPanelOffset]) { enableBits |= 0x80; }
    if (mem[SkillFloatSelOffset]) { enableBits |= 0x100; }
    panelEnabled = enableBits;

    if (cfg->showMonsters) {
        readUnitHashTable(hashTableBaseAddr + 8 * 0x80, [this](const auto &unit) {
            readUnit(unit);
        });
    }
    if (cfg->showObjects) {
        readUnitHashTable(hashTableBaseAddr + 8 * 0x100, [this](const auto &unit) {
            readUnit(unit);
        });
    }
    if (cfg->showItems) {
        readUnitHashTable(hashTableBaseAddr + 8 * 0x200, [this](const auto &unit) {
            readUnit(unit);
        });
    }
}

inline bool matchMem(size_t sz, const uint8_t *mem, const uint8_t *search, const uint8_t *mask) {
    for (size_t i = 0; i < sz; ++i) {
        uint8_t m = mask[i];
        if ((mem[i] & m) != (search[i] & m)) { return false; }
    }
    return true;
}

inline size_t searchMem(const uint8_t *mem, size_t sz, const uint8_t *search, const uint8_t *mask, size_t searchSz) {
    if (sz < searchSz) { return size_t(-1); }
    size_t e = sz - searchSz + 1;
    for (size_t i = 0; i < e; ++i) {
        if (matchMem(searchSz, mem + i, search, mask)) {
            return i;
        }
    }
    return size_t(-1);
}

void ProcessData::updateOffset() {
    auto *mem = new(std::nothrow) uint8_t[size_t(baseSize)];
    if (mem && READN(baseAddr, mem, uint32_t(baseSize))) {
        const uint8_t search0[] = {0x48, 0x03, 0xC7, 0x49, 0x8B, 0x8C, 0xC6 };
        const uint8_t mask0[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
        auto off = searchMem(mem, size_t(baseSize), search0, mask0, sizeof(search0));
        if (off != size_t(-1)) {
            int32_t rel;
            if (READ(baseAddr + off + 7, rel)) {
                hashTableBaseAddr = baseAddr + rel;
            }
        }

        const uint8_t search1[] = {0x45, 0x8B, 0xD7, 0x4C, 0x8D, 0x05, 0, 0, 0, 0};
        const uint8_t mask1[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0, 0, 0, 0};
        off = searchMem(mem, size_t(baseSize), search1, mask1, sizeof(search1));
        if (off != size_t(-1)) {
            int32_t rel;
            if (READ(baseAddr + off + 6, rel)) {
                uiBaseAddr = baseAddr + off + 10 + rel;
            }
        }

        const uint8_t search2[] = {0x48, 0x8B, 0x05, 0, 0, 0, 0, 0x48, 0x8B, 0xD9, 0xF3, 0x0F, 0x10, 0x50};
        const uint8_t mask2[] = {0xFF, 0xFF, 0xFF, 0, 0, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
        off = searchMem(mem, size_t(baseSize), search2, mask2, sizeof(search2));
        if (off != size_t(-1)) {
            int32_t rel;
            if (READ(baseAddr + off + 3, rel)) {
                isExpansionAddr = baseAddr + off + 7 + rel;
            }
        }

        const uint8_t search3[] = {0x02, 0x45, 0x33, 0xD2, 0x4D, 0x8B};
        const uint8_t mask3[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
        off = searchMem(mem, size_t(baseSize), search3, mask3, sizeof(search3));
        if (off != size_t(-1)) {
            int32_t rel;
            if (READ(baseAddr + off - 3, rel)) {
                rosterDataAddr = baseAddr + off + 1 + rel;
            }
        }

        const uint8_t search4[] = {0x44, 0x88, 0x25, 0, 0, 0, 0, 0x66, 0x44, 0x89, 0x25};
        const uint8_t mask4[] = {0xFF, 0xFF, 0xFF, 0, 0, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF};
        off = searchMem(mem, size_t(baseSize), search4, mask4, sizeof(search4));
        if (off != size_t(-1)) {
            int32_t rel;
            if (READ(baseAddr + off + 3, rel)) {
                gameInfoAddr = baseAddr + off - 0x121 + rel;
            }
        }

        const uint8_t search5[] = {0x41, 0x8B, 0xF9, 0x48, 0x8D, 0x0D, 0, 0, 0, 0};
        const uint8_t mask5[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0, 0, 0, 0};
        off = searchMem(mem, size_t(baseSize), search5, mask5, sizeof(search5));
        if (off != size_t(-1)) {
            int32_t rel;
            if (READ(baseAddr + off + 6, rel)) {
                mapSeedAddr = baseAddr + off + 0xEA + rel;
            }
        }
    }
    delete[] mem;
}

Skill *ProcessData::getSkill(uint16_t id) {
    if (!currPlayer) { return nullptr; }
    SkillInfo si;
    if (READ(currPlayer->skillPtr, si)) {
        static Skill sk;
        uint64_t ptr = si.firstSkillPtr;
        while (ptr && READ(ptr, sk)) {
            uint16_t skillId;
            READ(sk.skillTxtPtr, skillId);
            if (skillId == id) { return &sk; }
            ptr = sk.nextSkillPtr;
        }
    }
    return nullptr;
}

void ProcessData::readRosters() {
    uint64_t addr;
    READ(rosterDataAddr, addr);
    bool first = true;
    while (addr) {
        RosterUnit mem;
        if (!READ(addr, mem)) { break; }
        auto &p = mapPlayers[mem.unitId];
        p.classId = mem.classId;
        p.level = mem.level;
        p.party = mem.partyId;
        memcpy(p.name, mem.name, 16);
        if (first) {
            focusedPlayer = mem.unitId;
            currPlayer = &p;
            first = false;
        } else {
            p.posX = mem.posX;
            p.posY = mem.posY;
            p.act = mem.actId;
        }
        addr = mem.nextPtr;
    }
}

void ProcessData::readUnitHashTable(uint64_t addr, const std::function<void(const UnitAny &)> &callback) {
    uint64_t addrList[0x80];
    if (!READ(addr, addrList)) { return; }
    for (auto paddr: addrList) {
        while (paddr) {
            UnitAny unit;
            if (READ(paddr, unit)) {
                callback(unit);
            }
            paddr = unit.nextPtr;
        }
    }
}

void ProcessData::readStatList(uint64_t addr, uint32_t unitId, const std::function<void(const StatList &)> &callback) {
    StatList stats;
    if (!READ(addr, stats)) { return; }
    do {
        /* check if this is owner stat or aura */
        if (!unitId || stats.ownerId == unitId) {
            callback(stats);
        }
        if (!(stats.flag & 0x80000000u)) { break; }
        if (!stats.nextListEx || !READ(stats.nextListEx, stats)) { break; }
    } while (true);
}

void ProcessData::readPlayerStats(const UnitAny &unit, const std::function<void(uint16_t, int32_t)> &callback) {
    readStatList(unit.statListPtr, 0, [this, &callback](const StatList &stats) {
        if (!(stats.flag & 0x80000000u)) { return; }
        static StatEx statEx[256];
        auto cnt = std::min(255u, uint32_t(stats.fullStat.statCount));
        if (!READN(stats.fullStat.statPtr, statEx, sizeof(StatEx) * cnt)) { return; }
        StatEx *st = statEx;
        st[cnt].statId = 0xFFFF;
        uint16_t statId;
        for (; (statId = st->statId) != 0xFFFF; ++st) {
            if (statId >= 16) { break; }
            callback(statId, statId >= 6 && statId <= 11 ? (st->value >> 8) : st->value);
        }
    });
}

void ProcessData::readUnit(const UnitAny &unit) {
    switch (unit.unitType) {
    case 0:readUnitPlayer(unit);
        break;
    case 1:if (cfg->showMonsters) { readUnitMonster(unit); }
        break;
    case 2:if (cfg->showObjects) { readUnitObject(unit); }
        break;
    case 4:if (cfg->showItems) { readUnitItem(unit); }
        break;
    }
}

void ProcessData::readUnitPlayer(const UnitAny &unit) {
    if (unit.unitId == focusedPlayer) { return; }
    DrlgAct act;
    if (!READ(unit.actPtr, act)) { return; }
    auto &player = mapPlayers[unit.unitId];
    player.name[0] = 0;
    READ(unit.unionPtr, player.name);
    player.levelChanged = false;
    player.act = act.actId;
    player.seed = currSeed;
    READ(act.miscPtr + 0x830, player.difficulty);
    player.stats.fill(0);
    readPlayerStats(unit, [&player](uint16_t statId, int32_t value) {
        if (statId > 15) {
            return;
        }
        player.stats[statId] = value;
    });
    DynamicPath path;
    if (!READ(unit.pathPtr, path)) { return; }
    player.posX = path.posX;
    player.posY = path.posY;
    DrlgRoom1 room1;
    if (!READ(path.room1Ptr, room1)) { return; }
    DrlgRoom2 room2;
    if (!READ(room1.room2Ptr, room2)) { return; }
    if (!READ(room2.levelPtr + 0x1F8, player.levelId)) { return; }
}

void ProcessData::readUnitMonster(const UnitAny &unit) {
    if (unit.mode == 0 || unit.mode == 12 || unit.txtFileNo >= data::gamedata->monsters.size()) { return; }
    MonsterData monData;
    if (!READ(unit.unionPtr, monData)) { return; }
    auto isUnique = (monData.flag & 0x0E) != 0;
    auto &txtData = data::gamedata->monsters[unit.txtFileNo];
    auto isNpc = std::get<1>(txtData);
    auto sm = cfg->showMonsters;
    if (!isNpc && !(sm == 2 || (isUnique && sm == 1))) { return; }
    DynamicPath path;
    if (!READ(unit.pathPtr, path)) { return; }
    auto &mon = mapMonsters.emplace_back();
    mon.x = path.posX;
    mon.y = path.posY;
    mon.isNpc = isNpc;
    mon.isUnique = isUnique;
    mon.flag = monData.flag;
    if (isNpc) {
        if (cfg->showNpcNames) {
            if (monData.mercNameId == uint16_t(-1) || monData.mercNameId >= data::gamedata->mercNames.size()) {
                /* show only npcs' name, hide summons' name */
                if (isNpc == 1) { mon.name = std::get<2>(txtData); }
            } else {
                mon.name = &data::gamedata->mercNames[monData.mercNameId];
            }
        }
        return;
    } else if (auto sn = cfg->showMonsterNames; (sn == 2 || (isUnique && sn == 1))) {
        /* Super unique */
        if ((monData.flag & 2) && monData.uniqueNo < data::gamedata->superUniques.size()) {
            mon.name = data::gamedata->superUniques[monData.uniqueNo].second;
        } else {
            mon.name = std::get<2>(txtData);
        }
    }
    int off = 0;
    bool hasAura = false;
    if (auto sme = cfg->showMonsterEnchants; sme == 2 || (isUnique && sme == 1)) {
        uint8_t id;
        for (int n = 0; n < 9 && (id = monData.enchants[n]) != 0; ++n) {
            if (id == 30) {
                hasAura = true;
                continue;
            }
            const auto *str = enchantStrings[id];
            while (*str) {
                mon.enchants[off++] = *str++;
            }
        }
    }
    auto smi = cfg->showMonsterImmunities;
    bool showMI = smi == 2 || (isUnique && smi == 1);
    if (!showMI && !hasAura) {
        mon.enchants[off] = 0;
        return;
    }
    readStatList(unit.statListPtr, unit.unitId, [this, &off, &mon, hasAura, showMI](const StatList &stats) {
        if (stats.stateNo) {
            if (!hasAura) { return; }
            const wchar_t *str = auraStrings[stats.stateNo];
            while (*str) {
                mon.enchants[off++] = *str++;
            }
            return;
        }
        if (!showMI) { return; }
        static StatEx statEx[64];
        auto cnt = std::min(64u, uint32_t(stats.baseStat.statCount));
        if (!READN(stats.baseStat.statPtr, statEx, sizeof(StatEx) * cnt)) { return; }
        StatEx *st = statEx;
        for (; cnt; --cnt, ++st) {
            auto statId = st->statId;
            if (statId >= uint16_t(StatId::TotalCount)) { continue; }
            auto mapping = statsMapping[statId];
            if (!mapping || st->value < 100) { continue; }
            const wchar_t *str = immunityStrings[mapping];
            while (*str) {
                mon.enchants[off++] = *str++;
            }
        }
    });
    mon.enchants[off] = 0;
}

void ProcessData::readUnitObject(const UnitAny &unit) {
    if (/* Portals */ unit.txtFileNo != 59 && unit.txtFileNo != 60 && /* destroyed/opened */ unit.mode == 2) {
        StaticPath path;
        if (READ(unit.pathPtr, path)) {
            mapObjects.erase(path.posX | (uint32_t(path.posY) << 16));
        }
        return;
    }
    auto ite = data::gamedata->objects[0].find(unit.txtFileNo);
    if (ite == data::gamedata->objects[0].end()) { return; }
    auto type = std::get<0>(ite->second);
    switch (type) {
    case data::TypePortal:
    case data::TypeWell:
    case data::TypeShrine: {
        uint8_t flag;
        READ(unit.unionPtr + 8, flag);
        StaticPath path;
        if (!READ(unit.pathPtr, path)) { break; }
        auto &obj = mapObjects[path.posX | (uint32_t(path.posY) << 16)];
        if (obj.x) { break; }
        obj.type = std::get<0>(ite->second);
        obj.name = type == data::TypeShrine && flag < data::gamedata->shrines.size() ?
                   data::gamedata->shrines[flag].second : std::get<2>(ite->second);
        obj.x = path.posX;
        obj.y = path.posY;
        obj.flag = flag;
        obj.w = std::get<3>(ite->second) * .5f;
        obj.h = std::get<4>(ite->second) * .5f;
        break;
    }
    default:break;
    }
}

void ProcessData::readUnitItem(const UnitAny &unit) {
    ItemData item;
    if (!unit.actPtr /* ground items has pAct set */
        || !READ(unit.unionPtr, item)
        || item.ownerInvPtr || item.location != 0
        || (item.itemFlags & 0x01) /* InStore */) { return; }
    /* the item is on the ground */
    uint32_t sockets = 0;
    if (item.itemFlags & 0x800u) {
        readStatList(unit.statListPtr, unit.unitId, [this, &sockets](const StatList &stats) {
            if (stats.stateNo) { return; }
            static StatEx statEx[64];
            auto cnt = std::min(64u, uint32_t(stats.baseStat.statCount));
            if (!READN(stats.baseStat.statPtr, statEx, sizeof(StatEx) * cnt)) { return; }
            StatEx *st = statEx;
            for (; cnt; --cnt, ++st) {
                if (st->statId != uint16_t(StatId::ItemNumsockets)) { continue; }
                sockets = st->value;
                break;
            }
        });
    }
    auto n = data::filterItem(&unit, &item, sockets);
    if (!n) { return; }
    StaticPath path;
    if (!READ(unit.pathPtr, path)) { return; }
    auto &mitem = mapItems.emplace_back();
    mitem.x = path.posX;
    mitem.y = path.posY;
    mitem.name = unit.txtFileNo < data::gamedata->items.size() ? data::gamedata->items[unit.txtFileNo].second : nullptr;
    mitem.flag = n & 0x0F;
    auto color = (n >> 4) & 0x0F;
    if (color == 0) {
        if (auto quality = uint32_t(item.quality - 1); quality < 8) {
            const uint8_t qualityColors[8] = {15, 15, 5, 3, 2, 9, 4, 8};
            color = qualityColors[quality];
        }
    }
    mitem.color = color;
    auto snd = size_t(n >> 8);
    if (snd > 0 && knownItems.find(unit.unitId) == knownItems.end()) {
        knownItems.insert(unit.unitId);
        if (snd < cfg->sounds.size() && !cfg->sounds[snd].first.empty()) {
            if (cfg->sounds[snd].second) {
                PlaySoundW(cfg->sounds[snd].first.c_str(), nullptr, SND_ALIAS | SND_ASYNC | SND_NODEFAULT);
            } else {
                PlaySoundW(cfg->sounds[snd].first.c_str(), nullptr, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
            }
        }
    }
}

void ProcessData::readGameInfo() {
    GameInfo gameInfo;
    if (READ(gameInfoAddr, gameInfo)) {
        gameName.assign(gameInfo.gameNameBuffer, gameInfo.gameNameBuffer + gameInfo.gameName.size);
        gamePass.assign(gameInfo.gamePassBuffer, gameInfo.gamePassBuffer + gameInfo.gamePass.size);
        region.assign(gameInfo.regionBuffer, gameInfo.regionBuffer + gameInfo.region.size);
        season.assign(gameInfo.seasonBuffer, gameInfo.seasonBuffer + gameInfo.season.size);
    } else {
        gameName.clear();
        gamePass.clear();
        region.clear();
        season.clear();
    }
}

/*
void ProcessData::readRoomUnits(const DrlgRoom1 &room1, std::unordered_set<uint64_t> &roomList) {
    auto *currProcess = currProcess_;
    DrlgRoom2 room2;
    READ(room1.room2Ptr, room2);
    uint64_t roomsNear[64];
    uint64_t addr = room1.unitFirstAddr;
    while (addr) {
        UnitAny unit;
        if (!READ(addr, unit)) { break; }
        readUnit(unit);
        addr = unit.roomNextPtr;
    }
    auto count = std::min(64u, room1.roomsNear);
    if (READN(room1.roomsNearListPtr, roomsNear, count * sizeof(uint64_t))) {
        for (auto i = 0u; i < count; ++i) {
            auto addr = roomsNear[i];
            if (roomList.find(addr) != roomList.end()) { continue; }
            DrlgRoom1 room;
            if (!READ(addr, room)) { continue; }
            roomList.insert(addr);
            readRoomUnits(room, roomList);
        }
    }
}
*/

}

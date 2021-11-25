/*
 * Copyright (c) 2021 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#pragma once

#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>
#include <functional>
#include <cstdint>

struct UnitAny;
struct DrlgRoom1;
struct StatList;

class D2RProcess final {
public:
    struct MapPlayer {
        uint32_t act;
        uint32_t seed;
        uint32_t levelId;
        uint16_t posX, posY;
        char name[16];
        uint8_t difficulty;
        bool levelChanged;
    };
    struct MapMonster {
        uint32_t x, y;
        const std::array<std::wstring, 13> *name;
        wchar_t enchants[32];
        uint8_t flag;
        bool isNpc;
        bool isUnique;
    };
    struct MapObject {
        uint32_t x, y;
        const std::array<std::wstring, 13> *name;
        uint8_t type;
        uint8_t flag;
        float w, h;
    };
    struct MapItem {
        uint32_t x, y;
        const std::array<std::wstring, 13> *name;
        uint8_t flag;
        uint8_t color;
    };
public:
    explicit D2RProcess(uint32_t searchInterval = 5);
    ~D2RProcess();

    void updateData();
    void setWindowPosCallback(const std::function<void(int, int, int, int)> &cb);

    void *hwnd() { return hwnd_; }
    [[nodiscard]] inline bool available() const { return available_; }
    [[nodiscard]] inline bool mapEnabled() const { return mapEnabled_ != 0; }
    [[nodiscard]] inline uint8_t panelEnabled() const { return panelEnabled_; }
    [[nodiscard]] inline uint32_t realTombLevelId() const { return realTombLevelId_; }
    [[nodiscard]] inline uint32_t superUniqueTombLevelId() const { return superUniqueTombLevelId_; }

    [[nodiscard]] inline const MapPlayer *currPlayer() const { return currPlayer_; }
    [[nodiscard]] inline const std::unordered_map<uint32_t, MapPlayer> &players() const { return mapPlayers_; }
    [[nodiscard]] inline const std::vector<MapMonster> &monsters() const { return mapMonsters_; }
    [[nodiscard]] inline const std::unordered_map<uint32_t, MapObject> &objects() const { return mapObjects_; }
    [[nodiscard]] inline const std::vector<MapItem> &items() const { return mapItems_; }

    void reloadConfig() { loadFromCfg(); resetData(); }

private:
    void searchForProcess();
    void updateOffset();
    void resetData();
    void readUnitHashTable(uint64_t addr, const std::function<void(const UnitAny&)> &callback);
    void readStateList(uint64_t addr, uint32_t unitId, const std::function<void(const StatList&)> &callback);
    void readRoomUnits(const DrlgRoom1 &room1, std::unordered_set<uint64_t> &roomList);
    void readUnit(const UnitAny &unit);
    void readUnitPlayer(const UnitAny &unit);
    void readUnitMonster(const UnitAny &unit);
    void readUnitObject(const UnitAny &unit);
    void readUnitItem(const UnitAny &unit);
    void loadFromCfg();

private:
    void *handle_ = nullptr;
    void *hwnd_ = nullptr;
    void *hook_ = nullptr;
    bool available_ = false;
    uint64_t baseAddr_ = 0;
    uint64_t baseSize_ = 0;
    uint64_t hashTableBase_ = 0;
    uint64_t uiBase_ = 0;
    uint64_t isExpansionAddr_ = 0;

    uint32_t nextSearchTime_ = 0;
    uint32_t searchInterval_ = 0;

    uint8_t mapEnabled_ = 0;
    uint8_t panelEnabled_ = 0;

    uint32_t focusedPlayer_ = 0;

    uint32_t realTombLevelId_ = 0;
    uint32_t superUniqueTombLevelId_ = 0;

    const MapPlayer *currPlayer_ = nullptr;
    std::unordered_map<uint32_t, MapPlayer> mapPlayers_;
    std::vector<MapMonster> mapMonsters_;
    std::unordered_map<uint32_t, MapObject> mapObjects_;
    std::vector<MapItem> mapItems_;
    std::unordered_set<uint32_t> knownItems_;
};

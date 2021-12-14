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
#include <array>
#include <functional>
#include <chrono>
#include <cstdint>

struct UnitAny;
struct DrlgRoom1;
struct StatList;
struct Skill;

class D2RProcess final {
public:
    struct MapPlayer {
        uint32_t act;
        uint32_t seed;
        uint32_t levelId;
        int posX, posY;
        char name[16];
        uint32_t classId;
        uint16_t level;
        uint16_t party;
        uint8_t difficulty;
        bool levelChanged;
        std::array<int32_t, 16> stats;
        uint64_t skillPtr;
    };
    struct MapMonster {
        int x, y;
        const std::array<std::wstring, 13> *name;
        wchar_t enchants[32];
        uint8_t flag;
        bool isNpc;
        bool isUnique;
    };
    struct MapObject {
        int x, y;
        const std::array<std::wstring, 13> *name;
        uint8_t type;
        uint8_t flag;
        float w, h;
    };
    struct MapItem {
        int x, y;
        const std::array<std::wstring, 13> *name;
        uint8_t flag;
        uint8_t color;
    };
    struct ProcessData final {
        ProcessData();
        ~ProcessData();
        void resetData();

        void *handle = nullptr;
        void *hwnd = nullptr;
        void *hook = nullptr;

        uint64_t baseAddr = 0;
        uint64_t baseSize = 0;
        uint64_t hashTableBaseAddr = 0;
        uint64_t uiBaseAddr = 0;
        uint64_t isExpansionAddr = 0;
        uint64_t rosterDataAddr = 0;

        uint8_t mapEnabled = 0;
        uint32_t panelEnabled = 0;

        uint32_t focusedPlayer = 0;

        uint32_t realTombLevelId = 0;
        uint32_t superUniqueTombLevelId = 0;

        const MapPlayer *currPlayer = nullptr;
        std::unordered_map<uint32_t, MapPlayer> mapPlayers;
        std::vector<MapMonster> mapMonsters;
        std::unordered_map<uint32_t, MapObject> mapObjects;
        std::vector<MapItem> mapItems;
        std::unordered_set<uint32_t> knownItems;
    };
public:
    explicit D2RProcess(uint32_t searchInterval = 100);

    void killProcess();

    void updateData();
    void setWindowPosCallback(const std::function<void(int, int, int, int)> &cb);
    void setProcessCloseCallback(const std::function<void(void*)> &cb) {
        processCloseCallback_ = cb;
    }

    void *hwnd() { return currProcess_->hwnd; }
    [[nodiscard]] inline bool available() const { return currProcess_ != nullptr; }
    [[nodiscard]] inline bool mapEnabled() const { return currProcess_->mapEnabled != 0; }
    [[nodiscard]] inline uint32_t panelEnabled() const { return currProcess_->panelEnabled; }
    [[nodiscard]] inline uint32_t realTombLevelId() const { return currProcess_->realTombLevelId; }
    [[nodiscard]] inline uint32_t superUniqueTombLevelId() const { return currProcess_->superUniqueTombLevelId; }

    [[nodiscard]] inline const MapPlayer *currPlayer() const { return currProcess_->currPlayer; }
    [[nodiscard]] inline const std::unordered_map<uint32_t, MapPlayer> &players() const { return currProcess_->mapPlayers; }
    [[nodiscard]] inline const std::vector<MapMonster> &monsters() const { return currProcess_->mapMonsters; }
    [[nodiscard]] inline const std::unordered_map<uint32_t, MapObject> &objects() const { return currProcess_->mapObjects; }
    [[nodiscard]] inline const std::vector<MapItem> &items() const { return currProcess_->mapItems; }

    void reloadConfig() { loadFromCfg(); currProcess_ = nullptr; }

    /* functions for plugins */
    Skill *getSkill(uint16_t id);

private:
    void searchForProcess(void *hwnd);
    void updateOffset();
    void readUnitHashTable(uint64_t addr, const std::function<void(const UnitAny&)> &callback);
    void readStatList(uint64_t addr, uint32_t unitId, const std::function<void(const StatList&)> &callback);
    void readPlayerStats(const UnitAny &unit, const std::function<void(uint16_t, int32_t)> &callback);
    void readRoomUnits(const DrlgRoom1 &room1, std::unordered_set<uint64_t> &roomList);
    void readUnit(const UnitAny &unit);
    void readUnitPlayer(const UnitAny &unit);
    void readUnitMonster(const UnitAny &unit);
    void readUnitObject(const UnitAny &unit);
    void readUnitItem(const UnitAny &unit);
    void readRosters();
    void loadFromCfg();

private:
    std::chrono::steady_clock::time_point nextSearchTime_;
    std::chrono::steady_clock::duration searchInterval_;

    std::unordered_map<void*, ProcessData> processes_;
    ProcessData *currProcess_ = nullptr;

    std::function<void(void*)> processCloseCallback_;
};

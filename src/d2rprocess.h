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
#include <chrono>
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
    struct ProcessData final {
        ProcessData();
        ~ProcessData();
        void resetData();

        void *handle_ = nullptr;
        void *hwnd_ = nullptr;
        void *hook_ = nullptr;

        uint64_t baseAddr_ = 0;
        uint64_t baseSize_ = 0;
        uint64_t hashTableBase_ = 0;
        uint64_t uiBase_ = 0;
        uint64_t isExpansionAddr_ = 0;

        uint8_t mapEnabled_ = 0;
        uint32_t panelEnabled_ = 0;

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
public:
    explicit D2RProcess(uint32_t searchInterval = 100);

    void updateData();
    void setWindowPosCallback(const std::function<void(int, int, int, int)> &cb);
    void setProcessCloseCallback(const std::function<void(void*)> &cb) {
        processCloseCallback_ = cb;
    }

    void *hwnd() { return currProcess_->hwnd_; }
    [[nodiscard]] inline bool available() const { return currProcess_ != nullptr; }
    [[nodiscard]] inline bool mapEnabled() const { return currProcess_->mapEnabled_ != 0; }
    [[nodiscard]] inline uint32_t panelEnabled() const { return currProcess_->panelEnabled_; }
    [[nodiscard]] inline uint32_t realTombLevelId() const { return currProcess_->realTombLevelId_; }
    [[nodiscard]] inline uint32_t superUniqueTombLevelId() const { return currProcess_->superUniqueTombLevelId_; }

    [[nodiscard]] inline const MapPlayer *currPlayer() const { return currProcess_->currPlayer_; }
    [[nodiscard]] inline const std::unordered_map<uint32_t, MapPlayer> &players() const { return currProcess_->mapPlayers_; }
    [[nodiscard]] inline const std::vector<MapMonster> &monsters() const { return currProcess_->mapMonsters_; }
    [[nodiscard]] inline const std::unordered_map<uint32_t, MapObject> &objects() const { return currProcess_->mapObjects_; }
    [[nodiscard]] inline const std::vector<MapItem> &items() const { return currProcess_->mapItems_; }

    void reloadConfig() { loadFromCfg(); currProcess_ = nullptr; }

private:
    void searchForProcess(void *hwnd);
    void updateOffset();
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
    std::chrono::steady_clock::time_point nextSearchTime_;
    std::chrono::steady_clock::duration searchInterval_;

    std::unordered_map<void*, ProcessData> processes_;
    ProcessData *currProcess_ = nullptr;

    std::function<void(void*)> processCloseCallback_;
};

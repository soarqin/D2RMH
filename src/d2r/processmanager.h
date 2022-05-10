/*
 * Copyright (c) 2021 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#pragma once

#include "processdata.h"

#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>
#include <array>
#include <functional>
#include <chrono>
#include <cstdint>

namespace d2r {

struct UnitAny;
struct DrlgRoom1;
struct StatList;
struct Skill;

class ProcessManager final {
public:
    explicit ProcessManager(uint32_t searchInterval = 100);

    void killProcess();

    void updateData();
    void setWindowPosCallback(const std::function<void(int, int, int, int)> &cb);
    void setProcessCloseCallback(const std::function<void(void*)> &cb) {
        processCloseCallback_ = cb;
    }

    void *hwnd() { return currProcess_ ? currProcess_->hwnd : nullptr; }
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

    [[nodiscard]] inline const std::wstring &gameName() const { return currProcess_->gameName; }
    [[nodiscard]] inline const std::wstring &gamePass() const { return currProcess_->gamePass; }
    [[nodiscard]] inline const std::wstring &region() const { return currProcess_->region; }
    [[nodiscard]] inline const std::wstring &season() const { return currProcess_->season; }

    void reloadConfig() { loadFromCfg(); currProcess_ = nullptr; }

    /* functions for plugins */
    Skill *getSkill(uint16_t id);

private:
    void searchForProcess(void *hwnd);
    void loadFromCfg();

private:
    std::chrono::steady_clock::time_point nextSearchTime_;
    std::chrono::steady_clock::duration searchInterval_;

    std::unordered_map<void*, ProcessData> processes_;
    ProcessData *currProcess_ = nullptr;

    std::function<void(void*)> processCloseCallback_;
};

}

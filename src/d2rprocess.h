/*
 * Copyright (c) 2021 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#pragma once

#include <unordered_map>
#include <string>
#include <functional>
#include <cstdint>

class D2RProcess final {
public:
    struct MapMonster {
        uint32_t txtFileNo;
        uint32_t x, y;
        const std::array<std::wstring, 13> *name;
        wchar_t enchants[32];
        uint8_t flag;
    };
    struct MapObject {
        uint32_t txtFileNo;
        uint32_t x, y;
        const std::array<std::wstring, 13> *name;
        uint8_t type;
        uint8_t flag;
    };
public:
    explicit D2RProcess(uint32_t searchInterval = 500);
    ~D2RProcess();

    void updateData();
    void setWindowPosCallback(const std::function<void(int, int, int, int)> &cb);

    void *hwnd() { return hwnd_; }
    [[nodiscard]] inline bool available() const { return available_; }
    [[nodiscard]] inline bool mapEnabled() const { return mapEnabled_ != 0; }
    [[nodiscard]] inline const char *name() const { return name_; }
    [[nodiscard]] inline uint32_t act() const { return act_; }
    [[nodiscard]] inline uint8_t difficulty() const { return difficulty_; }
    [[nodiscard]] inline uint32_t levelId() const { return levelId_; }
    [[nodiscard]] inline uint32_t seed() const { return seed_; }
    [[nodiscard]] inline uint16_t posX() const { return posX_; }
    [[nodiscard]] inline uint16_t posY() const { return posY_; }

    [[nodiscard]] inline const std::unordered_map<uint32_t, MapMonster> &monsters() const { return mapMonsters_; }
    [[nodiscard]] inline const std::unordered_map<uint32_t, MapObject> &objects() const { return mapObjects_; }

private:
    void searchForProcess();
    void resetData();

private:
    void *handle_ = nullptr;
    void *hwnd_ = nullptr;
    void *hook_ = nullptr;
    bool available_ = false;
    uint64_t baseAddr_ = 0;
    uint64_t baseSize_ = 0;

    uint32_t nextSearchTime_ = 0;
    uint32_t searchInterval_ = 0;

    uint64_t playerHashOffset_ = 0;
    uint64_t playerPtrOffset_ = 0;

    uint8_t mapEnabled_ = 0;
    char name_[16] = {};
    uint32_t act_ = 0;
    uint8_t difficulty_ = 0;
    uint32_t levelId_ = 0;
    uint32_t seed_ = 0;
    uint16_t posX_ = 0, posY_ = 0;

    std::unordered_map<uint32_t, MapMonster> mapMonsters_;
    std::unordered_map<uint32_t, MapObject> mapObjects_;
};

/*
 * Copyright (c) 2021 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#pragma once

#include <functional>
#include <cstdint>

class D2RProcess final {
public:
    explicit D2RProcess(uint32_t searchInterval = 1);
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

    uint64_t playerUnitOffset_ = 0;
    uint64_t mapEnablePtr_ = 0;

    uint8_t mapEnabled_ = 0;
    char name_[16] = {};
    uint32_t act_ = 0;
    uint8_t difficulty_ = 0;
    uint32_t levelId_ = 0;
    uint32_t seed_ = 0;
    uint16_t posX_ = 0, posY_ = 0;
};

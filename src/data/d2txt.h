/*
 * Copyright (c) 2021 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#pragma once

#include <unordered_map>
#include <vector>
#include <iostream>
#include <string>
#include <cstddef>

namespace data {

class D2TXT final {
public:
    D2TXT() = default;
    D2TXT(const D2TXT &other) = delete;

    bool load(const std::string &filename);
    bool load(const void *data, size_t size);
    [[nodiscard]] inline size_t rows() const { return values_.size() / columns_; }
    [[nodiscard]] inline size_t columns() const { return columns_; }
    [[nodiscard]] inline int colIndexByName(const std::string &name) const {
        const auto ite = colIdxByName_.find(name);
        if (ite == colIdxByName_.end()) {
            return -1;
        }
        return ite->second;
    }
    [[nodiscard]] inline const std::pair<std::string, int> &value(size_t row, size_t col) const {
        static std::pair<std::string, int> empty = {"", 0};
        if (columns_ == 0) {
            return empty;
        }
        auto idx = row * columns_ + col;
        if (idx >= values_.size()) { return empty; }
        return values_[idx];
    }

private:
    void loadInternal(std::istream &stm);

private:
    size_t columns_ = 0;
    std::unordered_map<std::string, int> colIdxByName_;
    std::vector<std::pair<std::string, int>> values_;
};

}

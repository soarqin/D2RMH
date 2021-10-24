/*
 * Copyright (c) 2021 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#include "d2txt.h"

#include "viewstream.h"
#include <fstream>
#include <sstream>

bool D2TXT::load(const std::string &filename) {
    std::ifstream ifs(filename);
    if (!ifs.is_open()) { return false; }
    loadInternal(ifs);
    ifs.close();
    return true;
}

bool D2TXT::load(const void *data, size_t size) {
    std::string_view sv(static_cast<const char*>(data), size);
    view_istream<char> stm(sv);
    loadInternal(stm);
    return true;
}

void D2TXT::loadInternal(std::istream &stm) {
    bool processedTitle = false;
    for (std::string line; std::getline(stm, line); ) {
        std::istringstream stm2(line);
        size_t col = 0;
        if (processedTitle) {
            auto base = values_.size();
            values_.resize(base + columns_);
            for (std::string val; col < columns_ && std::getline(stm2, val, '\t'); col++) {
                values_[base + col] = std::make_pair(val, std::strtol(val.c_str(), nullptr, 0));
            }
        } else {
            for (std::string val; std::getline(stm2, val, '\t'); col++) {
                colIdxByName_[val] = int(col);
            }
            columns_ = col;
            processedTitle = true;
        }
    }
}

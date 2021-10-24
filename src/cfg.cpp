/*
 * Copyright (c) 2021 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#include "cfg.h"

#include "json/json.hpp"
#include <fstream>

static Cfg sCfg;
const Cfg *cfg = &sCfg;

template<typename T, typename V>
inline void loadval(nlohmann::json &j, T &val, const std::string &key, const V &defVal) {
    if (j.contains(key)) {
        val = j[key].get<T>();
    } else {
        val = defVal;
    }
}

void loadCfg(const std::string &filename) {
    std::ifstream ifs(filename);
    if (!ifs.is_open()) { return; }
    nlohmann::json j;
    ifs >> j;
    ifs.close();
    loadval(j, sCfg.d2Path, "d2_path", ".");
    loadval(j, sCfg.fontFilePath, "font_file_path", "normal.ttf");
    loadval(j, sCfg.fontSize, "font_size", 12.f);
    loadval(j, sCfg.language, "language", "enUS");
}

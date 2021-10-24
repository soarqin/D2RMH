/*
 * Copyright (c) 2021 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#pragma once

#include <string>

struct Cfg {
    std::string d2Path;
    std::string fontFilePath;
    float fontSize;
    std::string language;
};

extern void loadCfg(const std::string &filename = "D2RMH.ini");

extern const Cfg *cfg;

/*
 * Copyright (c) 2021 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#include "cfg.h"

#include "ini.h"
#include <cstring>

static Cfg sCfg;
const Cfg *cfg = &sCfg;

#define LOADVAL(n, m) else if (!strcmp(name, #n)) { sCfg.m = value; }
#define LOADVALN(n, m) else if (!strcmp(name, #n)) { sCfg.m = strtoul(value, nullptr, 0); }

void loadCfg(const std::string &filename) {
    ini_parse(filename.c_str(), [](void* user, const char* section,
                                   const char* name, const char* value)->int {
        if (!name) {
            return 1;
        }
        if (false) {}
        LOADVAL(d2_path, d2Path)
        LOADVAL(font_file_path, fontFilePath)
        LOADVALN(font_size, fontSize)
        LOADVAL(language, language)
        return 1;
    }, nullptr);
}

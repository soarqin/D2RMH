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
#define LOADVALF(n, m) else if (!strcmp(name, #n)) { sCfg.m = strtof(value, nullptr); }

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
        LOADVALN(show, show)
        LOADVALN(full_line, fullLine)
        LOADVALN(position, position)
        LOADVALF(scale, scale)
        LOADVALN(map_centered, mapCentered)
        return 1;
    }, nullptr);
    if (sCfg.scale < 1.f) { sCfg.scale = 1.f; }
    else if (sCfg.scale > 4.f) { sCfg.scale = 4.f; }
}

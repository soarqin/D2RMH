/*
 * Copyright (c) 2021 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#include "data.h"

#include "ini.h"
#include "util.h"

#include <cstring>
#include <cstdlib>

static Data sgamedata;
const Data *gamedata = &sgamedata;

void loadData() {
    int section = -1;
    ini_parse("D2RMH_data.ini", [](void* user, const char* section,
                                   const char* name, const char* value)->int {
        auto *isec = (int*)user;
        if (!name) {
            if (!strcmp(section, "guides")) { *isec = 0; }
            else if (!strcmp(section, "levels")) { *isec = 1; }
            else if (!strcmp(section, "objects")) { *isec = 2; }
            else if (!strcmp(section, "npcs")) { *isec = 3; }
            else if (!strcmp(section, "monsters")) { *isec = 4; }
            else if (!strcmp(section, "shrines")) { *isec = 5; }
            else if (!strcmp(section, "superuniques")) { *isec = 6; }
            else if (!strcmp(section, "strings")) { *isec = 7; }
            else { *isec = -1; }
            return 1;
        }
        switch (*isec) {
        case 0: {
            int from = strtol(name, nullptr, 0);
            int to;
            if (value[0] == '+') {
                to = strtol(value + 1, nullptr, 0) | 0x10000;
            } else if (value[0] == '-') {
                to = strtol(value + 1, nullptr, 0) | 0x20000;
            } else {
                to = strtol(value, nullptr, 0);
            }
            sgamedata.guides[from].insert(to);
            break;
        }
        case 1: {
            auto id = strtol(name, nullptr, 0);
            if (id >= sgamedata.levels.size()) {
                sgamedata.levels.resize(id + 1);
            }
            sgamedata.levels[id] = value;
            break;
        }
        case 2: case 3: {
            const char *pos = strchr(value, '|');
            if (!pos) { break; }
            auto ssize = pos - value;
            EObjType t = TypeNone;
            if (!strncmp(value, "Waypoint", ssize)) { t = TypeWayPoint; }
            else if (!strncmp(value, "Quest", ssize)) { t = TypeQuest; }
            else if (!strncmp(value, "Portal", ssize)) { t = TypePortal; }
            else if (!strncmp(value, "Chest", ssize)) { t = TypeChest; }
            else if (!strncmp(value, "Shrine", ssize)) { t = TypeShrine; }
            else if (!strncmp(value, "Well", ssize)) { t = TypeWell; }
            sgamedata.objects[*isec - 2][strtol(name, nullptr, 0)] = { t, pos + 1 };
            break;
        }
        case 4: {
            auto id = strtol(name, nullptr, 0);
            if (id >= sgamedata.monsters.size()) {
                sgamedata.monsters.resize(id + 1);
            }
            const char *pos = strchr(value, '|');
            if (pos) {
                sgamedata.monsters[id] = { std::string(value, pos), strtoul(pos + 1, nullptr, 0) > 0 };
            } else {
                sgamedata.monsters[id] = { value, false };
            }
            break;
        }
        case 5: {
            auto id = strtol(name, nullptr, 0);
            if (id >= sgamedata.shrines.size()) {
                sgamedata.shrines.resize(id + 1);
            }
            sgamedata.shrines[id] = value;
            break;
        }
        case 6: {
            auto id = strtol(name, nullptr, 0);
            if (id >= sgamedata.superUniques.size()) {
                sgamedata.superUniques.resize(id + 1);
            }
            sgamedata.superUniques[id] = value;
            break;
        }
        case 7: {
            const char *pos = strchr(name, '[');
            if (!pos) { break; }
            auto index = strtoul(pos + 1, nullptr, 0);
            if (index < 0 || index >= 13) { break; }
            char realname[256];
            auto ssize = pos - name;
            memcpy(realname, name, ssize);
            realname[ssize] = 0;
            sgamedata.strings[realname][index] = utf8toucs4(value);
            break;
        }
        default:
            break;
        }
        return 1;
    }, &section);
}

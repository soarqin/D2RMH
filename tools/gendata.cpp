/*
 * Copyright (c) 2021 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#include "d2txt.h"
#include "jsonlng.h"

#include "CascLib.h"
#include "ini.h"

#include <fstream>
#include <map>
#include <set>

char *loadFileToMem(HANDLE storage, const char *filename, size_t &size) {
    HANDLE file;
    if (!CascOpenFile(storage, filename, CASC_LOCALE_ALL, CASC_OPEN_BY_NAME, &file)) {
        CascCloseStorage(storage);
        return nullptr;
    }
    size = CascGetFileSize(file, nullptr);
    char *data = new char[size];
    CascReadFile(file, data, size, nullptr);
    CascCloseFile(file);
    return data;
}

bool loadTxt(D2TXT &txt, HANDLE storage, const char *filename) {
    size_t size;
    auto *data = loadFileToMem(storage, filename, size);
    if (!data) {
        return false;
    }
    txt.load(data, size);
    delete[] data;
    return true;
}

bool loadJsonLng(JsonLng &jlng, HANDLE storage, const char *filename) {
    size_t size;
    auto *data = loadFileToMem(storage, filename, size);
    if (!data) {
        return false;
    }
    jlng.load(data, size);
    delete[] data;
    return true;
}

int main(int argc, char *argv[]) {
    struct UserParseData {
        /* -1-not used  0-guides  1-useful_names  2-useful_objects  3-name_replace */
        int section = -1;
        std::map<int, std::pair<std::string, std::string>> usefulObjectOp, usefulObjects, usefulNpcs;
        std::map<std::string, std::set<std::string>> guides;
    } parseData;
    ini_parse("gendata.ini", [](void* user, const char* section, const char* name, const char* value)->int {
        auto *pd = (UserParseData*)user;
        if (!name) {
            if (!strcmp(section, "guides")) {
                pd->section = 0;
            } else if (!strcmp(section, "useful_object_op")) {
                pd->section = 1;
            } else if (!strcmp(section, "useful_objects")) {
                pd->section = 2;
            } else if (!strcmp(section, "useful_npcs")) {
                pd->section = 3;
            } else {
                pd->section = -1;
            }
            return 1;
        }
        switch (pd->section) {
        case 0: {
            if (strtoul(value, nullptr, 0) == 0) {
                break;
            }
            if (strncmp(name, "guide[", 6) != 0) {
                break;
            }
            const char *start = name + 6;
            const char *token = strchr(name, ']');
            if (token == nullptr) { break; }
            const char *start2 = strchr(token + 1, '[');
            if (start2 == nullptr) { break; }
            ++start2;
            const char *token2 = strchr(start2, ']');
            if (token2 == nullptr) { break; }
            pd->guides[std::string(start, token)].insert(std::string(start2, token2));
            break;
        }
        case 1: {
            const char *token = strchr(value, ',');
            pd->usefulObjectOp[strtol(name, nullptr, 0)] = token == nullptr ? std::make_pair(value, "")
                                                                            : std::make_pair(std::string(value, token), token + 1);
            break;
        }
        case 2: {
            const char *token = strchr(value, ',');
            pd->usefulObjects[strtol(name, nullptr, 0)] = token == nullptr ? std::make_pair(value, "")
                                                                           : std::make_pair(std::string(value, token), token + 1);
            break;
        }
        case 3: {
            const char *token = strchr(value, ',');
            pd->usefulNpcs[strtol(name, nullptr, 0)] = token == nullptr ? std::make_pair(value, "")
                                                                        : std::make_pair(std::string(value, token), token + 1);
            break;
        }
        default:
            break;
        }
        return 1;
    }, &parseData);

    HANDLE storage;
    char path[512];
    snprintf(path, 512, "%s\\Data", argv[1]);
    if (!CascOpenStorage(path, CASC_LOCALE_ALL, &storage)) {
        return -1;
    }

    JsonLng jlng;
    loadJsonLng(jlng, storage, "data:data/local/lng/strings/item-names.json");
    loadJsonLng(jlng, storage, "data:data/local/lng/strings/levels.json");
    loadJsonLng(jlng, storage, "data:data/local/lng/strings/monsters.json");
    loadJsonLng(jlng, storage, "data:data/local/lng/strings/npcs.json");
    loadJsonLng(jlng, storage, "data:data/local/lng/strings/objects.json");
    loadJsonLng(jlng, storage, "data:data/local/lng/strings/shrines.json");

    D2TXT levelTxt, objTxt, monTxt, shrineTxt, superuTxt;
    loadTxt(levelTxt, storage, "data:data/global/excel/levels.txt");
    loadTxt(objTxt, storage, "data:data/global/excel/objects.txt");
    loadTxt(monTxt, storage, "data:data/global/excel/monstats.txt");
    loadTxt(shrineTxt, storage, "data:data/global/excel/shrines.txt");
    loadTxt(superuTxt, storage, "data:data/global/excel/superuniques.txt");
    CascCloseStorage(storage);

    std::map<std::string, std::array<std::string, JsonLng::LNG_MAX>> strings;
    std::map<std::string, int> levelIdByName;

    std::ofstream ofs("D2RMH_data.ini");

    ofs << ";!!!THIS FILE IS GENERATED BY TOOL, DO NOT EDIT!!!" << std::endl << std::endl;
    ofs << '[' << "levels" << ']' << std::endl;
    auto idx0 = levelTxt.colIndexByName("Id");
    auto idx1 = levelTxt.colIndexByName("LevelName");
    auto rows = levelTxt.rows();
    for (size_t i = 0; i < rows; ++i) {
        auto id = levelTxt.value(i, idx0).second;
        const auto &key = levelTxt.value(i, idx1).first;
        const auto *arr = jlng.get(key);
        if (arr) {
            strings[key] = *arr;
            ofs << id << '=' << key << std::endl;
            levelIdByName[(*arr)[0]] = id;
        }
    }

    ofs << std::endl << '[' << "objects" << ']' << std::endl;
    idx0 = objTxt.colIndexByName("*ID");
    idx1 = objTxt.colIndexByName("Name");
    auto idx2 = objTxt.colIndexByName("OperateFn");
    rows = objTxt.rows();
    for (size_t i = 0; i < rows; ++i) {
        auto id = objTxt.value(i, idx0).second;
        auto op = objTxt.value(i, idx2).second;
        auto kite = parseData.usefulObjectOp.find(op);
        std::string typeStr;
        std::string key;
        if (kite == parseData.usefulObjectOp.end()) {
            auto ite = parseData.usefulObjects.find(id);
            if (ite == parseData.usefulObjects.end()) { continue; }
            key = ite->second.second;
            typeStr = ite->second.first;
        } else {
            key = kite->second.second;
            typeStr = kite->second.first;
        }
        if (key.empty()) { key = objTxt.value(i, idx1).first; }
        const auto *arr = jlng.get(key);
        if (arr) {
            strings[key] = *arr;
            ofs << id << '=' << typeStr << '|' << key << std::endl;
        }
    }

    idx0 = monTxt.colIndexByName("*hcIdx");
    idx1 = monTxt.colIndexByName("NameStr");
    idx2 = monTxt.colIndexByName("npc");
    ofs << std::endl << '[' << "npcs" << ']' << std::endl;
    rows = monTxt.rows();
    for (size_t i = 0; i < rows; ++i) {
        auto id = monTxt.value(i, idx0).second;
        auto ite = parseData.usefulNpcs.find(id);
        if (ite == parseData.usefulNpcs.end()) {
            continue;
        }
        std::string key = ite->second.second;
        if (key.empty()) { key = monTxt.value(i, idx1).first; }
        const auto *arr = jlng.get(key);
        if (arr) {
            strings[key] = *arr;
            ofs << id << '=' << ite->second.first << '|' << key << std::endl;
        }
    }

    ofs << std::endl << '[' << "monsters" << ']' << std::endl;
    for (size_t i = 0; i < rows; ++i) {
        auto id = monTxt.value(i, idx0).second;
        auto key = monTxt.value(i, idx1).first;
        const auto *arr = jlng.get(key);
        if (arr) {
            strings[key] = *arr;
            ofs << id << '=' << key << '|' << monTxt.value(i, idx2).second << std::endl;
        }
    }

    idx0 = shrineTxt.colIndexByName("Code");
    idx1 = shrineTxt.colIndexByName("StringName");
    ofs << std::endl << '[' << "shrines" << ']' << std::endl;
    rows = shrineTxt.rows();
    for (size_t i = 0; i < rows; ++i) {
        auto id = shrineTxt.value(i, idx0).second;
        auto key = shrineTxt.value(i, idx1).first;
        const auto *arr = jlng.get(key);
        if (arr) {
            strings[key] = *arr;
            ofs << id << '=' << key << std::endl;
        }
    }

    idx0 = superuTxt.colIndexByName("hcIdx");
    idx1 = superuTxt.colIndexByName("Name");
    ofs << std::endl << '[' << "superuniques" << ']' << std::endl;
    rows = superuTxt.rows();
    for (size_t i = 0; i < rows; ++i) {
        auto id = superuTxt.value(i, idx0).second;
        auto key = superuTxt.value(i, idx1).first;
        const auto *arr = jlng.get(key);
        if (arr) {
            strings[key] = *arr;
            ofs << id << '=' << key << std::endl;
        }
    }

    ofs << std::endl << '[' << "strings" << ']' << std::endl;
    for (auto &p: strings) {
        for (size_t i = 0; i < p.second.size(); ++i) {
            ofs << p.first << '[' << i << "]=" << p.second[i] << std::endl;
        }
    }

    ofs << std::endl << '[' << "guides" << ']' << std::endl;
    for (auto &p: parseData.guides) {
        int id;
        if (p.first[0] >= '0' && p.first[0] <= '9') {
            id = strtol(p.first.c_str(), nullptr, 0);
        } else {
            auto ite = levelIdByName.find(p.first);
            if (ite == levelIdByName.end()) {
                std::cerr << "Not found: " << p.first << std::endl;
                continue;
            }
            id = ite->second;
        }
        for (auto &s: p.second) {
            if (s.empty()) { continue; }
            if (s[0] == '+' || s[0] == '-' || (s[0] >= '0' && s[0] <= '9')) {
                ofs << id << '=' << s << std::endl;
                continue;
            }
            auto ite2 = levelIdByName.find(s);
            if (ite2 == levelIdByName.end()) {
                std::cerr << "Not found: " << s << std::endl;
                continue;
            }
            ofs << id << '=' << ite2->second << std::endl;
        }
    }
    ofs.close();
    return 0;
}

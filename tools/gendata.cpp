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
    wchar_t wpath[512];
    snprintf(path, 512, "%s\\Data", argv[1]);
    MultiByteToWideChar(CP_ACP, 0, path, 512, wpath, 512);
    if (!CascOpenStorage(wpath, CASC_LOCALE_ALL, &storage)) {
        HKEY key;
        if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Diablo II Resurrected", 0, KEY_READ, &key) == ERROR_SUCCESS) {
            wchar_t regpath[MAX_PATH];
            DWORD pathSize = sizeof(regpath);
            if (RegQueryValueExW(key, L"InstallSource", nullptr, nullptr, LPBYTE(regpath), &pathSize) == ERROR_SUCCESS) {
                if (!CascOpenStorage(regpath, CASC_LOCALE_ALL, &storage)) {
                    RegCloseKey(key);
                    return -1;
                }
            }
            RegCloseKey(key);
        }
    }

    JsonLng jlng, jlngMerc;
    loadJsonLng(jlng, storage, "data:data/local/lng/strings/item-names.json");
    loadJsonLng(jlng, storage, "data:data/local/lng/strings/item-runes.json");
    loadJsonLng(jlng, storage, "data:data/local/lng/strings/item-nameaffixes.json");
    loadJsonLng(jlng, storage, "data:data/local/lng/strings/item-modifiers.json");
    loadJsonLng(jlng, storage, "data:data/local/lng/strings/levels.json");
    loadJsonLng(jlng, storage, "data:data/local/lng/strings/monsters.json");
    loadJsonLng(jlng, storage, "data:data/local/lng/strings/npcs.json");
    loadJsonLng(jlng, storage, "data:data/local/lng/strings/objects.json");
    loadJsonLng(jlng, storage, "data:data/local/lng/strings/shrines.json");
    loadJsonLng(jlng, storage, "data:data/local/lng/strings/ui.json");
    loadJsonLng(jlngMerc, storage, "data:data/local/lng/strings/mercenaries.json");
    jlng.remove("dummy");
    jlng.remove("Dummy");
    jlng.remove("unused");

    D2TXT levelTxt, objTxt, monTxt, shrineTxt, superuTxt;
    D2TXT itemTxt[3];
    loadTxt(levelTxt, storage, "data:data/global/excel/levels.txt");
    loadTxt(objTxt, storage, "data:data/global/excel/objects.txt");
    loadTxt(monTxt, storage, "data:data/global/excel/monstats.txt");
    loadTxt(shrineTxt, storage, "data:data/global/excel/shrines.txt");
    loadTxt(superuTxt, storage, "data:data/global/excel/superuniques.txt");
    loadTxt(itemTxt[0], storage, "data:data/global/excel/weapons.txt");
    loadTxt(itemTxt[1], storage, "data:data/global/excel/armor.txt");
    loadTxt(itemTxt[2], storage, "data:data/global/excel/misc.txt");
    CascCloseStorage(storage);

    std::map<std::string, std::array<std::string, JsonLng::LNG_MAX>> strings;
    std::map<std::string, int> levelIdByName;

    std::ofstream ofs("D2RMH_data.ini");
    std::ofstream ofs2("ItemDesc.csv");
    ofs2 << /* UTF-8 BOM */ "\xEF\xBB\xBF" << "index,code,enUS,zhTW,deDE,esES,frFR,itIT,koKR,plPL,esMX,jaJP,ptBR,ruRU,zhCN" << std::endl;

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
    auto idx3 = objTxt.colIndexByName("SizeX");
    auto idx4 = objTxt.colIndexByName("SizeY");
    auto idx5 = objTxt.colIndexByName("IsDoor");
    rows = objTxt.rows();
    for (size_t i = 0; i < rows; ++i) {
        auto id = objTxt.value(i, idx0).second;
        if (objTxt.value(i, idx5).second) {
            auto w = std::max(1, objTxt.value(i, idx3).second);
            auto h = std::max(1, objTxt.value(i, idx4).second);
            ofs << id << '=' << "Door|" << w << ',' << h << std::endl;
            continue;
        }
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
            auto minValue = typeStr == "Waypoint" ? 3 : 2;
            auto w = std::max(minValue, objTxt.value(i, idx3).second);
            auto h = std::max(minValue, objTxt.value(i, idx4).second);
            ofs << id << '=' << typeStr << '|' << key << '|' << w << ',' << h << std::endl;
        }
    }

    idx0 = monTxt.colIndexByName("*hcIdx");
    idx1 = monTxt.colIndexByName("NameStr");
    idx2 = monTxt.colIndexByName("npc");
    idx3 = monTxt.colIndexByName("Align");
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
            ofs << id << '=' << key << '|' << (monTxt.value(i, idx2).second ? 1 : monTxt.value(i, idx3).second ? 2 : 0) << std::endl;
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

    uint32_t itemIndex = 0;
    ofs << std::endl << '[' << "items" << ']' << std::endl;
    for (const auto &item : itemTxt) {
        idx0 = item.colIndexByName("name");
        idx1 = item.colIndexByName("code");
        idx2 = item.colIndexByName("namestr");
        rows = item.rows();
        for (size_t i = 0; i < rows; ++i) {
            const auto &name = item.value(i, idx0).first;
            if (name == "Expansion") { continue; }
            const auto &code = item.value(i, idx1).first;
            const auto &key = item.value(i, idx2).first;
            const auto *arr = jlng.get(key);
            if (arr) {
                strings[key] = *arr;
                ofs << itemIndex << '=' << code << '|' << key << std::endl;
                ofs2 << itemIndex << ',' << code << "," << (*arr)[0]
                    << "," << (*arr)[1] << "," << (*arr)[2]
                    << "," << (*arr)[3] << "," << (*arr)[4]
                    << "," << (*arr)[5] << "," << (*arr)[6]
                    << "," << (*arr)[7] << "," << (*arr)[8]
                    << "," << (*arr)[9] << "," << (*arr)[10]
                    << "," << (*arr)[11] << "," << (*arr)[12] << std::endl;
            }
            itemIndex++;
        }
    }

    ofs << std::endl << '[' << "mercnames" << ']' << std::endl;
    jlngMerc.iterateById([&ofs, &jlngMerc, &strings](uint32_t id, const std::string &key) {
        const auto *arr = jlngMerc.get(key);
        if (arr) {
            strings[key] = *arr;
            ofs << id << '=' << key << std::endl;
        }
    });

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
    ofs2.close();
    ofs.close();
    return 0;
}

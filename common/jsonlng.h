/*
 * Copyright (c) 2021 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#pragma once

#include <map>
#include <array>
#include <string>
#include <istream>

class JsonLng final {
public:
    enum LNG {
        LNG_enUS,
        LNG_zhTW,
        LNG_deDE,
        LNG_esES,
        LNG_frFR,
        LNG_itIT,
        LNG_koKR,
        LNG_plPL,
        LNG_esMX,
        LNG_jaJP,
        LNG_ptBR,
        LNG_ruRU,
        LNG_zhCN,
        LNG_MAX,
    };

    bool load(const std::string &filename);
    void load(const void *data, size_t size);

    [[nodiscard]] const std::string &get(const std::string &key, LNG lang, LNG fallback = LNG_enUS) const;
    [[nodiscard]] const std::array<std::string, LNG_MAX> *get(const std::string &key) const;

private:
    void loadInternal(std::istream &stm);

private:
    std::map<std::string, std::array<std::string, LNG_MAX>> strings_;
};

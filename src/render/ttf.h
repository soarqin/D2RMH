/*
 * Copyright (c) 2021 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#pragma once

#include "fontrenderimpl.h"
#include "font.h"

#include <vector>
#include <unordered_map>
#include <string>
#include <cstdint>

namespace render {

class TTF: public Font {
protected:
    struct FontInfo {
#ifdef USE_FREETYPE
        FT_Face face = nullptr;
#else
        void *font = nullptr;
        std::vector<uint8_t> ttf_buffer;
#endif
    };
public:
    explicit TTF(FontRenderImpl &renderImpl);
    ~TTF() override;

    bool add(const std::string &filename, int param) override;

protected:
    bool makeCache(FontData *fd, uint32_t ch, int fontSize) override;

protected:
    std::vector<FontInfo> fonts_;

private:
#ifdef USE_FREETYPE
    FT_Library ftLib_ = nullptr;
#endif
};

}

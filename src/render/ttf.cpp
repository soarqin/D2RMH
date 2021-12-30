/*
 * Copyright (c) 2021 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#include "ttf.h"

#ifdef USE_FREETYPE
#include <ft2build.h>
#include FT_FREETYPE_H
#else
#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_STATIC
#include <stb_truetype.h>
#endif

#include <fstream>

namespace render {

TTF::TTF(FontRenderImpl &renderImpl) : Font(renderImpl) {
#ifdef USE_FREETYPE
    FT_Init_FreeType(&ftLib_);
#endif
}

TTF::~TTF() {
    for (auto &p: fonts_) {
#ifdef USE_FREETYPE
        FT_Done_Face(p.face);
#else
        delete static_cast<stbtt_fontinfo *>(p.font);
        p.ttf_buffer.clear();
#endif
    }
    fonts_.clear();
#ifdef USE_FREETYPE
    FT_Done_FreeType(ftLib_);
#endif
}

bool TTF::add(const std::string &filename, int param) {
    FontInfo fi;
#ifdef USE_FREETYPE
    if (FT_New_Face(ftLib_, filename.c_str(), param, &fi.face)) {
        return false;
    }
    fonts_.emplace_back(fi);
#else
    std::ifstream ifs(filename, std::ios::binary | std::ios::in);
    if (!ifs.is_open()) {
        return false;
    }
    ifs.seekg(0, std::ios::end);
    auto size = ifs.tellg();
    ifs.seekg(0, std::ios::beg);
    fi.ttf_buffer.resize(size_t(size));
    ifs.read((char *)fi.ttf_buffer.data(), size);
    ifs.close();
    auto *info = new stbtt_fontinfo;
    stbtt_InitFont(info, &fi.ttf_buffer[0], stbtt_GetFontOffsetForIndex(&fi.ttf_buffer[0], param));
    fi.font = info;
    fonts_.emplace_back(std::move(fi));
#endif
    return true;
}

bool TTF::makeCache(TTF::FontData *fd, uint32_t ch, int fontSize) {
    FontInfo *fi = nullptr;
#ifndef USE_FREETYPE
    stbtt_fontinfo *info;
    uint32_t index = 0;
#endif

    for (auto &f: fonts_) {
#ifdef USE_FREETYPE
        auto index = FT_Get_Char_Index(f.face, ch);
        if (index == 0) continue;
        FT_Set_Pixel_Sizes(f.face, 0, fontSize);
        auto err = FT_Load_Glyph(f.face, index, FT_LOAD_DEFAULT);
        if (!err) { fi = &f; break; }
#else
        info = static_cast<stbtt_fontinfo *>(f.font);
        index = stbtt_FindGlyphIndex(info, ch);
        if (index != 0) {
            fi = &f;
            break;
        }
#endif
    }
    if (fi == nullptr) {
        memset(fd, 0, sizeof(FontData));
        return false;
    }

#ifdef USE_FREETYPE
    unsigned char *srcPtr;
    int bitmapPitch;
    if (FT_Render_Glyph(fi->face->glyph, FT_RENDER_MODE_NORMAL)) return nullptr;
    FT_GlyphSlot slot = fi->face->glyph;
    fd->ix0 = slot->bitmap_left;
    fd->iy0 = fontSize * 7 / 8 - slot->bitmap_top;
    fd->w = slot->bitmap.width;
    fd->h = slot->bitmap.rows;
    fd->advW = slot->advance.x >> 6;
    srcPtr = slot->bitmap.buffer;
    bitmapPitch = slot->bitmap.pitch;
#else
    /* Read font data to cache */
    int advW, leftB;
    float fontScale = stbtt_ScaleForMappingEmToPixels(info, static_cast<float>(fontSize));
    stbtt_GetGlyphHMetrics(info, index, &advW, &leftB);
    int ascent, descent;
    stbtt_GetFontVMetrics(info, &ascent, &descent, nullptr);
    fd->advW = uint8_t(std::lround(fontScale * float(advW)));
    int w, h, x, y;
    stbtt_GetGlyphBitmap(info, fontScale, fontScale, index, &w, &h, &x, &y);
    fd->ix0 = x;
    fd->iy0 = int(float(ascent + descent) * fontScale) + y;
    fd->w = w;
    fd->h = h;
#endif
    fd->origW = uint8_t(uint32_t(fontSize));

    int dstPitch = int((fd->w + 1u) & ~1u);
    std::vector<uint8_t> dst(dstPitch * fd->h);

#ifdef USE_FREETYPE
    auto *dstPtr = dst.data();
    for (int k = 0; k < fd->h; ++k) {
        memcpy(dstPtr, srcPtr, fd->w);
        srcPtr += bitmapPitch;
        dstPtr += dstPitch;
    }
#else
    stbtt_MakeGlyphBitmapSubpixel(info, dst.data(), fd->w, fd->h, dstPitch, fontScale, fontScale, 0, 0, index);
#endif

    std::vector<uint32_t> dst2(dstPitch * fd->h);
    uint8_t *ptr = dst.data();
    uint32_t *ptr2 = dst2.data();
    for (int j = fd->h; j; --j) {
        for (int i = dstPitch; i; --i) {
            *ptr2++ = (uint32_t(*ptr++) << 24) | 0xFFFFFFu;
        }
    }
    if (!updateTexture(fd->rpidx, fd->rpx, fd->rpy, dstPitch, fd->h, (const uint8_t *)dst2.data())) {
        memset(fd, 0, sizeof(FontData));
        return false;
    }
    return true;
}

}

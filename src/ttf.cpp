/*
 * Copyright (c) 2021 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#include "ttf.h"

#define STB_RECT_PACK_IMPLEMENTATION
#define STBRP_STATIC
#include <stb_rect_pack.h>

#ifdef USE_FREETYPE
#include <ft2build.h>
#include FT_FREETYPE_H
#else
#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_STATIC
#include <stb_truetype.h>
#endif

#include <fstream>

enum {
    RectPackWidthDefault = 1024,
};

struct RectPackData {
    explicit RectPackData(int nodeCount): nodes(new stbrp_node[nodeCount]) {
    }
    ~RectPackData() {
        delete[] nodes;
    }
    stbrp_context context = {};
    stbrp_node *nodes;
};

class RectPacker final {
public:
    explicit RectPacker(int width = RectPackWidthDefault, int height = RectPackWidthDefault): width_(width), height_(height) {}
    ~RectPacker() {
        for (auto *rpd: rectpackData_) {
            delete rpd;
        }
        rectpackData_.clear();
    }
    int pack(uint16_t w, uint16_t h, int16_t &x, int16_t &y) {
        if (rectpackData_.empty()) {
            newRectPack();
        }
        auto sz = int(rectpackData_.size());
        stbrp_rect rc = {0, w, h};
        int rpidx = -1;
        for (int i = 0; i < sz; ++i) {
            auto &rpd = rectpackData_[i];
            if (stbrp_pack_rects(&rpd->context, &rc, 1)) {
                rpidx = i;
                break;
            }
        }
        if (rpidx < 0) {
            /* No space to hold the bitmap,
             * create a new bitmap */
            newRectPack();
            auto &rpd = rectpackData_.back();
            if (stbrp_pack_rects(&rpd->context, &rc, 1)) {
                rpidx = int(rectpackData_.size()) - 1;
            }
        }
        x = rc.x;
        y = rc.y;
        return rpidx;
    }

private:
    void newRectPack() {
        rectpackData_.resize(rectpackData_.size() + 1);
        auto *&rpd = rectpackData_.back();
        rpd = new RectPackData(width_);
        stbrp_init_target(&rpd->context, width_, height_, rpd->nodes, width_);
    }

private:
    int width_, height_;
    std::vector<RectPackData*> rectpackData_;
};

TTF::TTF(TTFRenderImpl &renderImpl): renderImpl_(renderImpl), rectpacker_(new RectPacker(RectPackWidthDefault, RectPackWidthDefault)) {
#ifdef USE_FREETYPE
    FT_Init_FreeType(&ftLib_);
#endif
}

TTF::~TTF() {
    deinit();
#ifdef USE_FREETYPE
    FT_Done_FreeType(ftLib_);
#endif
}

void TTF::init(int size, uint8_t width) {
    fontSize_ = size;
    monoWidth_ = width;
}

void TTF::deinit() {
    for (auto &tex: textures_) {
        renderImpl_.destroyTexture(tex);
    }
    textures_.clear();
    fontCache_.clear();
    for (auto &p: fonts_) {
#ifdef USE_FREETYPE
        FT_Done_Face(p.face);
#else
        delete static_cast<stbtt_fontinfo *>(p.font);
        p.ttf_buffer.clear();
#endif
    }
    fonts_.clear();
}

bool TTF::add(const std::string &filename, int index) {
    FontInfo fi;
#ifdef USE_FREETYPE
    if (FT_New_Face(ftLib_, filename.c_str(), index, &fi.face)) {
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
    fi.ttf_buffer.resize(size);
    ifs.read((char*)fi.ttf_buffer.data(), size);
    ifs.close();
    auto *info = new stbtt_fontinfo;
    stbtt_InitFont(info, &fi.ttf_buffer[0], stbtt_GetFontOffsetForIndex(&fi.ttf_buffer[0], index));
    fi.font = info;
    fonts_.emplace_back(std::move(fi));
#endif
    return true;
}

void TTF::charDimension(uint32_t ch, uint8_t &width, int8_t &t, int8_t &b, int fontSize) {
    if (fontSize < 0) fontSize = fontSize_;
    const FontData *fd;
    uint64_t key = (uint64_t(fontSize) << 32) | uint64_t(ch);
    auto ite = fontCache_.find(key);
    if (ite == fontCache_.end()) {
        fd = makeCache(ch, fontSize);
        if (!fd) {
            width = t = b = 0;
            return;
        }
    } else {
        fd = &ite->second;
        if (fd->advW == 0) {
            width = t = b = 0;
            return;
        }
    }
    if (monoWidth_)
        width = std::max(fd->advW, monoWidth_);
    else
        width = fd->advW;
    t = fd->iy0;
    b = fd->iy0 + fd->h;
}

int TTF::stringWidth(const std::wstring &str, int fontSize) {
    uint8_t w;
    int8_t t, b;
    int res = 0;
    for (auto &ch: str) {
        if (ch < 32) { continue; }
        charDimension(ch, w, t, b, fontSize);
        res += int(uint32_t(w));
    }
    return res;
}

void TTF::setColor(uint8_t r, uint8_t g, uint8_t b) {
    altR_[0] = r; altG_[0] = g; altB_[0] = b;
}

void TTF::setAltColor(int index, uint8_t r, uint8_t g, uint8_t b) {
    if (index > 0 && index <= 16) {
        --index;
        altR_[index] = r;
        altG_[index] = g;
        altB_[index] = b;
    }
}

void TTF::render(std::wstring_view str, float x, float y, bool shadow, int fontSize) {
    if (fontSize < 0) fontSize = fontSize_;
    int colorIndex = 0;
    renderImpl_.renderBegin();
    for (auto ch: str) {
        if (ch > 0 && ch < 17) { colorIndex = ch - 1; continue; }
        const FontData *fd;
        uint64_t key = (uint64_t(fontSize) << 32) | uint64_t(ch);
        auto ite = fontCache_.find(key);
        if (ite == fontCache_.end()) {
            fd = makeCache(ch, fontSize);
            if (!fd) {
                continue;
            }
        } else {
            fd = &ite->second;
            if (fd->advW == 0) continue;
        }
        auto *tex = textures_[fd->rpidx];
        if (shadow) {
            auto x0 = x + 2.f + fd->ix0, y0 = y + 2.f + fd->iy0;
            renderImpl_.render(tex, x0, y0, x0 + fd->w, y0 + fd->h, fd->rpx, fd->rpy, fd->rpx + fd->w, fd->rpy + fd->h, 0, 0, 0, 255);
        }
        {
            auto x0 = x + fd->ix0, y0 = y + fd->iy0;
            renderImpl_.render(tex, x0, y0, x0 + fd->w, y0 + fd->h, fd->rpx, fd->rpy, fd->rpx + fd->w, fd->rpy + fd->h, 255, 255, 255, 255);
        }
        x += fd->advW;
    }
    renderImpl_.renderEnd();
}

const TTF::FontData *TTF::makeCache(uint32_t ch, int fontSize) {
    if (fontSize < 0) fontSize = fontSize_;
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
        info = static_cast<stbtt_fontinfo*>(f.font);
        index = stbtt_FindGlyphIndex(info, ch);
        if (index != 0) { fi = &f; break; }
#endif
    }
    uint64_t key = (uint64_t(fontSize) << 32) | uint64_t(ch);
    FontData *fd = &fontCache_[key];
    if (fi == nullptr) {
        memset(fd, 0, sizeof(FontData));
        return nullptr;
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

    int dstPitch = int((fd->w + 1u) & ~1u);
    /* Get last rect pack bitmap */
    auto rpidx = rectpacker_->pack(dstPitch, fd->h, fd->rpx, fd->rpy);
    if (rpidx < 0) {
        memset(fd, 0, sizeof(FontData));
        return nullptr;
    }
    fd->rpidx = rpidx;

    uint8_t dst[64 * 64];

#ifdef USE_FREETYPE
    auto *dstPtr = dst;
    for (int k = 0; k < fd->h; ++k) {
        memcpy(dstPtr, srcPtr, fd->w);
        srcPtr += bitmapPitch;
        dstPtr += dstPitch;
    }
#else
    stbtt_MakeGlyphBitmapSubpixel(info, dst, fd->w, fd->h, dstPitch, fontScale, fontScale, 0, 0, index);
#endif

    if (rpidx >= textures_.size()) {
        textures_.resize(rpidx + 1, nullptr);
    }
    auto *tex = textures_[rpidx];
    if (tex == nullptr) {
        tex = renderImpl_.createTexture(RectPackWidthDefault, RectPackWidthDefault);
        if (tex) {
            textures_[rpidx] = tex;
        } else {
            return nullptr;
        }
    }
    uint32_t dst2[64 * 64];
    uint8_t *ptr = dst;
    uint32_t *ptr2 = dst2;
    for (int j = fd->h; j; --j) {
        for (int i = dstPitch; i; --i) {
            *ptr2++ = (uint32_t(*ptr++) << 24) | 0xFFFFFFu;
        }
    }
    renderImpl_.updateTexture(tex, fd->rpx, fd->rpy, dstPitch, fd->h, (const uint8_t*)dst2);
    return fd;
}

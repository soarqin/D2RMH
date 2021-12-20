/*
 * Copyright (c) 2021 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <cstdint>

namespace render {

class RectPacker;

class TTFRenderImpl {
public:
    virtual void *createTexture(int width, int height) = 0;
    virtual void destroyTexture(void *tex) = 0;
    virtual void updateTexture(void *tex, int x, int y, int w, int h, const uint8_t *data) = 0;
    virtual void renderBegin() = 0;
    virtual void render(void *tex, float x0, float y0, float x1, float y1, int u0, int v0, int u1, int v1, uint32_t color) = 0;
    virtual void renderEnd() = 0;
};

class TTF {
protected:
    struct FontData {
        int16_t rpx, rpy;
        uint8_t rpidx;

        int8_t ix0, iy0;
        uint8_t w, h;
        uint8_t advW;
    };
    struct FontInfo {
#ifdef USE_FREETYPE
        FT_Face face = nullptr;
#else
        void *font = nullptr;
        std::vector<uint8_t> ttf_buffer;
#endif
    };
public:
    explicit TTF(TTFRenderImpl &renderImpl);
    ~TTF();

    void init(int size, uint8_t width = 0);
    void deinit();
    bool add(const std::string &filename, int index = 0);
    void charDimension(uint32_t ch, uint8_t &width, int8_t &t, int8_t &b, int fontSize = -1);
    template<typename T>
    int stringWidth(const T &str, int fontSize = -1) {
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

    inline int fontSize() const { return fontSize_; }
    void setColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);
    void setAltColor(int index, uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);

    template<typename T>
    void render(const T &str, float x, float y, bool shadow, int fontSize = -1, int preSelColorIndex = 0) {
        if (fontSize < 0) fontSize = fontSize_;
        int colorIndex = preSelColorIndex;
        renderImpl_.renderBegin();
        for (auto ch: str) {
            if (ch < 32) {
                colorIndex = ch & 0x0F;
                continue;
            }
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
                renderImpl_.render(tex, x0, y0, x0 + fd->w, y0 + fd->h, fd->rpx, fd->rpy, fd->rpx + fd->w, fd->rpy + fd->h, 0xFF000000u);
            }
            {
                auto x0 = x + fd->ix0, y0 = y + fd->iy0;
                renderImpl_.render(tex, x0, y0, x0 + fd->w, y0 + fd->h, fd->rpx, fd->rpy, fd->rpx + fd->w, fd->rpy + fd->h, altColor_[colorIndex]);
            }
            x += fd->advW;
        }
        renderImpl_.renderEnd();
    }

private:
    const FontData *makeCache(uint32_t ch, int fontSize = - 1);

protected:
    int fontSize_ = 16;
    std::vector<FontInfo> fonts_;
    std::unordered_map<uint64_t, FontData> fontCache_;
    uint8_t monoWidth_ = 0;

private:
    TTFRenderImpl &renderImpl_;

    uint32_t altColor_[16] = {};
    std::vector<void*> textures_;

    RectPacker *rectpacker_;
#ifdef USE_FREETYPE
    FT_Library ftLib_ = nullptr;
#endif
};

}

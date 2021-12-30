/*
 * Copyright (c) 2021 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#pragma once

#include "fontrenderimpl.h"
#include <unordered_map>
#include <vector>
#include <string>
#include <cstdint>

namespace render {

struct RectPacker;

class Font {
protected:
    struct FontData {
        int16_t rpx, rpy;
        int rpidx;

        int8_t ix0, iy0;
        uint8_t w, h;
        uint8_t advW;
        uint8_t origW;
    };

public:
    explicit Font(FontRenderImpl &renderImpl);
    virtual ~Font();
    virtual bool add(const std::string &filename, int param) = 0;

    void init(int size, uint8_t width = 0);
    void charDimension(uint32_t ch, uint8_t &width, int8_t &t, int8_t &b, int fontSize = -1);
    uint8_t charWidth(uint32_t ch, int fontSize = -1);
    template<typename T>
    int stringWidth(const T &str, int fontSize = -1) {
        int res = 0;
        for (auto &ch: str) {
            if (ch < 32) { continue; }
            res += int(uint32_t(charWidth(ch, fontSize)));
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
            const auto *fd = getOrMakeCache(ch, fontSize);
            if (!fd || fd->advW == 0) {
                continue;
            }
            float ratio = fontSize == fd->origW ? 1.0f : (float(fontSize) / fd->origW);
            auto *tex = textures_[fd->rpidx];
            if (shadow) {
                auto x0 = x + 2.f + fd->ix0, y0 = y + 2.f + fd->iy0;
                renderImpl_.render(tex, x0, y0, x0 + fd->w * ratio, y0 + fd->h * ratio, fd->rpx, fd->rpy, fd->rpx + fd->w, fd->rpy + fd->h, 0xFF000000u);
            }
            {
                auto x0 = x + fd->ix0, y0 = y + fd->iy0;
                renderImpl_.render(tex, x0, y0, x0 + fd->w * ratio, y0 + fd->h * ratio, fd->rpx, fd->rpy, fd->rpx + fd->w, fd->rpy + fd->h, altColor_[colorIndex]);
            }
            x += fd->advW * fontSize / fd->origW;
        }
        renderImpl_.renderEnd();
    }

protected:
    virtual bool makeCache(FontData *fd, uint32_t ch, int fontSize) = 0;
    virtual uint64_t makeFontKey(uint32_t ch, int fontSize) {
        return (uint64_t(fontSize) << 32) | uint64_t(ch);
    }
    const FontData *getOrMakeCache(uint32_t ch, int fontSize = -1);
    bool updateTexture(int &rpidx, int16_t &rpx, int16_t &rpy, int w, int h, const uint8_t *data);

protected:
    int fontSize_ = 16;
    uint8_t monoWidth_ = 0;

private:
    RectPacker *rectpacker_;
    std::unordered_map<uint64_t, FontData> fontCache_;
    std::vector<void*> textures_;
    FontRenderImpl &renderImpl_;
    uint32_t altColor_[16] = {};
};

}

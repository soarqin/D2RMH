/*
 * Copyright (c) 2021 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#include "font.h"

#define STB_RECT_PACK_IMPLEMENTATION
#define STBRP_STATIC
#include <stb_rect_pack.h>
#include <cstring>

namespace render {

enum {
    RectPackWidthDefault = 1024,
};

struct RectPackData {
    explicit RectPackData(int nodeCount) : nodes(new stbrp_node[nodeCount]) {
    }
    ~RectPackData() {
        delete[] nodes;
    }
    stbrp_context context = {};
    stbrp_node *nodes;
};

class RectPacker final {
public:
    explicit RectPacker(int width = RectPackWidthDefault, int height = RectPackWidthDefault);
    ~RectPacker();
    int pack(uint16_t w, uint16_t h, int16_t &x, int16_t &y);

private:
    void newRectPack();

private:
    int width_, height_;
    std::vector<RectPackData*> rectpackData_;
};

RectPacker::RectPacker(int width, int height)
    : width_(width), height_(height) {}
RectPacker::~RectPacker() {
    for (auto *rpd: rectpackData_) {
        delete rpd;
    }
    rectpackData_.clear();
}
int RectPacker::pack(uint16_t w, uint16_t h, int16_t &x, int16_t &y) {
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
void RectPacker::newRectPack() {
    rectpackData_.resize(rectpackData_.size() + 1);
    auto *&rpd = rectpackData_.back();
    rpd = new RectPackData(width_);
    stbrp_init_target(&rpd->context, width_, height_, rpd->nodes, width_);
}

Font::Font(FontRenderImpl &renderImpl)
    : renderImpl_(renderImpl), rectpacker_(new RectPacker(RectPackWidthDefault, RectPackWidthDefault)) {
    memset(altColor_, 0xFF, sizeof(altColor_));
}

Font::~Font() {
    fontCache_.clear();
    for (auto &tex: textures_) {
        renderImpl_.destroyTexture(tex);
    }
    textures_.clear();
    delete rectpacker_;
}

void Font::init(int size, uint8_t width) {
    fontSize_ = size;
    monoWidth_ = width;
}

void Font::charDimension(uint32_t ch, uint8_t &width, int8_t &t, int8_t &b, int fontSize) {
    const auto *fd = getOrMakeCache(ch, fontSize);
    if (!fd || fd->advW == 0) {
        width = t = b = 0;
        return;
    }
    if (fontSize == fd->origW) {
        if (monoWidth_)
            width = std::max(fd->advW, monoWidth_);
        else
            width = fd->advW;
        t = fd->iy0;
        b = fd->iy0 + fd->h;
        return;
    }
    if (monoWidth_)
        width = std::max(fd->advW, monoWidth_) * fontSize / fd->origW;
    else
        width = fd->advW * fontSize / fd->origW;
    t = fd->iy0 * fontSize / fd->origW;
    b = (fd->iy0 + fd->h) * fontSize / fd->origW;
}

uint8_t Font::charWidth(uint32_t ch, int fontSize) {
    const auto *fd = getOrMakeCache(ch, fontSize);
    if (!fd || fd->advW == 0) {
        return 0;
    }
    if (fontSize == fd->origW) {
        if (monoWidth_)
            return std::max(fd->advW, monoWidth_);
        return fd->advW;
    }
    if (monoWidth_)
        return std::max(fd->advW, monoWidth_) * fontSize / fd->origW;
    return fd->advW * fontSize / fd->origW;
}

#define RGBA(r, g, b, a) (uint32_t(r) | (uint32_t(g) << 8) | (uint32_t(b) << 16) | (uint32_t(a) << 24))
void Font::setColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    altColor_[0] = RGBA(r, g, b, a);
}

void Font::setAltColor(int index, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    if (index > 0 && index < 16) {
        altColor_[index] = RGBA(r, g, b, a);
    }
}
#undef RGBA

const Font::FontData *Font::getOrMakeCache(uint32_t ch, int fontSize) {
    if (fontSize < 0) fontSize = fontSize_;
    uint64_t key = makeFontKey(ch, fontSize);
    auto ite = fontCache_.find(key);
    if (ite != fontCache_.end()) {
        return &ite->second;
    }

    FontData *fd = &fontCache_[key];
    if (!makeCache(fd, ch, fontSize)) {
        return nullptr;
    }
    return fd;
}

bool Font::updateTexture(int &rpidx, int16_t &rpx, int16_t &rpy, int w, int h, const uint8_t *data) {
    rpidx = rectpacker_->pack(w, h, rpx, rpy);
    if (rpidx < 0) {
        return false;
    }

    if (size_t(rpidx) >= textures_.size()) {
        textures_.resize(rpidx + 1, nullptr);
    }
    auto *tex = textures_[rpidx];
    if (tex == nullptr) {
        tex = renderImpl_.createTexture(RectPackWidthDefault, RectPackWidthDefault);
        if (!tex) { return false; }
        textures_[rpidx] = tex;
    }
    renderImpl_.updateTexture(tex, rpx, rpy, w, h, data);
    return true;
}

}

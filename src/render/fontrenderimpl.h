/*
 * Copyright (c) 2021 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#pragma once

#include <cstdint>

namespace render {

class FontRenderImpl {
public:
    virtual void *createTexture(int width, int height) = 0;
    virtual void destroyTexture(void *tex) = 0;
    virtual void updateTexture(void *tex, int x, int y, int w, int h, const uint8_t *data) = 0;
    virtual void renderBegin() = 0;
    virtual void render(void *tex, float x0, float y0, float x1, float y1, int u0, int v0, int u1, int v1, uint32_t color) = 0;
    virtual void renderEnd() = 0;
};

}

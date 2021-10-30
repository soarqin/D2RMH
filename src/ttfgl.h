/*
 * Copyright (c) 2021 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#pragma once

#include "ttf.h"

class Pipeline;
class PipelineTexture2D;
class Renderer;
class Texture;

class TTFRenderGL final: public TTFRenderImpl {
public:
    explicit TTFRenderGL(Renderer &renderer);
    explicit TTFRenderGL(Texture &tex);
    ~TTFRenderGL();
    Pipeline *pipeline();
    void *createTexture(int width, int height) override;
    void destroyTexture(void *tex) override;
    void updateTexture(void *tex, int x, int y, int w, int h, const uint8_t *data) override;
    void renderBegin() override;
    void render(void *tex,
                float x0, float y0, float x1, float y1, int u0, int v0, int u1, int v1,
                uint8_t r, uint8_t g, uint8_t b, uint8_t a) override;
    void renderEnd() override;

private:
    PipelineTexture2D *pipeline_;
    void *lastTex_ = nullptr;
};

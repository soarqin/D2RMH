/*
 * Copyright (c) 2021 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#include "ttfgl.h"

#include "renderer.h"

TTFRenderGL::TTFRenderGL(Renderer &renderer): pipeline_(new PipelineTexture2D(renderer)) {
}
TTFRenderGL::TTFRenderGL(Texture &tex): pipeline_(new PipelineTexture2D(tex)) {
}
TTFRenderGL::~TTFRenderGL() {
    delete pipeline_;
}
Pipeline *TTFRenderGL::pipeline() {
    return pipeline_;
}
void *TTFRenderGL::createTexture(int width, int height) {
    auto *tex = new Texture;
    tex->create(width, height);
    return tex;
}
void TTFRenderGL::destroyTexture(void *tex) {
    delete (Texture*)tex;
}
void TTFRenderGL::updateTexture(void *tex, int x, int y, int w, int h, const uint8_t *data) {
    ((Texture*)tex)->updateSubData(x, y, w, h, (const uint32_t*)data);
}
void TTFRenderGL::renderBegin() {
    pipeline_->reset();
    lastTex_ = nullptr;
}
void TTFRenderGL::render(void *tex,
                         float x0, float y0, float x1, float y1, int u0, int v0, int u1, int v1,
                         uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    auto *texture = (Texture*)tex;
    if (tex != lastTex_) {
        if (lastTex_) {
            pipeline_->setTexture(*texture);
            pipeline_->render();
            pipeline_->reset();
        }
        lastTex_ = tex;
    }
    float width = texture->width();
    float height = texture->height();
    pipeline_->pushQuad(x0, y0, x1, y1,
                        (float)u0 / width, (float)v0 / height,
                        (float)u1 / width, (float)v1 / height,
                        RGBA(r, g, b, a));
}
void TTFRenderGL::renderEnd() {
    if (lastTex_) {
        auto *texture = (Texture*)lastTex_;
        pipeline_->setTexture(*texture);
        pipeline_->render();
    }
}

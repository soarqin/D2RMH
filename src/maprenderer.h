/*
 * Copyright (c) 2021 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#pragma once

#include "renderer.h"
#include "ttfgl.h"
#include "d2rprocess.h"
#include "data.h"

#include "HandmadeMath.h"

#include "collisionmap.h"
#include "session.h"

#include "../common/jsonlng.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <vector>
#include <tuple>
#include <string>

class MapRenderer {
public:
    explicit MapRenderer(Renderer &renderer);
    void update();
    void render();

private:
    void updateWindowPos();
    void updatePlayerPos();
    void drawObjects();

private:
    Renderer &renderer_;
    PipelineTexture2D mapPipeline_;
    PipelineSquad2D framePipeline_;
    PipelineSquad2D dynamicPipeline_;
    PipelineSquad2D messagePipeline_;
    D2RProcess d2rProcess_;
    TTFRenderGL ttfgl_;
    TTF ttf_;
    Texture mapTex_;
    hmm_mat4 transform_ = {};
    int mapViewport_[4] = {};
    int msgViewport_[4] = {};

    bool enabled_ = false;
    Session session_;
    RECT d2rRect = {};
    uint32_t currLevelId_ = 0;
    CollisionMap *currMap_ = nullptr;
    uint32_t walkableColor_ = 0;
    JsonLng::LNG lng_ = JsonLng::LNG_enUS;
    uint16_t playerPosX_ = 0, playerPosY_ = 0;

    std::vector<std::tuple<float, float, std::wstring, float>> textStrings_;
    struct DynamicTextString {
        float x, y;
        const char *text;
        float offX;
    };
    std::vector<DynamicTextString> dynamicTextStrings_;
    std::vector<std::tuple<std::wstring_view, float, float, int, uint8_t>> textToDraw_, msgToDraw_;
    std::vector<std::tuple<float, float>> lines_;

    uint32_t objColors_[TypeMax] = {};
};

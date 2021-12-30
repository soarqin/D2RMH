/*
 * Copyright (c) 2021 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#pragma once

#include "render/renderer.h"
#include "render/font.h"
#include "render/ttfgl.h"
#include "render/HandmadeMath.h"
#include "d2r/processmanager.h"
#include "data/gamedata.h"
#include "plugin/plugin.h"

#include "collisionmap.h"
#include "pipehost.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <vector>
#include <map>
#include <memory>
#include <tuple>
#include <string>
#include <chrono>

namespace ui {

enum LNG {
    LNG_enUS,
    LNG_zhTW,
    LNG_deDE,
    LNG_esES,
    LNG_frFR,
    LNG_itIT,
    LNG_koKR,
    LNG_plPL,
    LNG_esMX,
    LNG_jaJP,
    LNG_ptBR,
    LNG_ruRU,
    LNG_zhCN,
    LNG_MAX,
};

class MapRenderer final {
    struct MapRenderSession {
        void *hwnd = nullptr;
        render::Texture mapTex;
        uint32_t currSeed = uint32_t(-1);
        uint8_t currDifficulty = uint8_t(-1);
        uint32_t currLevelId = 0;
        std::map<uint32_t, std::unique_ptr<d2mapapi::CollisionMap>> maps;
        std::vector<uint8_t> mapData;
        int x0 = 0, y0 = 0, x1 = 0, y1 = 0, cx = 0, cy = 0;
        int lastPosX = -1, lastPosY = -1;
        const d2mapapi::CollisionMap *currMap = nullptr;
        std::vector<std::tuple<float, float, std::wstring, float>> textStrings;
        std::vector<std::tuple<float, float>> lines;
        std::vector<std::vector<std::pair<int, int>>> path;
        std::chrono::steady_clock::time_point mapStartTime;
    };
public:
    MapRenderer(render::Renderer &renderer, d2mapapi::PipedChildProcess &);
    inline render::Renderer &getRenderer() { return renderer_; }
    void update();
    void render();
    void reloadConfig();

    void forceFlush() {
        forceFlush_ = true;
    }

    plugin::PluginTextList &getPluginText(const std::string &key);
    void removePluginText(const std::string &key);

private:
    void updateWindowPos();
    void updatePlayerPos();
    void drawObjects();
    void updatePanelText();
    void loadFromCfg();

    d2mapapi::CollisionMap *getMap(uint32_t levelId);

private:
    render::Renderer &renderer_;
    render::PipelineTexture2D mapPipeline_;
    render::PipelineSquad2D framePipeline_;
    render::PipelineSquad2D dynamicPipeline_;
    render::PipelineSquad2D messagePipeline_;
    d2r::ProcessManager d2rProcess_;
    render::TTFRenderGL ttfgl_;
    std::unique_ptr<render::Font> ttf_;
    hmm_mat4 transform_ = {};
    int mapViewport_[4] = {};
    int scissor_[4] = {};

    bool enabled_ = false;
    RECT d2rRect = {};
    LNG lng_ = LNG_enUS;

    std::map<void*, std::unique_ptr<MapRenderSession>> sessions_;
    MapRenderSession *currSession_ = nullptr;

    struct DynamicTextString {
        float x, y;
        const char *text;
        float offX;
    };
    std::vector<DynamicTextString> dynamicTextStrings_;
    std::vector<std::tuple<std::wstring_view, float, float, int, uint8_t>> textToDraw_, msgToDraw_;

    uint32_t objColors_[data::TypeMax] = {};

    std::chrono::steady_clock::time_point nextPanelUpdateTime_;
    std::vector<std::wstring> panelText_;

    d2mapapi::PipedChildProcess &childProcess_;

    bool forceFlush_ = false;
    plugin::Plugin plugin_;
    std::map<std::string, plugin::PluginTextList> pluginTextMap_;
};

}

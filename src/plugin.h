/*
 * Copyright (c) 2021 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#pragma once

#include <chrono>
#include <vector>
#include <string>

struct PluginCtx;
class D2RProcess;
class MapRenderer;

struct PluginText {
    std::wstring text;
    std::chrono::steady_clock::time_point timeout;
    int fontSize;
};
struct PluginTextList {
    float x = .2f, y = .78f;
    int align = 0;
    int valign = 1;
    std::vector<PluginText> textList;
    void add(const char *text, uint32_t timeout, int fontSize);
    void clear() { textList.clear(); }
};

class Plugin final {
public:
    Plugin(D2RProcess *process, MapRenderer *renderer);
    ~Plugin();
    void load();
    void run();
    void onEnterGame();

private:
    void addCFunctions();

private:
    PluginCtx *ctx_;
    D2RProcess *d2rProcess_;
    MapRenderer *mapRenderer_;
};

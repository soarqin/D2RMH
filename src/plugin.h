/*
 * Copyright (c) 2021 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#pragma once

struct PluginCtx;
class D2RProcess;
class MapRenderer;

class Plugin final {
public:
    Plugin(D2RProcess *process, MapRenderer *renderer);
    ~Plugin();
    void load();
    void run();

private:
    void addCFunctions();

private:
    PluginCtx *ctx_;
    D2RProcess *d2rProcess_;
    MapRenderer *mapRenderer_;
};

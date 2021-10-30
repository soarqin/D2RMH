/*
 * Copyright (c) 2021 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#pragma once

#include <functional>

struct WindowCtx;

class Window final {
    friend class Renderer;
public:
    enum {
        TRAYMENU_DISABLED = 1,
        TRAYMENU_CHECKED = 2,
        TRAYMENU_SUBMENU = 4,
    };

    Window(int x, int y, int width, int height);
    ~Window();

    void enableTrayMenu(bool enable, const wchar_t *icon, const wchar_t *tip = nullptr, const wchar_t *info = nullptr, const wchar_t *infoTitle = nullptr);
    int addTrayMenuItem(const wchar_t *name, int parent, unsigned flags, const std::function<void()> &cb);

    [[nodiscard]] bool run() const;
    [[nodiscard]] bool running() const;
    void quit();

    [[nodiscard]] inline WindowCtx *ctx() { return ctx_; }
    void getDimension(int &w, int &h);
    void move(int x, int y, int w, int h);

private:
    void setSizeCallback(const std::function<void(int, int)> &cb);
    void *hwnd();

private:
    WindowCtx *ctx_;
};

/*
 * Copyright (c) 2022 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#pragma once

#include <map>
#include <functional>
#include <cstdint>

namespace ui {

class ConfigDlg {
public:
    void setupConfigItems(void *hwnd);
    bool run();

    inline void call(uint32_t id, uintptr_t lParam) const {
        auto ite = functions_.find(id);
        if (ite != functions_.end()) {
            ite->second(lParam);
        }
    }

private:
    std::map<uint32_t, std::function<void(uintptr_t)>> functions_;
};

}

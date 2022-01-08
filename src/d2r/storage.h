/*
 * Copyright (c) 2021 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#pragma once

#include <vector>
#include <cstdint>

namespace d2r {

struct StorageCtx;

class Storage final {
public:
    Storage() noexcept;
    ~Storage();
    bool init();
    bool readFile(const char *filename, std::vector<uint8_t> &data);

private:
    StorageCtx *ctx_;
};

extern Storage storage;

}

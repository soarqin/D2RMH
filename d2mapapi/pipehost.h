/*
 * Copyright (c) 2021 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#pragma once

#include "collisionmap.h"

#include <string>
#include <cstdint>

namespace d2mapapi {

class PipedChildProcess final {
public:
    ~PipedChildProcess();
    bool start(const wchar_t *filename, wchar_t *parameters);

    bool writePipe(const void *data, size_t size);
    bool readPipe(void *data, size_t size);

    std::string queryMapRaw(uint32_t seed, uint8_t difficulty, uint32_t levelId);
    CollisionMap *queryMap(uint32_t seed, uint8_t difficulty, uint32_t levelId);

private:
    void *childStdinRd = nullptr;
    void *childStdinWr = nullptr;
    void *childStdoutRd = nullptr;
    void *childStdoutWr = nullptr;
    void *process = nullptr;
};

}

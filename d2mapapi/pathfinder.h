/*
 * Copyright (c) 2021 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#pragma once

#include <vector>
#include <utility>
#include <cstdint>

namespace d2mapapi {

std::vector<std::pair<int, int>> pathFindBFS(int startX, int startY, int targetX, int targetY,
                                             const uint8_t *mapData, int mapWidth, int mapHeight, bool merge = false);

}

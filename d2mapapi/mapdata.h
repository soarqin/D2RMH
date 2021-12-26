/*
 * Copyright (c) 2021 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#pragma once

#include "collisionmap.h"
#include "d2structs.h"

namespace d2mapapi {

class MapData: public CollisionMap {
public:
    MapData(Act *act, unsigned int areaId, bool generatePathData = false);
    void genPathData(const int16_t *map);
};

}

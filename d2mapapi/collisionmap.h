#pragma once

#include "d2structs.h"

#include <map>
#include <vector>

namespace d2mapapi {

struct Point {
    int x = 0;
    int y = 0;
};

struct Size {
    int width = 0;
    int height = 0;
};

struct Exit {
    std::vector<Point> offsets;
    bool isPortal = false;
};

class CollisionMap {
public:
    CollisionMap(Act *act, unsigned int areaId);

    bool built = false;
    unsigned int id;

    Point offset;
    Size size;

    /* Collission maps are cropped in rect [cropX0, cropY0] to [cropX1, cropY1] relative to [offset.x, offset.y] */
    int cropX0 = -1, cropY0 = -1, cropX1 = -1, cropY1 = -1;

    /* Collision maps are encoded using a simple run length encoding to save memory space
     *  -1 ends of a row
     * Given this small map:
     *  [1,5,1,-1,
     *   2,3,2,-1,
     *   1,5,1,-1]
     * It would generate the following map where `X` is collision and `.` is open space
     *   X.....X
     *   XX...XX
     *   X.....X
     */
    std::vector<int16_t> mapData;
    std::map<uint32_t, Exit> exits;
    std::map<uint32_t, std::vector<Point>> npcs;
    std::map<uint32_t, std::vector<Point>> objects;
};

}

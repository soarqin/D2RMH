#pragma once

#include "d2structs.h"

#include <map>
#include <vector>

struct Point {
    int x = 0;
    int y = 0;
};

struct AdjacentLevel {
    std::vector<Point> exits;
    Point levelOrigin;
    int width = 0;
    int height = 0;
};

class CollisionMap {
private:
    unsigned int areaId_;
    Act *act_;

public:
    CollisionMap(Act *act, unsigned int areaId);
    bool build();

    Point levelOrigin; // level top-left
    int cropX = -1, cropY = -1, cropX2 = -1, cropY2 = -1;
    std::vector<std::vector<uint16_t>> map;
    std::map<uint32_t, AdjacentLevel> adjacentLevels;
    std::map<uint32_t, std::vector<Point>> npcs;
    std::map<uint32_t, std::vector<Point>> objects;
};

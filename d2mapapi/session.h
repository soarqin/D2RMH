#pragma once

#include "mapdata.h"

#include <map>
#include <memory>

namespace d2mapapi {

class Session {
public:
    ~Session();

    bool update(unsigned int seed, unsigned char difficulty);

    const CollisionMap *getMap(unsigned int areaid, bool generatePathData = false);

private:
    void unloadAll();

private:
    std::map<int, std::unique_ptr<MapData>> maps_;
    unsigned int seed_ = 0;
    unsigned char difficulty_ = 0;
    Act *acts_[5] = {};
};

}

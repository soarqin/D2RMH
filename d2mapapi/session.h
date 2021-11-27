#pragma once

#include "collisionmap.h"

#include <map>
#include <memory>

class Session {
public:
    ~Session();

    bool update(unsigned int seed, unsigned char difficulty);

    const CollisionMap *getMap(unsigned int areaid);

private:
    void unloadAll();

private:
    std::map<int, std::unique_ptr<CollisionMap>> maps_;
    unsigned int seed_ = 0;
    unsigned char difficulty_ = 0;
    Act *acts_[5] = {};
};

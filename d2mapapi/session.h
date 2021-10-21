#pragma once

#include "collisionmap.h"

#include <map>
#include <memory>

class Session {
public:
    Session(unsigned int seed, unsigned int difficulty);
    ~Session();

    CollisionMap *getMap(unsigned int areaid);
private:
    std::map<int, std::unique_ptr<CollisionMap>> maps_;
    unsigned int seed_;
    unsigned int difficulty_;
    Act *acts_[5];
};

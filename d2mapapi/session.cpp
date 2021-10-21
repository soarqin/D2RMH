#include "session.h"

#include "d2ptrs.h"

#include <iostream>

static const unsigned int ActLevels[] = {1, 40, 75, 103, 109, 137};

namespace Helpers {
unsigned int getAct(unsigned int areaid) {
    for (int i = 1; i < 5; i++) {
        if (areaid < ActLevels[i]) {
            return i - 1;
        }
    }

    return 4;
}
}

Session::Session(unsigned seed, unsigned difficulty) {
    seed_ = seed;
    difficulty = difficulty;

    for (int i = 0; i < 5; i++) {
        acts_[i] = D2COMMON_LoadAct(i,
                                    seed,
                                    1 /*TRUE*/,
                                    0 /*FALSE*/,
                                    difficulty,
                                    0,
                                    ActLevels[i],
                                    D2CLIENT_LoadAct_1,
                                    D2CLIENT_LoadAct_2);
    }
}

Session::~Session() {
    for (int i = 0; i < 5; i++) {
        D2COMMON_UnloadAct(acts_[i]);
    }
}

CollisionMap *Session::getMap(unsigned int areaid) {
    auto ite = maps_.find(areaid);
    if (ite == maps_.end()) {
        auto map = std::make_unique<CollisionMap>(acts_[Helpers::getAct(areaid)], areaid);
        if (!map->build()) { return nullptr; }
        auto &output = maps_[areaid];
        output = std::move(map);
        return output.get();
    }

    return ite->second.get();
}

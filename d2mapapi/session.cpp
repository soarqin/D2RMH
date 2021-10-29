#include "session.h"

#include "d2ptrs.h"

#include <iostream>
#include <cstring>

static const unsigned int ActLevels[] = {1, 40, 75, 103, 109, 137};

namespace Helpers {
unsigned int getAct(unsigned int areaid) {
    for (int i = 1; i < 5; i++) {
        if (areaid < ActLevels[i]) {
            return i - 1;
        }
    }

    return -1;
}
}

Session::Session(unsigned seed, unsigned difficulty) {
    seed_ = seed;
    difficulty_ = std::min(difficulty, 2u);
    memset(acts_, 0, sizeof(acts_));
}

Session::~Session() {
    for (int i = 0; i < 5; i++) {
        if (acts_[i]) {
            D2COMMON_UnloadAct(acts_[i]);
        }
    }
}

CollisionMap *Session::getMap(unsigned int areaid) {
    auto ite = maps_.find(areaid);
    if (ite == maps_.end()) {
        auto actId = Helpers::getAct(areaid);
        if (actId < 0) { return nullptr; }
        if (!acts_[actId]) {
            acts_[actId] = D2COMMON_LoadAct(actId,
                                        seed_,
                                        1 /*TRUE*/,
                                        0 /*FALSE*/,
                                        difficulty_,
                                        0,
                                        ActLevels[actId],
                                        D2CLIENT_LoadAct_1,
                                        D2CLIENT_LoadAct_2);
        }
        auto map = std::make_unique<CollisionMap>(acts_[actId], areaid);
        if (!map->build()) { return nullptr; }
        auto &output = maps_[areaid];
        output = std::move(map);
        return output.get();
    }

    return ite->second.get();
}

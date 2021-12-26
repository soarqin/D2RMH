#include "session.h"

#include "d2ptrs.h"

namespace d2mapapi {

static const unsigned int ActLevels[] = {1, 40, 75, 103, 109, 137};

namespace Helpers {
int getAct(unsigned int areaid) {
    for (int i = 0; i < 6; i++) {
        if (areaid < ActLevels[i]) {
            return i - 1;
        }
    }

    return -1;
}
}

Session::~Session() {
    unloadAll();
}

bool Session::update(unsigned int seed, unsigned char difficulty) {
    if (difficulty > 2) difficulty = 2;
    if (seed_ == seed && difficulty == difficulty_) { return false; }
    seed_ = seed;
    difficulty_ = difficulty;
    unloadAll();
    return true;
}

const CollisionMap *Session::getMap(unsigned int areaid, bool generatePathData) {
    auto ite = maps_.find(areaid);
    if (ite != maps_.end()) {
        if (generatePathData && ite->second->pathData.empty()) {
            auto w = ite->second->size.width;
            auto h = ite->second->size.height;
            std::vector<int16_t> map(w, h);
            if (ite->second->extractCellData<int16_t>(map.data(), w, h, 0, 0, 1, 0)) {
                ite->second->genPathData(map.data());
            }
        }
        return ite->second.get();
    }
    auto actId = Helpers::getAct(areaid);
    if (actId < 0) {
        return nullptr;
    }
    if (!acts_[actId]) {
        acts_[actId] = D2COMMON_LoadAct(actId, seed_, 1 /*TRUE*/, nullptr, difficulty_, nullptr,
                                        ActLevels[actId], D2CLIENT_LoadAct_1, D2CLIENT_LoadAct_2);
    }
    auto map = std::make_unique<MapData>(acts_[actId], areaid, generatePathData);
    if (!map->built) { return nullptr; }
    auto &output = maps_[areaid];
    output = std::move(map);
    return output.get();
}

void Session::unloadAll() {
    maps_.clear();
    for (auto *&act: acts_) {
        if (act) {
            D2COMMON_UnloadAct(act);
            act = nullptr;
        }
    }
}

}

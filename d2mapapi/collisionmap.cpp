#include "collisionmap.h"

#include <json.hpp>

#include <sstream>

namespace d2mapapi {

CollisionMap::CollisionMap(std::string_view str) {
    decode(str);
}

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Point, x, y)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Size, width, height)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Rect, x0, y0, x1, y1)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Exit, offsets, isPortal)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CollisionMap, id, offset, size, crop, mapData, exits, npcs, objects)

template<typename T>
void to_json(nlohmann::json &j, const std::map<uint32_t, T> &objs) {
    for (auto &p: objs) {
        j[std::to_string(p.first)] = p.second;
    }
}

template<typename T>
void from_json(const nlohmann::json &j, std::map<uint32_t, T> &objs) {
    for (auto &[key, value]: j.items()) {
        nlohmann::adl_serializer<T>::from_json(value, objs[uint32_t(strtoul(key.c_str(), nullptr, 0))]);
    }
}

std::string CollisionMap::encode(int indentation) const {
    if (!built) { return ""; }
    nlohmann::json j = *this;
    return j.dump(indentation > 0 ? indentation : -1);
}

void CollisionMap::decode(std::string_view str) {
    auto j = nlohmann::json::parse(str, nullptr, false);
    if (j.empty()) {
        built = false;
        errorString = "Wrong input.";
        return;
    }
    auto ite = j.find("error");
    if (ite != j.end()) {
        built = false;
        errorString = ite->get<std::string>();
        return;
    }

    try {
        from_json(j, *this);
    } catch (const std::exception &e) {
        built = false;
        errorString = e.what();
        return;
    }
    built = true;
    errorString.clear();
}

}

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

std::string CollisionMap::encode() const {
    if (!built) { return ""; }
    nlohmann::json j = *this;
    std::ostringstream oss;
    oss << j;
    return oss.str();
}

void CollisionMap::decode(std::string_view str) {
    auto j = nlohmann::json::parse(str, nullptr, false);
    if (j.empty()) { built = false; return; }
    from_json(j, *this);
    built = true;
}

}

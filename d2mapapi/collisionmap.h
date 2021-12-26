#pragma once

#include <map>
#include <vector>
#include <string>
#include <string_view>

namespace d2mapapi {

struct Point {
    int x = 0;
    int y = 0;
};

struct Size {
    int width = 0;
    int height = 0;
};

struct Rect {
    int x0, y0, x1, y1;
};

struct Exit {
    std::vector<Point> offsets;
    bool isPortal = false;
};

class CollisionMap {
public:
    explicit CollisionMap(unsigned int areaId): id(areaId) {}
    explicit CollisionMap(std::string_view str);
    virtual ~CollisionMap() = default;

    [[nodiscard]] std::string encode(bool encPathData = false, int indentation = 0) const;
    void decode(std::string_view str);

    template<typename T>
    inline bool extractCellData(T *output, int width, int height, int originX, int originY, T nonwalkableVal, T walkableVal, T edgeVal = T()) const {
        auto w = std::max(crop.x1 - crop.x0, 0);
        auto h = std::max(crop.y1 - crop.y0, 0);
        if (originX + w > width || originY + h > height) {
            return false;
        }
        int x = 0, y = 0;
        int index = originY * width + originX;
        bool walkable = false;
        bool hasEdge = edgeVal != T();
        for (int v: mapData) {
            if (v < 0) {
                if (++y >= h) { break; }
                x = 0;
                index = (originY + y) * width + originX;
                walkable = false;
                continue;
            }
            v = std::min(int(v), w - x);
            if (!walkable) {
                std::fill_n(output + index, v, nonwalkableVal);
                if (hasEdge && y > 0) {
                    T *ptr = output + index - width;
                    for (int z = v; z; z--, ptr++) {
                        if (*ptr == walkableVal) {
                            *ptr = edgeVal;
                        }
                    }
                }
                index += v;
            } else {
                if (hasEdge && y + 1 == crop.y1 && crop.y1 < height) {
                    std::fill_n(output + index, v, edgeVal);
                    index += v;
                } else {
                    int z = v;
                    if (hasEdge) {
                        *(output + index) = x > 0 ? edgeVal : walkableVal;
                        index++;
                        z -= 2;
                    }
                    if (z >= 0) {
                        if (hasEdge && y > 0) {
                            for (; z; z--) {
                                *(output + index) = *(output + index - width) == nonwalkableVal ? edgeVal : walkableVal;
                                index++;
                            }
                        } else {
                            std::fill_n(output + index, z, walkableVal);
                            index += z;
                        }
                        if (hasEdge) {
                            *(output + index) = x + v < w ? edgeVal : walkableVal;
                            index++;
                        }
                    }
                }
            }
            x += v;
            walkable = !walkable;
        }
        return true;
    }

    bool built = false;
    std::string errorString;

    unsigned int id = 0;

    Point offset = {0, 0};
    Size size = {0, 0};

    /* Collission maps are cropped in rect [crop.x0, crop.y0] to [crop.x1, crop.y1] relative to [offset.x, offset.y] */
    Rect crop = {-1, -1, -1, -1};

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

    std::vector<uint8_t> path;
    std::vector<uint8_t> pathData;
};

}

/*
 * Copyright (c) 2021 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#include "pathfinder.h"

#include <queue>
#include <cmath>

namespace d2mapapi {

std::vector<std::pair<int, int>> pathFindBFS(int startX, int startY, int targetX, int targetY,
                const uint8_t *mapData, int mapWidth, int mapHeight, bool merge) {
    const int n = mapWidth * mapHeight;
    if (startX >= mapWidth || startY >= mapHeight || targetX >= mapWidth || targetY >= mapHeight) {
        return {};
    }
    const int startPos = startX + startY * mapWidth, targetPos = targetX + targetY * mapWidth;
    std::vector<std::pair<int, int>> p(n, {-1, -1});
    p[startPos].second = 0;
    std::queue<int> q;
    q.push(startPos);
    int offset[] = {0, -1, +1, 0, -mapWidth, 0, 0, 0, +mapWidth};
    while (!q.empty()) {
        int u = q.front();
        q.pop();
        for (auto e: {1, 2, 4, 8}) {
            if (!(mapData[u] & e)) { continue; }
            int off = offset[e];
            int v = u + off;
            if ((off == 1 && (v % mapWidth == 0)) || (off == -1 && (u % mapWidth == 0)))
                continue;
            if (0 <= v && v < n && p[v].second == -1 && mapData[v]) {
                p[v] = { u, p[u].second + 1 };
                if (v == targetPos)
                    goto end;
                q.push(v);
            }
        }
    }
end:
    if (p[targetPos].second == -1) {
        return {};
    }
    std::vector<std::pair<int, int>> result;
    result.reserve(p[targetPos].second);
    if (merge) {
        int curr = targetPos;
        if (curr == -1) { return {}; }
        int prevX = curr % mapWidth, prevY = curr / mapWidth;
        int lastX = prevX, lastY = prevY;
        result.emplace_back(lastX, lastY);
        curr = p[curr].first;
        while (curr != -1) {
            int currX = curr % mapWidth, currY = curr / mapWidth;
            bool blocked = false;
            /* Check if line is blocked here */
            if (currX == prevX) {
                int delta = prevY < currY ? 1 : -1;
                int indexDelta = delta * mapWidth;
                int checkBit = delta == 1 ? 8 : 4;
                int index = prevY * mapWidth + prevX;
                for (int y = prevY; y != currY; y += delta, index += indexDelta) {
                    if (!(mapData[index] & checkBit)) {
                        blocked = true;
                        break;
                    }
                }
            } else if (currY == prevY) {
                int delta = prevX < currX ? 1 : -1;
                int checkBit = delta == 1 ? 2 : 1;
                int index = prevY * mapWidth + prevX;
                for (int x = prevX; x != currX; x += delta, index += delta) {
                    if (!(mapData[index] & checkBit)) {
                        blocked = true;
                        break;
                    }
                }
            } else {
                int dx = std::abs(currX - prevX);
                int dy = std::abs(currY - prevY);
                if (dx < dy) {
                    int delta = prevY < currY ? 1 : -1;
                    int indexDelta = delta * mapWidth;
                    int delta2 = prevX < currX ? 1 : -1;
                    int checkBit = delta == 1 ? 8 : 4;
                    int checkBit2 = delta2 == 1 ? 2 : 1;
                    int index = prevY * mapWidth + prevX;
                    int total = dy / 2;
                    for (int y = prevY; y != currY; y += delta, index += indexDelta) {
                        total += dx;
                        if (total >= dy) {
                            total -= dy;
                            auto val0 = dx - total;
                            if (val0 >= total) {
                                if (!(mapData[index] & checkBit) || !(mapData[index + indexDelta] & checkBit2)) {
                                    blocked = true;
                                    break;
                                }
                            } else {
                                if (!(mapData[index] & checkBit2) || !(mapData[index + delta2] & checkBit)) {
                                    blocked = true;
                                    break;
                                }
                            }
                            index += delta2;
                        } else {
                            if (!(mapData[index] & checkBit)) {
                                blocked = true;
                                break;
                            }
                        }
                    }
                } else {
                    int delta = prevX < currX ? 1 : -1;
                    int delta2 = prevY < currY ? 1 : -1;
                    int indexDelta2 = delta2 * mapWidth;
                    int checkBit = delta == 1 ? 2 : 1;
                    int checkBit2 = delta2 == 1 ? 8 : 4;
                    int index = prevY * mapWidth + prevX;
                    int total = dx / 2;
                    for (int x = prevX; x != currX; x += delta, index += delta) {
                        total += dy;
                        if (total >= dx) {
                            total -= dx;
                            auto val0 = dy - total;
                            if (val0 >= total) {
                                if (!(mapData[index] & checkBit) || !(mapData[index + delta] & checkBit2)) {
                                    blocked = true;
                                    break;
                                }
                            } else {
                                if (!(mapData[index] & checkBit2) || !(mapData[index + indexDelta2] & checkBit)) {
                                    blocked = true;
                                    break;
                                }
                            }
                            index += indexDelta2;
                        } else {
                            if (!(mapData[index] & checkBit)) {
                                blocked = true;
                                break;
                            }
                        }
                    }
                }
            }
            if (blocked) {
                result.emplace_back(lastX, lastY);
                prevX = lastX; prevY = lastY;
                continue;
            }
            lastX = currX; lastY = currY;
            curr = p[curr].first;
        }
        result.emplace_back(lastX, lastY);
        return result;
    }
    int curr = targetPos;
    while (curr != -1) {
        int currX = curr % mapWidth, currY = curr / mapWidth;
        result.emplace_back(currX, currY);
        curr = p[curr].first;
    }
    return std::move(result);
}

}

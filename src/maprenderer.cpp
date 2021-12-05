/*
 * Copyright (c) 2021 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#include "maprenderer.h"

#include "window.h"
#include "util.h"
#include "cfg.h"

static LNG lngFromString(const std::string &language) {
#define CHECK_LNG(n) if (language == #n) { return LNG_##n; }
    CHECK_LNG(enUS)
    CHECK_LNG(zhTW)
    CHECK_LNG(deDE)
    CHECK_LNG(esES)
    CHECK_LNG(frFR)
    CHECK_LNG(itIT)
    CHECK_LNG(koKR)
    CHECK_LNG(plPL)
    CHECK_LNG(esMX)
    CHECK_LNG(jaJP)
    CHECK_LNG(ptBR)
    CHECK_LNG(ruRU)
    CHECK_LNG(zhCN)
    return LNG_enUS;
#undef CHECK_LNG
}

MapRenderer::MapRenderer(Renderer &renderer, d2mapapi::PipedChildProcess &childProcess) :
    renderer_(renderer),
    mapPipeline_(renderer),
    framePipeline_(renderer),
    dynamicPipeline_(renderer),
    messagePipeline_(renderer),
    ttfgl_(renderer),
    childProcess_(childProcess),
    plugin_(&d2rProcess_) {
    d2rProcess_.setWindowPosCallback([this](int left, int top, int right, int bottom) {
        d2rRect = {left, top, right, bottom};
        updateWindowPos();
    });
    d2rProcess_.setProcessCloseCallback([this](void *hwnd) {
        auto ite = sessions_.find(hwnd);
        if (ite == sessions_.end()) { return; }
        if (currHWND_ == ite->first) {
            currHWND_ = nullptr;
            currSession_ = nullptr;
        }
        sessions_.erase(ite);
    });

    loadFromCfg();
}

void MapRenderer::update() {
    d2rProcess_.updateData();
    if (!d2rProcess_.available()) {
        enabled_ = false;
        return;
    }
    plugin_.run();
    const auto *currPlayer = d2rProcess_.currPlayer();
    if (!currPlayer || !currPlayer->seed || !currPlayer->levelId) {
        enabled_ = false;
        return;
    }
    if (d2rProcess_.panelEnabled() & cfg->panelMask) {
        enabled_ = false;
    } else {
        switch (cfg->show) {
        case 0:
            enabled_ = !d2rProcess_.mapEnabled();
            break;
        case 1:
            enabled_ = d2rProcess_.mapEnabled();
            break;
        default:
            enabled_ = true;
            break;
        }
    }
    auto *hwnd = d2rProcess_.hwnd();
    bool changed = false;
    bool switched = false;
    if (currHWND_ != hwnd) {
        auto &session = sessions_[d2rProcess_.hwnd()];
        if (!session) {
            session = std::make_unique<SessionInfo>();
            changed = true;
        }
        currHWND_ = hwnd;
        currSession_ = session.get();
        nextPanelUpdateTime_ = getCurrTime();
        switched = true;
    }
    if (currSession_->currSeed != currPlayer->seed || currSession_->currDifficulty != currPlayer->difficulty) {
        currSession_->maps.clear();
        currSession_->currMap = nullptr;
        currSession_->currSeed = currPlayer->seed;
        currSession_->currDifficulty = currPlayer->difficulty;
        currSession_->mapStartTime = getCurrTime();
        changed = true;
    }
    if (!enabled_) {
        return;
    }
    if (uint32_t levelId = currPlayer->levelId; levelId != currSession_->currLevelId) {
        currSession_->currLevelId = levelId;
        changed = true;
    }
    if (changed) {
        currSession_->textStrings.clear();
        currSession_->lines.clear();
        auto *currMap = getMap(currSession_->currLevelId);
        currSession_->currMap = currMap;
        if (!currMap) {
            enabled_ = false;
            return;
        }
        auto originX = currMap->offset.x, originY = currMap->offset.y;
        std::map<uint32_t, const d2mapapi::CollisionMap*> knownMaps;
        int x0 = currMap->crop.x0, y0 = currMap->crop.y0,
            x1 = currMap->crop.x1, y1 = currMap->crop.y1;
        int totalX0 = originX + x0, totalY0 = originY + y0, totalX1 = originX + x1, totalY1 = originY + y1;
        auto cx = currMap->offset.x + (x1 + x0) / 2;
        auto cy = currMap->offset.y + (y1 + y0) / 2;
        if (cfg->neighbourMapBounds != 0) {
            auto bounds = cfg->neighbourMapBounds > 0 ? cfg->neighbourMapBounds : 2048;
            auto steps = cfg->neighbourMapBounds < 0 ? -cfg->neighbourMapBounds : 32;
            auto mx0 = std::max(0, cx - bounds);
            auto my0 = std::max(0, cy - bounds);
            auto mx1 = cx + bounds;
            auto my1 = cy + bounds;
            std::vector<std::pair<const d2mapapi::CollisionMap*, int>> mapsToProcess = {{currMap, 0}};
            while (!mapsToProcess.empty()) {
                const auto [map, step] = mapsToProcess.back();
                mapsToProcess.pop_back();
                knownMaps[map->id] = map;
                if (step >= steps) { continue; }
                for (auto &p: map->exits) {
                    if (p.second.isPortal) { continue; }
                    if (knownMaps.find(p.first) != knownMaps.end()) { continue; }
                    auto *nearMap = getMap(p.first);
                    if (nearMap) {
                        mapsToProcess.emplace_back(nearMap, step + 1);
                    }
                }
            }
            knownMaps.erase(currMap->id);
            for (auto ite = knownMaps.begin(); ite != knownMaps.end();) {
                auto tx0 = ite->second->offset.x + ite->second->crop.x0;
                auto ty0 = ite->second->offset.y + ite->second->crop.y0;
                auto tx1 = ite->second->offset.x + ite->second->crop.x1;
                auto ty1 = ite->second->offset.y + ite->second->crop.y1;
                if (tx0 < mx0 || ty0 < my0 || tx1 > mx1 || ty1 > my1) {
                    ite = knownMaps.erase(ite);
                    continue;
                } else {
                    if (tx0 < totalX0) { totalX0 = tx0; }
                    if (ty0 < totalY0) { totalY0 = ty0; }
                    if (tx1 > totalX1) { totalX1 = tx1; }
                    if (ty1 > totalY1) { totalY1 = ty1; }
                    ++ite;
                }
            }
        }
        currSession_->x0 = totalX0;
        currSession_->y0 = totalY0;
        currSession_->x1 = totalX1;
        currSession_->y1 = totalY1;
        currSession_->cx = cx;
        currSession_->cy = cy;
        int width = totalX1 - totalX0;
        int height = totalY1 - totalY0;
        auto *pixels = new uint32_t[width * height];
        memset(pixels, 0, sizeof(uint32_t) * width * height);
        auto &mapTex = currSession_->mapTex;
        auto totalWidth = currMap->size.width;
        auto decodeMapFunc = [this, totalX0, totalY0, width, pixels](const d2mapapi::CollisionMap *currMap) {
            auto offX = currMap->offset.x - totalX0;
            auto offY = currMap->offset.y - totalY0;
            auto edgeColor = cfg->edgeColor;
            bool hasEdge = (edgeColor & 0xFFFFFFu) != 0;
            auto x0 = currMap->crop.x0, y0 = currMap->crop.y0, y1 = currMap->crop.y1,
                 mw = currMap->size.width, mh = currMap->size.height;
            int y = y0;
            int x = x0;
            bool isWalkable = false;
            for (auto v: currMap->mapData) {
                if (v == -1) {
                    ++y;
                    x = x0;
                    isWalkable = false;
                    continue;
                }
                if (!isWalkable) {
                    /* draw bottom edge */
                    if (hasEdge && y > 0) {
                        auto *ptr = pixels + offX + x + (offY + y) * width;
                        for (int z = v; z; z--, ptr++) {
                            if (*(ptr - width) != 0) {
                                *(ptr - width) = edgeColor;
                            }
                        }
                    }
                } else {
                    auto *ptr = pixels + offX + x + (offY + y) * width;
                    int z = v;
                    if (hasEdge && y + 1 == y1 && y1 < mh) {
                        /* bottom of cropped area, but not whole area */
                        for (; z; z--) {
                            *ptr++ = edgeColor;
                        }
                    } else {
                        /* draw left edge */
                        if (hasEdge && x > 0) {
                            *ptr++ = edgeColor;
                            z -= 2;
                        }
                        /* if only 1 dot block, skip this */
                        if (z > 0) {
                            bool drawEdge = hasEdge && y > 0;
                            for (; z; z--) {
                                /* check if this is top edge */
                                *ptr++ = drawEdge && *(ptr - width) == 0 ? edgeColor : walkableColor_;
                            }
                            /* draw right edge */
                            if (hasEdge && x < mw) {
                                *ptr = edgeColor;
                            }
                        }
                    }
                }
                isWalkable = !isWalkable;
                x += v;
            }
        };
        decodeMapFunc(currMap);
        for (auto &p: knownMaps) {
            decodeMapFunc(p.second);
        }
        mapTex.setData(width, height, pixels);
        delete[] pixels;

        const std::set<int> *guides;
        {
            auto gdite = gamedata->guides.find(currSession_->currLevelId);
            if (gdite != gamedata->guides.end()) {
                guides = &gdite->second;
            } else {
                guides = nullptr;
            }
        }

        PipelineSquad2D squadPip(mapTex);
        squadPip.setOrtho(0, float(mapTex.width()), 0, float(mapTex.height()));
        auto widthf = float(x1 - x0) * .5f, heightf = float(y1 - y0) * .5f;
        auto realTombLevelId = d2rProcess_.realTombLevelId();
        auto superUniqueTombLevelId = d2rProcess_.superUniqueTombLevelId();
        for (auto &p: currMap->exits) {
            if (p.first >= gamedata->levels.size()) { continue; }
            for (auto &e: p.second.offsets) {
                auto px = float(e.x - totalX0);
                auto py = float(e.y - totalY0);
                const auto *lngarr = gamedata->levels[p.first].second;
                std::wstring name = lngarr ? (*lngarr)[lng_] : L"";
                auto rx = float(e.x - currSession_->cx), ry = float(e.y - currSession_->cy);
                /* Check for TalTombs */
                if (p.first == realTombLevelId) {
                    name = L">>> " + name + L" <<<";
                    currSession_->lines.emplace_back(rx, ry);
                }
                if (p.first == superUniqueTombLevelId) {
                    name += L" <== SuperUnique";
                }
                squadPip.pushQuad(px - 4, py - 4, px + 4, py + 4, objColors_[TypePortal]);
                currSession_->textStrings
                    .emplace_back(rx, ry, name, float(ttf_->stringWidth(name, cfg->fontSize)) * .5f);
                if (guides && (*guides).find(p.first) != (*guides).end()) {
                    currSession_->lines.emplace_back(rx, ry);
                }
            }
        }
        const std::map<uint32_t, std::vector<d2mapapi::Point>> *objs[2] = {&currMap->objects, &currMap->npcs};
        for (int i = 0; i < 2; ++i) {
            for (const auto &[id, vec]: *objs[i]) {
                auto ite = gamedata->objects[i].find(id);
                if (ite == gamedata->objects[i].end()) { continue; }
                for (auto &pt: vec) {
                    auto ptx = float(pt.x - totalX0);
                    auto pty = float(pt.y - totalY0);
                    auto tp = std::get<0>(ite->second);
                    switch (tp) {
                    case TypeDoor: {
                        float w = std::get<3>(ite->second) * .5f, h = std::get<4>(ite->second) * .5f;
                        squadPip.pushQuad(ptx - w, pty - h, ptx + w, pty + h, objColors_[tp]);
                        break;
                    }
                    case TypeWayPoint:
                    case TypeQuest:
                    case TypePortal:
                    case TypeChest:
                    case TypeShrine:
                    case TypeWell: {
                        float w = std::get<3>(ite->second) * .5f, h = std::get<4>(ite->second) * .5f;
                        squadPip.pushQuad(ptx - w, pty - h, ptx + w, pty + h, objColors_[tp]);
                        if (tp != TypeShrine && tp != TypeWell) {
                            const auto *lngarr = std::get<2>(ite->second);
                            std::wstring name = lngarr ? (*lngarr)[lng_] : L"";
                            currSession_->textStrings.emplace_back(float(pt.x - currSession_->cx), float(pt.y - currSession_->cy), name, float(ttf_->stringWidth(name, cfg->fontSize)) * .5f);
                        }
                        if (guides && (*guides).find(id | (0x10000 * (i + 1))) != (*guides).end()) {
                            currSession_->lines.emplace_back(float(pt.x - currSession_->cx), float(pt.y - currSession_->cy));
                        }
                        break;
                    }
                    default:
                        break;
                    }
                }
            }
        }
        squadPip.render();
        enabled_ = true;
    } else {
        enabled_ = currSession_->currMap != nullptr;
    }
    if (switched || changed) {
        if (auto *currMap = currSession_->currMap) {
            int x0 = currSession_->x0, y0 = currSession_->y0,
                x1 = currSession_->x1, y1 = currSession_->y1,
                cx = currSession_->cx, cy = currSession_->cy;
            mapPipeline_.reset();
            mapPipeline_.setTexture(currSession_->mapTex);
            mapPipeline_.pushQuad(float(x0 - cx), float(y0 - cy), float(x1 - cx), float(y1 - cy));
            updateWindowPos();
        }
    }
}
void MapRenderer::render() {
    if (!enabled_) { return; }
    auto *ttfPipeline = ttfgl_.pipeline();
    auto widthf = float(mapViewport_[2]) * .5f;
    auto heightf = float(mapViewport_[3]) * .5f;
    ttfPipeline->setViewport(mapViewport_[0], mapViewport_[1], mapViewport_[2], mapViewport_[3]);
    ttfPipeline->setOrtho(-widthf, widthf, heightf, -heightf);
    dynamicTextStrings_.clear();
    dynamicPipeline_.reset();
    messagePipeline_.reset();
    updatePlayerPos();
    mapPipeline_.render();
    framePipeline_.render();
    drawObjects();
    dynamicPipeline_.render();
    messagePipeline_.render();
    auto fontSize = cfg->fontSize;
    for (const auto &[x, y, text, offX]: currSession_->textStrings) {
        if (text.empty()) { continue; }
        auto coord = transform_ * HMM_Vec4(x - 4.f, y - 4.f, 0, 1);
        ttf_->render(text, coord.X - offX, coord.Y - fontSize, false, fontSize);
    }
    for (const auto &s: dynamicTextStrings_) {
        auto coord = transform_ * HMM_Vec4(s.x - 4.f, s.y - 4.f, 0, 1);
        ttf_->render(std::string_view(s.text), coord.X - s.offX, coord.Y - fontSize, false, fontSize);
    }
    for (auto [sv, x, y, fsize, color]: textToDraw_) {
        ttf_->render(sv, x, y, false, fsize, color);
    }
    widthf = float(msgViewport_[2]) * .5f;
    heightf = float(msgViewport_[3]) * .5f;
    ttfPipeline->setViewport(msgViewport_[0], msgViewport_[1], msgViewport_[2], msgViewport_[3]);
    ttfPipeline->setOrtho(-widthf, widthf, heightf, -heightf);
    for (auto [sv, x, y, fsize, color]: msgToDraw_) {
        ttf_->render(sv, x, y, false, fsize, color);
    }
    textToDraw_.clear();
    msgToDraw_.clear();
    updatePanelText();
    if (panelText_.empty()) { return; }
    int vw, vh;
    renderer_.getDimension(vw, vh);
    auto cx = float(vw) * cfg->panelPositionX;
    auto cy = float(vh) * cfg->panelPositionY;
    auto align = cfg->panelAlign;
    auto fontSize2 = cfg->msgFontSize;
    for (const auto &s: panelText_) {
        float nx;
        switch (align) {
        case 1:
            nx = cx - float(ttf_->stringWidth(s, fontSize2)) * .5f;
            break;
        case 2:
            nx = cx - float(ttf_->stringWidth(s, fontSize2));
            break;
        default:
            nx = cx;
            break;
        }
        ttf_->render(s, nx, cy, false, fontSize2, 0);
        cy = cy + fontSize2 + 2;
    }
}
void MapRenderer::updateWindowPos() {
    if (!currSession_) { return; }
    auto *currMap = currSession_->currMap;
    if (!currMap) { return; }
    auto width = d2rRect.right - d2rRect.left, height = d2rRect.bottom - d2rRect.top;
    renderer_.owner()->move(d2rRect.left, d2rRect.top, width, height);

    int x0 = currMap->crop.x0, y0 = currMap->crop.y0, x1 = currMap->crop.x1, y1 = currMap->crop.y1;
    int w, h;
    auto mw = float(x1 - x0) * .5f, mh = float(y1 - y0) * .5f;
    if (cfg->mapAreaW > .0f) {
        w = std::lround(float(width) * cfg->mapAreaW);
        h = std::lround(float(height) * cfg->mapAreaH);
    } else {
        if (cfg->position == 2) {
            w = std::lround(float(width));
            h = std::lround(float(height));
        } else {
            w = (int)lroundf(cfg->scale * (mw + mh) * 1.42f /* sqrt(2) */) + cfg->fontSize;
            h = w / 2;
        }
    }
    auto widthf = float(w) * 0.5f, heightf = float(h) * 0.5f;

    switch (cfg->position) {
    case 0:
        mapViewport_[0] = 0;
        mapViewport_[1] = height - h;
        mapViewport_[2] = w;
        mapViewport_[3] = h;
        break;
    case 1:
        mapViewport_[0] = width - w;
        mapViewport_[1] = height - h;
        mapViewport_[2] = w;
        mapViewport_[3] = h;
        break;
    default:
        mapViewport_[0] = (width - w) / 2;
        mapViewport_[1] = (height - h) / 2;
        mapViewport_[2] = w;
        mapViewport_[3] = h;
        break;
    }
    msgViewport_[0] = 0;
    msgViewport_[1] = 0;
    msgViewport_[2] = width;
    msgViewport_[3] = height;

    mapPipeline_.setViewport(mapViewport_[0], mapViewport_[1], mapViewport_[2], mapViewport_[3]);
    mapPipeline_.setOrtho(-widthf, widthf, heightf, -heightf);
    framePipeline_.setViewport(mapViewport_[0], mapViewport_[1], mapViewport_[2], mapViewport_[3]);
    framePipeline_.reset();
    framePipeline_.setOrtho(-widthf, widthf, heightf, -heightf);
    dynamicPipeline_.setViewport(mapViewport_[0], mapViewport_[1], mapViewport_[2], mapViewport_[3]);
    dynamicPipeline_.reset();
    dynamicPipeline_.setOrtho(-widthf, widthf, heightf, -heightf);
    widthf = float(width) * .5f;
    heightf = float(height) * .5f;
    messagePipeline_.setViewport(0, 0, width, height);
    messagePipeline_.reset();
    messagePipeline_.setOrtho(-widthf, widthf, heightf, -heightf);
    updatePlayerPos();
}
void MapRenderer::reloadConfig() {
    loadFromCfg();
    d2rProcess_.reloadConfig();
}
void MapRenderer::updatePlayerPos() {
    auto *currMap = currSession_->currMap;
    auto cx = currSession_->cx, cy = currSession_->cy;
    const auto *currPlayer = d2rProcess_.currPlayer();
    bool showPlayerNames = cfg->showPlayerNames;
    for (const auto &[id, plr]: d2rProcess_.players()) {
        auto posX = plr.posX;
        auto posY = plr.posY;
        auto oxf = float(posX - cx);
        auto oyf = float(posY - cy);
        if (showPlayerNames && plr.name[0]) {
            dynamicTextStrings_.emplace_back(DynamicTextString { oxf, oyf, plr.name, float(ttf_->stringWidth(plr.name, cfg->fontSize)) * .5f });
        }
        if (&plr == currPlayer) {
            if (posX == currSession_->playerPosX && posY == currSession_->playerPosY) {
                continue;
            }
            currSession_->playerPosX = posX;
            currSession_->playerPosY = posY;
            framePipeline_.reset();
            framePipeline_.pushQuad(oxf - 4, oyf - 4, oxf - 2, oyf + 4, cfg->playerOuterColor);
            framePipeline_.pushQuad(oxf + 2, oyf - 4, oxf + 4, oyf + 4, cfg->playerOuterColor);
            framePipeline_.pushQuad(oxf - 2, oyf - 4, oxf + 2, oyf - 2, cfg->playerOuterColor);
            framePipeline_.pushQuad(oxf - 2, oyf + 2, oxf + 2, oyf + 4, cfg->playerOuterColor);
            framePipeline_.pushQuad(oxf - 2, oyf - 2, oxf + 2, oyf + 2, cfg->playerInnerColor);
            auto c = cfg->lineColor;
            for (auto [x, y]: currSession_->lines) {
                if (cfg->fullLine) {
                    auto line = HMM_Vec2(x, y) - HMM_Vec2(oxf, oyf);
                    auto len = HMM_Length(line);
                    auto sx = oxf + line.X / len * 8.f;
                    auto sy = oyf + line.Y / len * 8.f;
                    auto ex = x - line.X / len * 8.f;
                    auto ey = y - line.Y / len * 8.f;
                    if (len < 17.f) {
                        ex = sx; ey = sy;
                    }
                    framePipeline_.drawLine(sx, sy, ex, ey, 1.5f, c);
                } else {
                    const float mlen = 78.f;
                    const float gap = 12.f;
                    float sx, sy, ex, ey;
                    auto line = HMM_Vec2(x, y) - HMM_Vec2(oxf, oyf);
                    auto len = HMM_Length(line);
                    sx = oxf + line.X / len * 8.f;
                    sy = oyf + line.Y / len * 8.f;
                    if (len > mlen) {
                        ex = oxf + line.X / len * (mlen - gap);
                        ey = oyf + line.Y / len * (mlen - gap);
                    } else if (len > gap) {
                        ex = x - line.X / len * gap;
                        ey = y - line.Y / len * gap;
                    } else {
                        ex = sx;
                        ey = sy;
                    }

                    /* Draw the line */
                    framePipeline_.drawLine(sx, sy, ex, ey, 1.5f, c);

                    /* Draw the dot */
                    if (ex != sx) {
                        ex += line.X / len * gap;
                        ey += line.Y / len * gap;
                        framePipeline_.pushQuad(ex - 3, ey - 3, ex + 1.5f, ey - 1.5f, ex + 3, ey + 3, ex - 1.5f, ey + 1.5f, c);
                    }
                }
            }

            transform_ = HMM_Scale(HMM_Vec3(1, .5f, 1.f))
                * HMM_Rotate(45.f, HMM_Vec3(0, 0, 1))
                * HMM_Scale(HMM_Vec3(cfg->scale, cfg->scale, 1));
            if (cfg->mapCentered) {
                transform_ = transform_ * HMM_Translate(HMM_Vec3(-oxf, -oyf, 0));
            }
            mapPipeline_.setTransform(&transform_.Elements[0][0]);
            framePipeline_.setTransform(&transform_.Elements[0][0]);
            dynamicPipeline_.setTransform(&transform_.Elements[0][0]);
        } else {
            dynamicPipeline_.pushQuad(oxf - 4, oyf - 4, oxf + 4, oyf + 4, cfg->playerOuterColor);
            dynamicPipeline_.pushQuad(oxf - 2, oyf - 2, oxf + 2, oyf + 2, cfg->playerInnerColor);
        }
    }
}
void MapRenderer::drawObjects() {
    auto &mons = d2rProcess_.monsters();
    auto &objs = d2rProcess_.objects();
    auto &items = d2rProcess_.items();
    auto fontSize = cfg->fontSize;
    if (!mons.empty() || !objs.empty() || !items.empty()) {
        auto *currMap = currSession_->currMap;
        auto ctx = currSession_->cx, cty = currSession_->cy;
        for (const auto &mon: mons) {
            auto x = float(mon.x - ctx);
            auto y = float(mon.y - cty);
            dynamicPipeline_.pushQuad(x - 1.5f, y - 1.5f, x + 1.5f, y + 1.5f, objColors_[mon.isNpc ? TypeNpc : mon.isUnique ? TypeUniqueMonster : TypeMonster]);
            if (mon.name) {
                auto coord = transform_ * HMM_Vec4(x - 1.f, y - 1.f, 0, 1);
                std::wstring_view sv = (*mon.name)[lng_];
                textToDraw_.emplace_back(sv, coord.X - float(ttf_->stringWidth(sv, fontSize)) * .5f, coord.Y - fontSize, fontSize, 0);
                if (mon.enchants[0]) {
                    std::wstring_view svenc = mon.enchants;
                    textToDraw_.emplace_back(svenc, coord.X - float(ttf_->stringWidth(svenc, fontSize)) * .5f, coord.Y - fontSize * 2.f, fontSize, 0);
                }
            } else {
                if (mon.enchants[0]) {
                    auto coord = transform_ * HMM_Vec4(x - 1.f, y - 1.f, 0, 1);
                    std::wstring_view svenc = mon.enchants;
                    textToDraw_.emplace_back(svenc, coord.X - float(ttf_->stringWidth(svenc, fontSize)) * .5f, coord.Y - fontSize, fontSize, 0);
                }
            }
        }
        for (const auto &p: objs) {
            const auto &obj = p.second;
            auto x = float(obj.x - ctx);
            auto y = float(obj.y - cty);
            auto dw = obj.w, dh = obj.h;
            dynamicPipeline_.pushQuad(x - dw, y - dh, x + dw, y + dh, objColors_[obj.type]);
            if (obj.name) {
                auto coord = transform_ * HMM_Vec4(x - dw, y - dh, 0, 1);
                std::wstring_view sv = (*obj.name)[lng_];
                /* if type is Portal, the portal target level is stored in field `flag` */
                if (obj.type == TypePortal && obj.flag < gamedata->levels.size()) {
                    const auto *lngarr = gamedata->levels[obj.flag].second;
                    if (lngarr) {
                        sv = (*lngarr)[lng_];
                    }
                }
                textToDraw_.emplace_back(sv, coord.X - float(ttf_->stringWidth(sv, fontSize)) * .5f, coord.Y - fontSize, fontSize, 0);
            }
        }
        if (!items.empty()) {
            int vw, vh;
            renderer_.getDimension(vw, vh);
            auto cx = float(vw) * cfg->msgPositionX;
            auto cy = float(vh) * cfg->msgPositionY;
            auto align = cfg->msgAlign;
            auto fontSize2 = cfg->msgFontSize;
            for (const auto &item: items) {
                std::wstring_view sv = (*item.name)[lng_];
                if (item.flag & 1) {
                    auto x = float(item.x - ctx);
                    auto y = float(item.y - cty);
                    dynamicPipeline_.pushQuad(x - 1.5f, y - 1.5f, x + 1.5f, y + 1.5f, 0xFFFFFFFF);
                    if (item.name) {
                        auto coord = transform_ * HMM_Vec4(x - 4.f, y - 4.f, 0, 1);
                        textToDraw_.emplace_back(sv, coord.X - float(ttf_->stringWidth(sv, fontSize)) * .5f, coord.Y - fontSize, fontSize, item.color);
                    }
                }
                if (item.flag & 2) {
                    auto txtw = float(ttf_->stringWidth(sv, fontSize2));
                    float nx;
                    switch (align) {
                    case 1:
                        nx = cx - txtw * .5f;
                        break;
                    case 2:
                        nx = cx - txtw;
                        break;
                    default:
                        nx = cx;
                        break;
                    }
                    messagePipeline_.pushQuad(nx - 1.f, cy - 1.f, nx + 1.f + txtw, cy + 1.f + fontSize2, cfg->msgBgColor);
                    msgToDraw_.emplace_back(sv, nx, cy, fontSize2, item.color);
                    cy = cy + fontSize2 + 2;
                }
            }
        }
    }
}

void MapRenderer::updatePanelText() {
    auto now = getCurrTime();
    if (now < nextPanelUpdateTime_) {
        return;
    }
    nextPanelUpdateTime_ = now + std::chrono::seconds(1);
    const auto &panels = cfg->panelPatterns;
    auto size = panels.size();
    panelText_.resize(size);
    for (size_t i = 0; i < size; ++i) {
        const auto &pat = panels[i];
        auto &target = panelText_[i];
        target.clear();
        size_t len = pat.size();
        size_t pos = 0;
        while (pos < len) {
            if (pat[pos] == '{') {
                ++pos;
                auto start = pos;
                while (pos < len && pat[pos] != '}') ++pos;
                if (pos >= len) { break; }
                std::wstring_view sv(pat.data() + start, pos - start);
                if (sv == L"duration") {
                    wchar_t n[16];
                    auto dur = uint32_t(std::chrono::duration_cast<std::chrono::seconds>(now - currSession_->mapStartTime).count());
                    if (dur < 3600) {
                        wsprintfW(n, L"%02u:%02u", dur / 60, dur % 60);
                    } else {
                        wsprintfW(n, L"%u:%02u:%02u", dur / 3600, (dur % 3600) / 60, dur % 60);
                    }
                    target += n;
                } else if (sv == L"time") {
                    wchar_t n[16];
                    auto currUnixTime = time(nullptr);
                    struct tm tm = {};
                    localtime_s(&tm, &currUnixTime);
                    wsprintfW(n, L"%d:%02d:%02d", tm.tm_hour, tm.tm_min, tm.tm_sec);
                    target += n;
                } else if (sv == L"difficulty") {
                    const auto *cpl = d2rProcess_.currPlayer();
                    if (cpl) {
                        static const char *diffKey[] = {"strCreateGameNormalText", "strCreateGameNightmareText", "strCreateGameHellText"};
                        auto ite = gamedata->strings.find(diffKey[std::min(uint8_t(2), cpl->difficulty)]);
                        if (ite != gamedata->strings.end()) {
                            target += ite->second[lng_];
                        }
                    }
                } else if (sv == L"act") {
                    const auto *cpl = d2rProcess_.currPlayer();
                    if (cpl) {
                        static const wchar_t *actNames[] = {L"ACT I", L"ACT II", L"ACT III", L"ACT IV", L"ACT V"};
                        target += actNames[std::min(4u, cpl->act)];
                    }
                } else if (sv == L"mapname") {
                    const auto *cpl = d2rProcess_.currPlayer();
                    if (cpl) {
                        if (cpl->levelId < gamedata->levels.size()) {
                            target += (*gamedata->levels[cpl->levelId].second)[lng_];
                        }
                    }
                }
            } else {
                target += pat[pos];
            }
            ++pos;
        }
    }
}
void MapRenderer::loadFromCfg() {
    ttf_ = std::make_unique<TTF>(ttfgl_);
    ttf_->add(cfg->fontFilePath);
    lng_ = lngFromString(cfg->language);
    walkableColor_ = cfg->walkableColor;
    objColors_[TypeWayPoint] = cfg->waypointColor;
    objColors_[TypePortal] = cfg->portalColor;
    objColors_[TypeChest] = cfg->chestColor;
    objColors_[TypeQuest] = cfg->questColor;
    objColors_[TypeShrine] = cfg->shrineColor;
    objColors_[TypeWell] = cfg->wellColor;
    objColors_[TypeMonster] = cfg->monsterColor;
    objColors_[TypeUniqueMonster] = cfg->uniqueMonsterColor;
    objColors_[TypeNpc] = cfg->npcColor;
    objColors_[TypeDoor] = cfg->doorColor;
    auto alpha = (cfg->textColor >> 24);
    ttf_->setColor(cfg->textColor & 0xFF, (cfg->textColor >> 8) & 0xFF, (cfg->textColor >> 16) & 0xFF, alpha);
    ttf_->setAltColor(1, 228, 88, 67, alpha);
    ttf_->setAltColor(2, 31, 255, 0, alpha);
    ttf_->setAltColor(3, 104, 104, 223, alpha);
    ttf_->setAltColor(4, 192, 166, 130, alpha);
    ttf_->setAltColor(5, 104, 104, 104, alpha);
    ttf_->setAltColor(6, 0, 0, 0, alpha);
    ttf_->setAltColor(7, 223, 202, 130, alpha);
    ttf_->setAltColor(8, 255, 171, 41, alpha);
    ttf_->setAltColor(9, 255, 239, 130, alpha);
    ttf_->setAltColor(10, 31, 130, 10, alpha);
    ttf_->setAltColor(11, 213, 41, 255, alpha);
    ttf_->setAltColor(12, 52, 161, 26, alpha);
    ttf_->setAltColor(13, 255, 255, 255, alpha);
    ttf_->setAltColor(14, 255, 255, 255, alpha);
    ttf_->setAltColor(15, 255, 255, 255, alpha);
    plugin_.load();
}

d2mapapi::CollisionMap *MapRenderer::getMap(uint32_t levelId) {
    if (!currSession_) { return nullptr; }
    d2mapapi::CollisionMap *currMap;
    auto &map = currSession_->maps[levelId];
    if (!map) {
        currMap = childProcess_.queryMap(currSession_->currSeed, currSession_->currDifficulty, levelId);
        map = std::unique_ptr<d2mapapi::CollisionMap>(currMap);
    } else {
        currMap = map.get();
    }
    return currMap;
}

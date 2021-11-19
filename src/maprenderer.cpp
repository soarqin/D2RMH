/*
 * Copyright (c) 2021 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#include "maprenderer.h"

#include "window.h"
#include "cfg.h"

static JsonLng::LNG lngFromString(const std::string &language) {
    if (language == "enUS") return JsonLng::LNG_enUS;
    if (language == "zhTW") return JsonLng::LNG_zhTW;
    if (language == "deDE") return JsonLng::LNG_deDE;
    if (language == "esES") return JsonLng::LNG_esES;
    if (language == "frFR") return JsonLng::LNG_frFR;
    if (language == "itIT") return JsonLng::LNG_itIT;
    if (language == "koKR") return JsonLng::LNG_koKR;
    if (language == "plPL") return JsonLng::LNG_plPL;
    if (language == "esMX") return JsonLng::LNG_esMX;
    if (language == "jaJP") return JsonLng::LNG_jaJP;
    if (language == "ptBR") return JsonLng::LNG_ptBR;
    if (language == "ruRU") return JsonLng::LNG_ruRU;
    if (language == "zhCN") return JsonLng::LNG_zhCN;
    return JsonLng::LNG_enUS;
}

MapRenderer::MapRenderer(Renderer &renderer) :
    renderer_(renderer),
    mapPipeline_(renderer),
    framePipeline_(renderer),
    dynamicPipeline_(renderer),
    messagePipeline_(renderer),
    ttfgl_(renderer),
    ttf_(ttfgl_),
    walkableColor_(cfg->walkableColor) {
    ttf_.add(cfg->fontFilePath);
    mapPipeline_.setTexture(mapTex_);
    lng_ = lngFromString(cfg->language);
    d2rProcess_.setWindowPosCallback([this](int left, int top, int right, int bottom) {
        d2rRect = {left, top, right, bottom};
        updateWindowPos();
    });
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
    ttf_.setColor(cfg->textColor & 0xFF, (cfg->textColor >> 8) & 0xFF, (cfg->textColor >> 16) & 0xFF);
    ttf_.setAltColor(1, 228, 88, 67);
    ttf_.setAltColor(2, 31, 255, 0);
    ttf_.setAltColor(3, 104, 104, 223);
    ttf_.setAltColor(4, 192, 166, 130);
    ttf_.setAltColor(5, 104, 104, 104);
    ttf_.setAltColor(6, 0, 0, 0);
    ttf_.setAltColor(7, 223, 202, 130);
    ttf_.setAltColor(8, 255, 171, 41);
    ttf_.setAltColor(9, 255, 239, 130);
    ttf_.setAltColor(10, 31, 130, 10);
    ttf_.setAltColor(11, 213, 41, 255);
    ttf_.setAltColor(12, 52, 161, 26);
    ttf_.setAltColor(13, 255, 255, 255);
    ttf_.setAltColor(14, 255, 255, 255);
    ttf_.setAltColor(15, 255, 255, 255);
}

void MapRenderer::update() {
    d2rProcess_.updateData();
    if (!d2rProcess_.available()) {
        enabled_ = false;
        return;
    }
    if (d2rProcess_.panelEnabled() & cfg->panelMask) {
        enabled_ = false;
        return;
    }
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
    if (!enabled_) {
        return;
    }
    const auto *currPlayer = d2rProcess_.currPlayer();
    if (!currPlayer) {
        return;
    }
    bool changed = session_.update(currPlayer->seed, currPlayer->difficulty);
    if (uint32_t levelId = currPlayer->levelId; levelId != currLevelId_) {
        currLevelId_ = levelId;
        changed = true;
    }
    if (changed) {
        textStrings_.clear();
        lines_.clear();
        currMap_ = session_.getMap(currLevelId_);
        if (!currMap_) {
            enabled_ = false;
            return;
        }
        int x0 = currMap_->cropX, y0 = currMap_->cropY,
            x1 = currMap_->cropX2, y1 = currMap_->cropY2;
        int width = std::max(0, x1 - x0);
        int height = std::max(0, y1 - y0);
        auto totalWidth = currMap_->totalWidth;
        auto *pixels = new uint32_t[width * height];
        auto *ptr = pixels;
        auto edgeColor = cfg->edgeColor;
        bool hasEdge = (edgeColor & 0xFFFFFFu) != 0;
        for (int y = y0; y < y1; ++y) {
            int idx = y * totalWidth + x0;
            for (int x = x0; x < x1; ++x) {
                bool blocked = currMap_->map[idx] & 1;
                uint32_t clr = blocked ? 0 : walkableColor_;
                if (hasEdge && !blocked && (
                    (x > x0 && (currMap_->map[idx - 1] & 1))
                    || (x + 1 < x1 && (currMap_->map[idx + 1] & 1))
                    || (y > y0 && (currMap_->map[idx - totalWidth] & 1))
                    || (y + 1 < y1 && (currMap_->map[idx + totalWidth] & 1))
                )) {
                    clr = edgeColor;
                }
                *ptr++ = clr;
                ++idx;
            }
        }
        mapTex_.setData(width, height, pixels);
        delete[] pixels;

        const std::set<int> *guides;
        {
            auto gdite = gamedata->guides.find(currLevelId_);
            if (gdite != gamedata->guides.end()) {
                guides = &gdite->second;
            } else {
                guides = nullptr;
            }
        }

        PipelineSquad2D squadPip(mapTex_);
        squadPip.setOrtho(0, float(mapTex_.width()), 0, float(mapTex_.height()));
        auto originX = currMap_->levelOrigin.x, originY = currMap_->levelOrigin.y;
        auto widthf = float(width) * .5f, heightf = float(height) * .5f;
        for (auto &p: currMap_->adjacentLevels) {
            if (p.first >= gamedata->levels.size()) { continue; }
            if (p.second.exits.empty()) {
                continue;
            }
            auto &e = p.second.exits[0];
            auto px = float(e.x - originX - x0);
            auto py = float(e.y - originY - y0);
            const auto *lngarr = gamedata->levels[p.first].second;
            std::wstring name = lngarr ? (*lngarr)[lng_] : L"";
            /* Check for TalTombs */
            if (p.first >= 66 && p.first <= 72) {
                auto *m = session_.getMap(p.first);
                if (m && m->objects.find(152) != m->objects.end()) {
                    name = L">>> " + name + L" <<<";
                    lines_.emplace_back(px - widthf, py - heightf);
                }
            }
            squadPip.pushQuad(px - 4, py - 4, px + 4, py + 4, objColors_[TypePortal]);
            textStrings_.emplace_back(px - widthf, py - heightf, name, float(ttf_.stringWidth(name, cfg->fontSize)) * .5f);
            if (guides && (*guides).find(p.first) != (*guides).end()) {
                lines_.emplace_back(px - widthf, py - heightf);
            }
        }
        std::map<uint32_t, std::vector<Point>> *objs[2] = {&currMap_->objects, &currMap_->npcs};
        for (int i = 0; i < 2; ++i) {
            for (const auto &[id, vec]: *objs[i]) {
                auto ite = gamedata->objects[i].find(id);
                if (ite == gamedata->objects[i].end()) { continue; }
                for (auto &pt: vec) {
                    auto ptx = float(pt.x - originX - x0);
                    auto pty = float(pt.y - originY - y0);
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
                            textStrings_.emplace_back(ptx - widthf, pty - heightf, name, float(ttf_.stringWidth(name, cfg->fontSize)) * .5f);
                        }
                        if (guides && (*guides).find(id | (0x10000 * (i + 1))) != (*guides).end()) {
                            lines_.emplace_back(ptx - widthf, pty - heightf);
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
        updateWindowPos();
        enabled_ = true;
    } else {
        enabled_ = currMap_ != nullptr;
    }
}
void MapRenderer::render() {
    if (enabled_) {
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
        for (const auto &[x, y, text, offX]: textStrings_) {
            if (text.empty()) { continue; }
            auto coord = transform_ * HMM_Vec4(x - 4.f, y - 4.f, 0, 1);
            ttf_.render(text, coord.X - offX, coord.Y - fontSize, false, fontSize);
        }
        for (const auto &s: dynamicTextStrings_) {
            auto coord = transform_ * HMM_Vec4(s.x - 4.f, s.y - 4.f, 0, 1);
            ttf_.render(std::string_view(s.text), coord.X - s.offX, coord.Y - fontSize, false, fontSize);
        }
        for (auto [sv, x, y, fsize, color]: textToDraw_) {
            ttf_.render(sv, x, y, false, fsize, color);
        }
        widthf = float(msgViewport_[2]) * .5f;
        heightf = float(msgViewport_[3]) * .5f;
        ttfPipeline->setViewport(msgViewport_[0], msgViewport_[1], msgViewport_[2], msgViewport_[3]);
        ttfPipeline->setOrtho(-widthf, widthf, heightf, -heightf);
        for (auto [sv, x, y, fsize, color]: msgToDraw_) {
            ttf_.render(sv, x, y, false, fsize, color);
        }
        textToDraw_.clear();
        msgToDraw_.clear();
    }
}
void MapRenderer::updateWindowPos() {
    if (!currMap_) { return; }
    auto width = d2rRect.right - d2rRect.left, height = d2rRect.bottom - d2rRect.top;
    renderer_.owner()->move(d2rRect.left, d2rRect.top, width, height);
    int w = std::lround(float(width) * cfg->mapAreaW);
    int h = std::lround(float(height) * cfg->mapAreaH);
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

    int x0 = currMap_->cropX, y0 = currMap_->cropY, x1 = currMap_->cropX2,
        y1 = currMap_->cropY2;
    auto mw = float(x1 - x0) * .5f, mh = float(y1 - y0) * .5f;
    mapPipeline_.setViewport(mapViewport_[0], mapViewport_[1], mapViewport_[2], mapViewport_[3]);
    mapPipeline_.reset();
    mapPipeline_.setOrtho(-widthf, widthf, heightf, -heightf);
    mapPipeline_.pushQuad(-mw, -mh, mw, mh);
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
void MapRenderer::updatePlayerPos() {
    int x0 = currMap_->cropX, y0 = currMap_->cropY, x1 = currMap_->cropX2,
        y1 = currMap_->cropY2;
    auto originX = currMap_->levelOrigin.x, originY = currMap_->levelOrigin.y;
    auto widthf = (float)(x1 - x0) * 0.5f, heightf = (float)(y1 - y0) * 0.5f;
    const auto *currPlayer = d2rProcess_.currPlayer();
    bool showPlayerNames = cfg->showPlayerNames;
    for (const auto &[id, plr]: d2rProcess_.players()) {
        auto posX = plr.posX;
        auto posY = plr.posY;
        auto oxf = float(posX - originX - x0) - widthf;
        auto oyf = float(posY - originY - y0) - heightf;
        if (showPlayerNames && plr.name[0]) {
            dynamicTextStrings_.emplace_back(DynamicTextString { oxf, oyf, plr.name, float(ttf_.stringWidth(plr.name, cfg->fontSize)) * .5f });
        }
        if (&plr == currPlayer) {
            if (posX == playerPosX_ && posY == playerPosY_) {
                continue;
            }
            playerPosX_ = posX;
            playerPosY_ = posY;
            framePipeline_.reset();
            framePipeline_.pushQuad(oxf - 4, oyf - 4, oxf + 4, oyf + 4, cfg->playerOuterColor);
            framePipeline_.pushQuad(oxf - 2, oyf - 2, oxf + 2, oyf + 2, cfg->playerInnerColor);
            auto c = cfg->lineColor;
            for (auto [x, y]: lines_) {
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
        int x0 = currMap_->cropX, y0 = currMap_->cropY, x1 = currMap_->cropX2,
            y1 = currMap_->cropY2;
        auto originX = currMap_->levelOrigin.x;
        auto originY = currMap_->levelOrigin.y;
        auto w = float(x1 - x0) * .5f;
        auto h = float(y1 - y0) * .5f;
        for (const auto &mon: mons) {
            auto x = float(mon.x - originX - x0) - w;
            auto y = float(mon.y - originY - y0) - h;
            dynamicPipeline_.pushQuad(x - 1.5f, y - 1.5f, x + 1.5f, y + 1.5f, objColors_[mon.isNpc ? TypeNpc : mon.isUnique ? TypeUniqueMonster : TypeMonster]);
            if (mon.name) {
                auto coord = transform_ * HMM_Vec4(x - 1.f, y - 1.f, 0, 1);
                std::wstring_view sv = (*mon.name)[lng_];
                textToDraw_.emplace_back(sv, coord.X - float(ttf_.stringWidth(sv, fontSize)) * .5f, coord.Y - fontSize, fontSize, 0);
                if (mon.enchants[0]) {
                    std::wstring_view svenc = mon.enchants;
                    textToDraw_.emplace_back(svenc, coord.X - float(ttf_.stringWidth(svenc, fontSize)) * .5f, coord.Y - fontSize * 2.f, fontSize, 0);
                }
            } else {
                if (mon.enchants[0]) {
                    auto coord = transform_ * HMM_Vec4(x - 1.f, y - 1.f, 0, 1);
                    std::wstring_view svenc = mon.enchants;
                    textToDraw_.emplace_back(svenc, coord.X - float(ttf_.stringWidth(svenc, fontSize)) * .5f, coord.Y - fontSize, fontSize, 0);
                }
            }
        }
        for (const auto &p: objs) {
            const auto &obj = p.second;
            auto x = float(obj.x - originX - x0) - w;
            auto y = float(obj.y - originY - y0) - h;
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
                textToDraw_.emplace_back(sv, coord.X - float(ttf_.stringWidth(sv, fontSize)) * .5f, coord.Y - fontSize, fontSize, 0);
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
                    auto x = float(item.x - originX - x0) - w;
                    auto y = float(item.y - originY - y0) - h;
                    dynamicPipeline_.pushQuad(x - 1.5f, y - 1.5f, x + 1.5f, y + 1.5f, 0xFFFFFFFF);
                    if (item.name) {
                        auto coord = transform_ * HMM_Vec4(x - 4.f, y - 4.f, 0, 1);
                        textToDraw_.emplace_back(sv, coord.X - float(ttf_.stringWidth(sv, fontSize)) * .5f, coord.Y - fontSize, fontSize, item.color);
                    }
                }
                if (item.flag & 2) {
                    auto txtw = float(ttf_.stringWidth(sv, fontSize2));
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

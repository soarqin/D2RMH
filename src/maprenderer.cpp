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
    ttf_.setColor(cfg->textColor & 0xFF, (cfg->textColor >> 8) & 0xFF, (cfg->textColor >> 16) & 0xFF);
}
void MapRenderer::update() {
    d2rProcess_.updateData();
    if (!d2rProcess_.available()) {
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
    bool changed = session_.update(d2rProcess_.seed(), d2rProcess_.difficulty());
    if (uint32_t levelId = d2rProcess_.levelId(); levelId != currLevelId_) {
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
        for (int y = y0; y < y1; ++y) {
            int idx = y * totalWidth + x0;
            for (int x = x0; x < x1; ++x) {
                auto clr = currMap_->map[idx++] & 1 ? 0 : walkableColor_;
                *ptr++ = clr;
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
        squadPip.setOrtho(0, mapTex_.width(), 0, mapTex_.height());
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
            auto strite = gamedata->strings.find(gamedata->levels[p.first]);
            std::wstring name = strite != gamedata->strings.end() ? strite->second[lng_] : L"";
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
                    auto tp = ite->second.type;
                    switch (tp) {
                    case TypeWayPoint:
                    case TypeQuest:
                    case TypePortal:
                    case TypeChest:
                    case TypeShrine:
                    case TypeWell: {
                        squadPip.pushQuad(ptx - 4, pty - 4, ptx + 4, pty + 4, objColors_[tp]);
                        if (tp != TypeShrine && tp != TypeWell) {
                            auto strite = gamedata->strings.find(ite->second.name);
                            std::wstring name = strite != gamedata->strings.end() ? strite->second[lng_] : L"";
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
    }
    enabled_ = true;
}
void MapRenderer::render() {
    if (enabled_) {
        updatePlayerPos();
        mapPipeline_.render();
        framePipeline_.render();
        dynamicPipeline_.render();
        drawObjects();
        auto fontSize = cfg->fontSize;
        for (const auto &[x, y, text, offX]: textStrings_) {
            if (text.empty()) { continue; }
            auto coord = transform_ * HMM_Vec4(x - 4.f, y - 4.f, 0, 1);
            ttf_.render(text, coord.X - offX, coord.Y - fontSize, false, fontSize);
        }
    }
}
void MapRenderer::updateWindowPos() {
    if (!currMap_) { return; }
    int x0 = currMap_->cropX, y0 = currMap_->cropY, x1 = currMap_->cropX2,
        y1 = currMap_->cropY2;
    int width = x1 - x0;
    int height = y1 - y0;
    auto windowSize = (int)lroundf(cfg->scale * (float)(width + height) * 0.75) + 8;
    const auto bear = 16;
    if (windowSize + bear * 2 > d2rRect.right - d2rRect.left) {
        windowSize = d2rRect.right - d2rRect.left - bear * 2;
    }
    if (windowSize / 2 + bear * 2 > d2rRect.bottom - d2rRect.top) {
        windowSize = (d2rRect.bottom - d2rRect.top - bear * 2) * 2;
    }
    float w, h;
    switch (cfg->position) {
    case 0:
        renderer_.owner()->move(d2rRect.left + bear, d2rRect.top + bear, windowSize, windowSize / 2);
        w = (float)windowSize;
        h = w / 2;
        break;
    case 1:
        renderer_.owner()->move(d2rRect.right - windowSize - bear, d2rRect.top + bear, windowSize, windowSize / 2);
        w = (float)windowSize;
        h = w / 2;
        break;
    default:
        renderer_.owner()->move(d2rRect.left + bear, d2rRect.top + bear, d2rRect.right - d2rRect.left - bear * 2, d2rRect.bottom - d2rRect.top - bear * 2);
        w = (float)(d2rRect.right - d2rRect.left - bear * 2);
        h = (float)(d2rRect.bottom - d2rRect.top - bear * 2);
        break;
    }
    auto widthf = (float)width * 0.5f, heightf = (float)height * 0.5f;
    mapPipeline_.reset();
    mapPipeline_.setOrtho(-w / 2, w / 2, h / 2, -h / 2);
    mapPipeline_.pushQuad(-widthf, -heightf, widthf, heightf);
    framePipeline_.reset();
    framePipeline_.setOrtho(-w / 2, w / 2, h / 2, -h / 2);
    dynamicPipeline_.reset();
    dynamicPipeline_.setOrtho(-w / 2, w / 2, h / 2, -h / 2);
    ttfgl_.pipeline()->setOrtho(-w / 2, w / 2, h / 2, -h / 2);
    updatePlayerPos();
}
void MapRenderer::updatePlayerPos() {
    if (!currMap_) { return; }
    auto posX = d2rProcess_.posX();
    auto posY = d2rProcess_.posY();
    if (posX == playerPosX_ && posY == playerPosY_) {
        return;
    }
    playerPosX_ = posX;
    playerPosY_ = posY;
    int x0 = currMap_->cropX, y0 = currMap_->cropY, x1 = currMap_->cropX2,
        y1 = currMap_->cropY2;
    auto originX = currMap_->levelOrigin.x, originY = currMap_->levelOrigin.y;
    posX -= originX + x0;
    posY -= originY + y0;
    auto widthf = (float)(x1 - x0) * 0.5f, heightf = (float)(y1 - y0) * 0.5f;
    auto oxf = float(posX) - widthf;
    auto oyf = float(posY) - heightf;
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

            const float angle = 35.f;
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
}
void MapRenderer::drawObjects() {
    auto &mons = d2rProcess_.monsters();
    auto &objs = d2rProcess_.objects();
    if (!mons.empty() || !objs.empty()) {
        auto fontSize = cfg->fontSize;

        int x0 = currMap_->cropX, y0 = currMap_->cropY, x1 = currMap_->cropX2,
            y1 = currMap_->cropY2;
        auto originX = currMap_->levelOrigin.x;
        auto originY = currMap_->levelOrigin.y;
        auto w = float(x1 - x0) * .5f;
        auto h = float(y1 - y0) * .5f;
        dynamicPipeline_.reset();
        for (const auto &[id, mon]: mons) {
            auto x = float(mon.x - originX - x0) - w;
            auto y = float(mon.y - originY - y0) - h;
            dynamicPipeline_.pushQuad(x - 1.5f, y - 1.5f, x + 1.5f, y + 1.5f, objColors_[TypeMonster]);
            if (mon.name) {
                auto coord = transform_ * HMM_Vec4(x - 1.f, y - 1.f, 0, 1);
                std::wstring_view sv = (*mon.name)[lng_];
                ttf_.render(sv, coord.X - float(ttf_.stringWidth(sv, fontSize)) * .5f, coord.Y - fontSize, false, fontSize);
                if (mon.enchants[0]) {
                    std::wstring_view svenc = mon.enchants;
                    ttf_.render(svenc, coord.X - float(ttf_.stringWidth(svenc, fontSize)) * .5f, coord.Y - fontSize * 2.f, false, fontSize);
                }
            } else {
                if (mon.enchants[0]) {
                    auto coord = transform_ * HMM_Vec4(x - 1.f, y - 1.f, 0, 1);
                    std::wstring_view svenc = mon.enchants;
                    ttf_.render(svenc, coord.X - float(ttf_.stringWidth(svenc, fontSize)) * .5f, coord.Y - fontSize, false, fontSize);
                }
            }
        }
        for (const auto &[id, obj]: objs) {
            auto x = float(obj.x - originX - x0) - w;
            auto y = float(obj.y - originY - y0) - h;
            dynamicPipeline_.pushQuad(x - 4, y - 4, x + 4, y + 4, objColors_[obj.type]);
            if (obj.name) {
                auto coord = transform_ * HMM_Vec4(x - 4.f, y - 4.f, 0, 1);
                std::wstring_view sv = (*obj.name)[lng_];
                ttf_.render(sv, coord.X - float(ttf_.stringWidth(sv, fontSize)) * .5f, coord.Y - fontSize, false, fontSize);
            }
        }
        dynamicPipeline_.render();
    }
}

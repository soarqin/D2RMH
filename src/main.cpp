#include "d2map.h"
#include "session.h"
#include "d2rprocess.h"
#include "cfg.h"
#include "ini.h"
#define TRAY_WINAPI 1
#include "tray/tray.h"
#include "../common/jsonlng.h"

#include "sokol/HandmadeMath.h"

#define FONTSTASH_IMPLEMENTATION
#include "sokol/fontstash.h"

#define SOKOL_GLCORE33
#define SOKOL_IMPL
#define SOKOL_GL_IMPL
#include "sokol/sokol_app.h"
#include "sokol/sokol_gfx.h"
#include "sokol/sokol_glue.h"
#include "sokol/sokol_gl.h"
#include "sokol/sokol_fontstash.h"

#include <unordered_map>
#include <map>
#include <set>
#include <iostream>
#include <fstream>

static sg_pass_action pass_action = {};
static struct {
    bool enable = false;
    sg_pipeline pip = {};
    sg_bindings bind = {};
    int count = 0;
} drawstate[2];

struct MapObject {
    /* Rendering coord */
    float x, y;
    /* Rendering color */
    uint32_t color;
    std::string str;
};

struct LinePoint {
    float x, y;
};

#define RGBA(r, g, b, a) (uint32_t(r) | (uint32_t(g) << 8) | (uint32_t(b) << 16) | (uint32_t(a) << 24))

struct DrawVertex {
    float x, y;
    uint32_t color;
};

static struct {
    hmm_mat4 baseMVP = {}, baseMapMVP = {}, mapMVP = {}, textMVP = {};
    float dpiScale = .0f;
    FONScontext *fonsCtx = nullptr;
    unsigned char *fontBuf = nullptr;
    size_t fontBufSize = 0;
    int font = FONS_INVALID;
    std::vector<MapObject> mapObjs;
    std::vector<LinePoint> lineEnds;

    uint16_t *indices = nullptr;
    DrawVertex *vertices = nullptr;
    int drawArrayCapacity = 0;
    float offsetX = 0, offsetY = 0;
} skstate;

enum EObjType {
    TypeNone,
    TypeWayPoint,
    TypePortal,
    TypeChest,
    TypeQuest,
    TypeShrine,
    TypeWell,
    TypeMax,
};

static struct {
    Session *session = nullptr;
    CollisionMap *currMap = nullptr;
    D2RProcess *d2rProcess = nullptr;
    JsonLng::LNG language = JsonLng::LNG_enUS;
    int currLevelId = -1;
    uint32_t currSeed = 0;
    uint8_t currDifficulty = 0xFF;
    uint16_t currPosX = 0, currPosY = 0;
    uint32_t objColors[TypeMax] = {
        0,
        RGBA(153, 153, 255, 255),
        RGBA(255, 153, 255, 255),
        RGBA(255, 104, 104, 255),
        RGBA(104, 104, 255, 255),
        RGBA(255, 51, 178, 255),
        RGBA(51, 51, 255, 255),
    };
    uint32_t mapColor[2] = {
        RGBA(50, 50, 50, 255),
        RGBA(0, 0, 0, 255),
    };
    struct ObjType {
        EObjType type;
        std::string name;
    };
    std::unordered_map<std::string, std::array<std::string, JsonLng::LNG_MAX>> strings;
    std::map<int, std::string> levels;
    std::map<int, ObjType> objects[2];
    std::map<int, std::set<int>> guides;
} mapstate;

static int round_pow2(float v) {
    uint32_t vi = ((uint32_t)v) - 1;
    for (uint32_t i = 0; i < 5; i++) {
        vi |= (vi >> (1 << i));
    }
    return (int)(vi + 1);
}

unsigned char *loadFromFile(const char *filename, size_t &size) {
    std::ifstream ifs(filename, std::ios::in | std::ios::binary);
    if (!ifs.is_open()) {
        return nullptr;
    }
    ifs.seekg(0, std::ios::end);
    size = ifs.tellg();
    ifs.seekg(0, std::ios::beg);
    auto *buf = new unsigned char[size];
    ifs.read((char *)buf, size);
    ifs.close();
    return buf;
}

static void initData() {
    int section = -1;
    ini_parse("D2RMH_data.ini", [](void* user, const char* section,
                             const char* name, const char* value)->int {
        auto *isec = (int*)user;
        if (!name) {
            if (!strcmp(section, "guides")) { *isec = 0; }
            else if (!strcmp(section, "levels")) { *isec = 1; }
            else if (!strcmp(section, "objects")) { *isec = 2; }
            else if (!strcmp(section, "npcs")) { *isec = 3; }
            else if (!strcmp(section, "strings")) { *isec = 4; }
            else { *isec = -1; }
            return 1;
        }
        switch (*isec) {
        case 0: {
            int from = strtol(name, nullptr, 0);
            int to;
            if (value[0] == '+') {
                to = strtol(value + 1, nullptr, 0) | 0x10000;
            } else if (value[0] == '-') {
                to = strtol(value + 1, nullptr, 0) | 0x20000;
            } else {
                to = strtol(value, nullptr, 0);
            }
            mapstate.guides[from].insert(to);
            break;
        }
        case 1:
            mapstate.levels[strtol(name, nullptr, 0)] = value;
            break;
        case 2: case 3: {
            const char *pos = strchr(value, '|');
            if (!pos) { break; }
            auto ssize = pos - value;
            EObjType t = TypeNone;
            if (!strncmp(value, "Waypoint", ssize)) { t = TypeWayPoint; }
            else if (!strncmp(value, "Quest", ssize)) { t = TypeQuest; }
            else if (!strncmp(value, "Portal", ssize)) { t = TypePortal; }
            else if (!strncmp(value, "Chest", ssize)) { t = TypeChest; }
            else if (!strncmp(value, "Shrine", ssize)) { t = TypeShrine; }
            else if (!strncmp(value, "Well", ssize)) { t = TypeWell; }
            mapstate.objects[*isec - 2][strtol(name, nullptr, 0)] = { t, pos + 1 };
            break;
        }
        case 4: {
            const char *pos = strchr(name, '[');
            if (!pos) { break; }
            auto index = strtoul(pos + 1, nullptr, 0);
            if (index < 0 || index >= JsonLng::LNG_MAX) { break; }
            char realname[256];
            auto ssize = pos - name;
            memcpy(realname, name, ssize);
            realname[ssize] = 0;
            mapstate.strings[realname][index] = value;
            break;
        }
        default:
            break;
        }
        return 1;
    }, &section);
}

static void initSokol() {
    mapstate.objColors[TypeWayPoint] = cfg->waypointColor;
    mapstate.objColors[TypePortal] = cfg->portalColor;
    mapstate.objColors[TypeChest] = cfg->chestColor;
    mapstate.objColors[TypeQuest] = cfg->questColor;
    mapstate.objColors[TypeWell] = cfg->wellColor;
    mapstate.mapColor[0] = cfg->walkableColor;

    sg_setup(sg_desc{
        .context = sapp_sgcontext()
    });
    sgl_setup(sgl_desc_t{
        .sample_count = sapp_sample_count()
    });

    skstate.dpiScale = sapp_dpi_scale();
    const int atlas_dim = round_pow2(512.0f * skstate.dpiScale);
    skstate.fonsCtx = sfons_create(atlas_dim, atlas_dim, FONS_ZERO_TOPLEFT);
    skstate.fontBuf = loadFromFile(cfg->fontFilePath.c_str(), skstate.fontBufSize);
    if (skstate.fontBuf) {
        skstate.font = fonsAddFontMem(skstate.fonsCtx, "normal", skstate.fontBuf, skstate.fontBufSize, 0);
    }
    skstate.baseMVP = HMM_Scale(HMM_Vec3(1, 0.5, 1)) * HMM_Rotate(45.f, HMM_Vec3(0, 0, 1));

    pass_action = sg_pass_action{
        .colors = {
            {.action = SG_ACTION_CLEAR, .value = {0, 0, 0, 1}}
        },
    };

    const uint16_t indices[] = {
        0, 1, 2, 0, 2, 3,
    };
    drawstate[0].bind.index_buffer = sg_make_buffer(sg_buffer_desc{
        .type = SG_BUFFERTYPE_INDEXBUFFER,
        .data = SG_RANGE(indices),
        .label = "draw-indices"
    });
    drawstate[0].bind.vertex_buffers[0] = sg_make_buffer(sg_buffer_desc{
        .size = sizeof(float) * 5 * 4,
        .usage = SG_USAGE_DYNAMIC,
        .label = "draw-vertices"
    });
    sg_shader image_shd = sg_make_shader(sg_shader_desc{
        .vs = {
            .source =
            "#version 330\n"
            "uniform mat4 mvp;\n"
            "layout(location=0) in vec2 position;\n"
            "layout(location=1) in vec2 texcoord0;\n"
            "out vec2 uv;\n"
            "void main() {\n"
            "  gl_Position = mvp * vec4(position, 0, 1);\n"
            "  uv = texcoord0;\n"
            "}\n",
            .uniform_blocks = {
                {
                    .size = sizeof(hmm_mat4),
                    .uniforms = {
                        {.name="mvp", .type=SG_UNIFORMTYPE_MAT4}
                    }
                }
            },
        },
        .fs = {
            .source =
            "#version 330\n"
            "uniform sampler2D tex;\n"
            "in vec2 uv;\n"
            "out vec4 frag_color;\n"
            "void main() {\n"
            "  frag_color = texture(tex, uv);\n"
            "}\n",
            .images = {{.name="tex", .image_type=SG_IMAGETYPE_2D}},
        }
    });
    drawstate[0].pip = sg_make_pipeline(sg_pipeline_desc{
        .shader = image_shd,
        .layout = {
            .attrs = {
                {.format = SG_VERTEXFORMAT_FLOAT2},
                {.format = SG_VERTEXFORMAT_FLOAT2},
            }
        },
        .colors = {
            {
                .write_mask = SG_COLORMASK_RGB,
                .blend = {
                    .enabled = true,
                    .src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA,
                    .dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
                },
            },
        },
        .index_type = SG_INDEXTYPE_UINT16,
        .label = "draw-pipeline",
    });

    drawstate[1].bind.index_buffer = sg_make_buffer(sg_buffer_desc{
        .size = sizeof(uint16_t) * 6 * 256,
        .type = SG_BUFFERTYPE_INDEXBUFFER,
        .usage = SG_USAGE_DYNAMIC,
        .label = "color-indices"
    });
    drawstate[1].bind.vertex_buffers[0] = sg_make_buffer(sg_buffer_desc{
        .size = sizeof(float) * 6 * 4 * 256,
        .usage = SG_USAGE_DYNAMIC,
        .label = "color-vertices"
    });
    sg_shader color_shd = sg_make_shader(sg_shader_desc{
        .vs = {
            .source =
            "#version 330\n"
            "uniform mat4 mvp;\n"
            "layout(location=0) in vec2 position;\n"
            "layout(location=1) in vec4 color;\n"
            "out vec4 outColor;\n"
            "void main() {\n"
            "  gl_Position = mvp * vec4(position.xy, 0, 1);\n"
            "  outColor = color;\n"
            "}\n",
            .uniform_blocks = {
                {
                    .size = sizeof(hmm_mat4),
                    .uniforms = {
                        {.name="mvp", .type=SG_UNIFORMTYPE_MAT4}
                    }
                }
            },
        },
        .fs = {
            .source =
            "#version 330\n"
            "in vec4 outColor;\n"
            "out vec4 frag_color;\n"
            "void main() {\n"
            "  frag_color = outColor;\n"
            "}\n",
        }
    });
    drawstate[1].pip = sg_make_pipeline(sg_pipeline_desc{
        .shader = color_shd,
        .layout = {
            .attrs = {
                {.format = SG_VERTEXFORMAT_FLOAT2},
                {.format = SG_VERTEXFORMAT_UBYTE4N},
            }
        },
        .colors = {
            {
                .write_mask = SG_COLORMASK_RGB,
                .blend = {
                    .enabled = true,
                    .src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA,
                    .dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
                },
            },
        },
        .index_type = SG_INDEXTYPE_UINT16,
        .label = "color-pipeline",
    });
}

static void quit_cb(struct tray_menu *item) {
    tray_exit();
}

static void init() {
    mapstate.d2rProcess = new D2RProcess;
    HWND hwnd = (HWND)sapp_win32_get_hwnd();
    ShowWindow(hwnd, SW_HIDE);

    DWORD style = WS_POPUP;
    DWORD exStyle = WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW;
    SetWindowLong(hwnd, GWL_STYLE, style);
    SetWindowLong(hwnd, GWL_EXSTYLE, exStyle);
    SetLayeredWindowAttributes(hwnd, 0, cfg->alpha, LWA_COLORKEY | LWA_ALPHA);
    SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG)CreateSolidBrush(0));
    InvalidateRect(hwnd, nullptr, TRUE);

    static tray_menu menu[] = {
        {.text = (char*)"Quit", .cb = quit_cb},
        {.text = nullptr}
    };

    static tray tmenu = {
        .icon = "D2RMH.exe",
        .menu = menu,
    };
    tray_init(&tmenu);

    d2MapInit(cfg->d2Path.c_str());
    initData();
    initSokol();

    ShowWindow(hwnd, SW_SHOW);
}

static ULONGLONG nextTick = 0, nextSearchTick = 0;

static void updateWindowPosition() {
    if (!mapstate.currMap) { return; }
    int x0 = mapstate.currMap->cropX, y0 = mapstate.currMap->cropY, x1 = mapstate.currMap->cropX2,
        y1 = mapstate.currMap->cropY2;
    int width = x1 - x0;
    int height = y1 - y0;

    HWND hwnd = (HWND)sapp_win32_get_hwnd();
    auto windowSize = (int)lroundf(cfg->scale * (float)(width + height) * 0.75) + 8;
    auto d2rhwnd = (HWND)mapstate.d2rProcess->hwnd();
    RECT rc;
    if (GetClientRect(d2rhwnd, &rc)) {
        POINT pt = {rc.left, rc.top};
        ClientToScreen(d2rhwnd, &pt);
        rc.left = pt.x;
        rc.top = pt.y;
        pt = {rc.right, rc.bottom};
        ClientToScreen(d2rhwnd, &pt);
        rc.right = pt.x;
        rc.bottom = pt.y;
    } else {
        HMONITOR hm = MonitorFromPoint(POINT{1, 1}, MONITOR_DEFAULTTONEAREST);
        MONITORINFO mi;
        mi.cbSize = sizeof(MONITORINFO);
        GetMonitorInfo(hm, &mi);
        rc = mi.rcWork;
    }
    auto bear = 16;
    if (windowSize + bear * 2 > rc.right - rc.left) {
        windowSize = rc.right - rc.left - bear * 2;
    }
    if (windowSize / 2 + bear * 2 > rc.bottom - rc.top) {
        windowSize = (rc.bottom - rc.top - bear * 2) * 2;
    }
    float w, h;
    switch (cfg->position) {
    case 0:
        MoveWindow(hwnd, rc.left + bear, rc.top + bear, windowSize, windowSize / 2, FALSE);
        w = (float)windowSize;
        h = w / 2;
        break;
    case 1:
        MoveWindow(hwnd, rc.right - windowSize - bear, rc.top + bear, windowSize, windowSize / 2, FALSE);
        w = (float)windowSize;
        h = w / 2;
        break;
    default:
        MoveWindow(hwnd, rc.left + bear, rc.top + bear, rc.right - rc.left - bear * 2, rc.bottom - rc.top - bear * 2, FALSE);
        w = (float)(rc.right - rc.left - bear * 2);
        h = (float)(rc.bottom - rc.top - bear * 2);
        break;
    }
    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    skstate.baseMapMVP = HMM_Scale(HMM_Vec3(cfg->scale, cfg->scale, 1.f)) * HMM_Orthographic(-w / 2, w / 2, h / 2, -h / 2, -1, 1)
        * skstate.baseMVP;
    skstate.mapMVP = skstate.baseMapMVP * HMM_Translate(HMM_Vec3(skstate.offsetX, skstate.offsetY, 0));
}

static void drawLineBuild(DrawVertex *verticesOut, float x0, float y0, float x1, float y1, float width, uint32_t c) {
    auto fromPt = HMM_Vec2(x0, y0);
    auto toPt = HMM_Vec2(x1, y1);
    auto delta = toPt - fromPt;
    auto perp = HMM_Normalize(HMM_Vec2(-delta.Y, delta.X)) * width * .5f;
    auto A = fromPt + perp;
    auto B = fromPt - perp;
    auto C = toPt - perp;
    auto D = toPt + perp;
    DrawVertex vertices[] = {
        {A.X, A.Y, c},
        {B.X, B.Y, c},
        {C.X, C.Y, c},
        {D.X, D.Y, c},
    };
    memcpy(verticesOut, vertices, sizeof(vertices));
}

static void updatePlayerPos(uint16_t posX, uint16_t posY) {
    mapstate.currPosX = posX;
    mapstate.currPosY = posY;

    int idx = drawstate[1].count - 2 - int(skstate.lineEnds.size() * 2);
    if (idx >= 0) {
        int x0 = mapstate.currMap->cropX, y0 = mapstate.currMap->cropY, x1 = mapstate.currMap->cropX2,
            y1 = mapstate.currMap->cropY2;
        auto originX = mapstate.currMap->levelOrigin.x, originY = mapstate.currMap->levelOrigin.y;
        posX -= originX + x0;
        posY -= originY + y0;
        auto oxf = float(posX) - float(x1 - x0) * .5f;
        auto oyf = float(posY) - float(y1 - y0) * .5f;
        if (cfg->mapCentered) {
            skstate.offsetX = -oxf;
            skstate.offsetY = -oyf;
        } else {
            skstate.offsetX = skstate.offsetY = 0;
        }
        auto transMVP = HMM_Scale(HMM_Vec3(cfg->scale, cfg->scale, 1.f)) * HMM_Translate(HMM_Vec3(skstate.offsetX, skstate.offsetY, 0));
        skstate.textMVP = skstate.baseMVP * transMVP;
        skstate.mapMVP = skstate.baseMapMVP * transMVP;

        for (auto &le: skstate.lineEnds) {
            if (cfg->fullLine) {
                auto line = HMM_Vec2(le.x, le.y) - HMM_Vec2(oxf, oyf);
                auto len = HMM_Length(line);
                auto sx = oxf + line.X / len * 8.f;
                auto sy = oyf + line.Y / len * 8.f;
                auto ex = le.x - line.X / len * 8.f;
                auto ey = le.y - line.Y / len * 8.f;
                if (len < 17.f) {
                    ex = sx; ey = sy;
                }
                drawLineBuild(skstate.vertices + 4 * idx, sx, sy, ex, ey, 1.5f, cfg->lineColor);
                ++idx;
                DrawVertex vertices[] = {
                    {0, 0, 0},
                    {0, 0, 0},
                    {0, 0, 0},
                    {0, 0, 0},
                };
                memcpy(skstate.vertices + 4 * idx, &vertices, sizeof(vertices));
                ++idx;
            } else {
                const float mlen = 78.f;
                const float gap = 12.f;
                float sx, sy, ex, ey;
                auto line = HMM_Vec2(le.x, le.y) - HMM_Vec2(oxf, oyf);
                auto len = HMM_Length(line);
                sx = oxf + line.X / len * 8.f;
                sy = oyf + line.Y / len * 8.f;
                if (len > mlen) {
                    ex = oxf + line.X / len * (mlen - gap);
                    ey = oyf + line.Y / len * (mlen - gap);
                } else if (len > gap) {
                    ex = le.x - line.X / len * gap;
                    ey = le.y - line.Y / len * gap;
                } else {
                    ex = sx;
                    ey = sy;
                }

                const float angle = 35.f;
                /* Draw the line */
                drawLineBuild(skstate.vertices + 4 * idx, sx, sy, ex, ey, 1.5f, cfg->lineColor);
                ++idx;

                /* Draw the dot */
                if (ex == sx) {
                    DrawVertex vertices[] = {
                        {0, 0, 0},
                        {0, 0, 0},
                        {0, 0, 0},
                        {0, 0, 0},
                    };
                    memcpy(skstate.vertices + 4 * idx, &vertices, sizeof(vertices));
                } else {
                    ex += line.X / len * gap;
                    ey += line.Y / len * gap;
                    auto c = cfg->lineColor;
                    DrawVertex vertices[] = {
                        {ex - 3, ey - 3, c},
                        {ex + 1.5f, ey - 1.5f, c},
                        {ex + 3, ey + 3, c},
                        {ex - 1.5f, ey + 1.5f, c},
                    };
                    memcpy(skstate.vertices + 4 * idx, &vertices, sizeof(vertices));
                }
                ++idx;
            }
        }
        auto c1 = cfg->playerOuterColor, c2 = cfg->playerInnerColor;
        DrawVertex vertices[] = {
            {oxf - 4, oyf - 4, c1},
            {oxf + 4, oyf - 4, c1},
            {oxf + 4, oyf + 4, c1},
            {oxf - 4, oyf + 4, c1},
            {oxf - 2, oyf - 2, c2},
            {oxf + 2, oyf - 2, c2},
            {oxf + 2, oyf + 2, c2},
            {oxf - 2, oyf + 2, c2},
        };
        memcpy(skstate.vertices + 4 * (drawstate[1].count - 2), &vertices, sizeof(vertices));
        sg_update_buffer(drawstate[1].bind.vertex_buffers[0],
                         sg_range{skstate.vertices, sizeof(DrawVertex) * 4 * drawstate[1].count});
    }
}

static void checkForUpdate() {
    auto ticks = GetTickCount64();
    bool needUpdate = ticks >= nextTick;
    if (needUpdate) {
        nextTick = ticks + 50;
        bool searchProcess = ticks >= nextSearchTick;
        if (searchProcess) {
            nextSearchTick = ticks + 1000;
        }
        mapstate.d2rProcess->updateData(searchProcess);
        if (!mapstate.d2rProcess->available()) {
            mapstate.currMap = nullptr;
            drawstate[0].enable = false;
            drawstate[1].enable = false;
            return;
        }
    }
    auto seed = mapstate.d2rProcess->seed();
    auto difficulty = mapstate.d2rProcess->difficulty();
    bool changed = false;
    if (!mapstate.session || seed != mapstate.currSeed || difficulty != mapstate.currDifficulty) {
        mapstate.currSeed = seed;
        mapstate.currDifficulty = difficulty;
        changed = true;
        delete mapstate.session;
        mapstate.session = new Session(seed, difficulty);
    }
    auto levelId = mapstate.d2rProcess->levelId();
    if (changed || levelId != mapstate.currLevelId) {
        mapstate.currLevelId = levelId;
        drawstate[0].enable = drawstate[1].enable = levelId > 0;
        if (levelId <= 0) {
            drawstate[0].enable = drawstate[1].enable = false;
            return;
        }
        mapstate.currMap = mapstate.session->getMap(levelId);
        if (!mapstate.currMap) {
            drawstate[0].enable = drawstate[1].enable = false;
            return;
        }
        drawstate[0].count = 1;
        sg_destroy_image(drawstate[0].bind.fs_images[0]);
        drawstate[0].bind.fs_images[0] = sg_alloc_image();
        int x0 = mapstate.currMap->cropX, y0 = mapstate.currMap->cropY, x1 = mapstate.currMap->cropX2,
            y1 = mapstate.currMap->cropY2;
        int width = x1 - x0;
        int height = y1 - y0;
        auto totalWidth = mapstate.currMap->totalWidth;
        auto *pixels = new uint32_t[width * height];
        auto *ptr = pixels;
        for (int y = y0; y < y1; ++y) {
            int idx = y * totalWidth + x0;
            for (int x = x0; x < x1; ++x) {
                auto clr = mapstate.mapColor[mapstate.currMap->map[idx++] & 1];
                *ptr++ = clr;
            }
        }
        needUpdate = true;
        sg_init_image(drawstate[0].bind.fs_images[0], sg_image_desc{
            .width = width,
            .height = height,
            .pixel_format = SG_PIXELFORMAT_RGBA8,
            .min_filter = SG_FILTER_LINEAR,
            .mag_filter = SG_FILTER_LINEAR,
            .data = {
                .subimage = {{{
                                  .ptr = pixels,
                                  .size = (size_t)(width * height * 4),
                              }}}
            }
        });
        delete[] pixels;
        auto widthf = (float)width * .5f;
        auto heightf = (float)height * .5f;
        const float vertices[] = {
            -widthf, -heightf, 0, 0,
            widthf, -heightf, 1, 0,
            widthf, heightf, 1, 1,
            -widthf, heightf, 0, 1,
        };
        sg_update_buffer(drawstate[0].bind.vertex_buffers[0], SG_RANGE(vertices));

        skstate.mapObjs.clear();
        skstate.lineEnds.clear();
        const std::set<int> *guides = nullptr;
        {
            auto gdite = mapstate.guides.find(levelId);
            if (gdite != mapstate.guides.end()) {
                guides = &gdite->second;
            }
        }
        auto originX = mapstate.currMap->levelOrigin.x, originY = mapstate.currMap->levelOrigin.y;
        for (auto &p: mapstate.currMap->adjacentLevels) {
            auto ite = mapstate.levels.find(p.first);
            if (ite == mapstate.levels.end()) { continue; }
            float px, py;
            if (p.second.exits.empty()) {
                px = float(p.second.levelOrigin.x - originX - x0) - widthf;
                py = float(p.second.levelOrigin.y - originY - y0) - heightf;
            } else {
                auto &e = p.second.exits[0];
                px = float(e.x - originX - x0) - widthf;
                py = float(e.y - originY - y0) - heightf;
            }
            std::string name = mapstate.strings[ite->second][mapstate.language];
            /* Check for TalTombs */
            if (p.first >= 66 && p.first <= 72) {
                auto *m = mapstate.session->getMap(p.first);
                if (m->objects.find(152) != m->objects.end()) {
                    name = ">>> " + name + " <<<";
                    skstate.lineEnds.emplace_back(LinePoint{px, py});
                }
            }
            skstate.mapObjs.emplace_back(MapObject{px, py,
                                                   mapstate.objColors[TypePortal],
                                                   std::move(name)});
            if (guides && (*guides).find(p.first) != (*guides).end()) {
                skstate.lineEnds.emplace_back(LinePoint{px, py});
            }
        }
        std::map<uint32_t, std::vector<Point>> *objs[2] = {&mapstate.currMap->objects, &mapstate.currMap->npcs};
        for (int i = 0; i < 2; ++i) {
            for (auto &p: *objs[i]) {
                auto ite = mapstate.objects[i].find(p.first);
                if (ite == mapstate.objects[i].end()) { continue; }
                for (auto &pt: p.second) {
                    auto ptx = float(pt.x - originX - x0) - widthf;
                    auto pty = float(pt.y - originY - y0) - heightf;
                    auto tp = ite->second.type;
                    switch (tp) {
                    case TypeWayPoint:
                    case TypeQuest:
                    case TypePortal:
                    case TypeChest:
                    case TypeShrine:
                    case TypeWell: {
                        std::string name = mapstate.strings[ite->second.name][mapstate.language];
                        skstate.mapObjs.emplace_back(MapObject{ptx, pty,
                                                               mapstate.objColors[tp],
                                                               std::move(name)});
                        if (guides && (*guides).find(p.first | (0x10000 * (i + 1))) != (*guides).end()) {
                            skstate.lineEnds.emplace_back(LinePoint{ptx, pty});
                        }
                        break;
                    }
                    default:
                        break;
                    }
                }
            }
        }
        auto drawCount = 2 + skstate.mapObjs.size() + skstate.lineEnds.size() * 2;
        drawstate[1].count = drawCount;
        if (drawCount > skstate.drawArrayCapacity) {
            skstate.drawArrayCapacity = drawCount;
            delete[] skstate.indices;
            skstate.indices = new uint16_t[6 * drawCount];
            delete[] skstate.vertices;
            skstate.vertices = new DrawVertex[4 * drawCount];
        }
        auto *indices = skstate.indices;
        int index = 0;
        for (size_t i = 0; i < drawCount; ++i) {
            uint16_t base = i * 4;
            indices[index++] = base;
            indices[index++] = base + 1;
            indices[index++] = base + 2;
            indices[index++] = base;
            indices[index++] = base + 2;
            indices[index++] = base + 3;
        }
        sg_update_buffer(drawstate[1].bind.index_buffer, sg_range{indices, sizeof(uint16_t) * 6 * drawCount});

        auto posX = mapstate.d2rProcess->posX(), posY = mapstate.d2rProcess->posY();
        auto *vertices2 = skstate.vertices;
        index = 0;
        for (auto &e: skstate.mapObjs) {
            vertices2[index++] = {e.x - 4, e.y - 4, e.color};
            vertices2[index++] = {e.x + 4, e.y - 4, e.color};
            vertices2[index++] = {e.x + 4, e.y + 4, e.color};
            vertices2[index++] = {e.x - 4, e.y + 4, e.color};
        }
        updatePlayerPos(posX, posY);
    } else {
        if (mapstate.currMap) {
            auto posX = mapstate.d2rProcess->posX(), posY = mapstate.d2rProcess->posY();
            if (posX != mapstate.currPosX || posY != mapstate.currPosY) {
                updatePlayerPos(posX, posY);
            }
        }
    }
    if (needUpdate) {
        updateWindowPosition();
    }
}

static void frame() {
    if (tray_loop(0) != 0) {
        sapp_request_quit();
        return;
    }
    checkForUpdate();
    sg_begin_default_pass(&pass_action, sapp_width(), sapp_height());
    if (mapstate.d2rProcess->available()) {
        bool show;
        switch (cfg->show) {
        case 0:
            show = !mapstate.d2rProcess->mapEnabled();
            break;
        case 1:
            show = mapstate.d2rProcess->mapEnabled();
            break;
        default:
            show = true;
            break;
        }
        if (show) {
            for (auto &s: drawstate) {
                if (!s.enable) { continue; }
                sg_apply_pipeline(s.pip);
                sg_apply_bindings(&s.bind);
                sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, SG_RANGE(skstate.mapMVP));
                sg_draw(0, s.count * 6, 1);
            }
            if (skstate.font != FONS_INVALID) {
                fonsClearState(skstate.fonsCtx);
                sgl_defaults();
                sgl_matrix_mode_projection();
                float w = sapp_widthf() * .5f, h = sapp_heightf() * .5f;
                sgl_ortho(-w, w, h, -h, -1, 1);
                fonsSetFont(skstate.fonsCtx, skstate.font);
                fonsSetSize(skstate.fonsCtx, cfg->fontSize * skstate.dpiScale);
                fonsSetColor(skstate.fonsCtx, cfg->textColor);
                fonsSetBlur(skstate.fonsCtx, .5f);
                fonsSetAlign(skstate.fonsCtx, FONS_ALIGN_CENTER);
                for (auto &t: skstate.mapObjs) {
                    if (!t.str.empty()) {
                        auto coord = skstate.textMVP * HMM_Vec4(t.x, t.y, 0, 1);
                        fonsDrawText(skstate.fonsCtx, coord.X, coord.Y - 8, t.str.c_str(), nullptr);
                    }
                }
                sfons_flush(skstate.fonsCtx);
                sgl_draw();
            }
        }
    }
    sg_end_pass();
    sg_commit();
}

static void cleanup() {
    sfons_destroy(skstate.fonsCtx);
    if (skstate.fontBuf) {
        delete[] skstate.fontBuf;
        skstate.fontBuf = nullptr;
    }
    if (skstate.indices) {
        delete[] skstate.indices;
        skstate.indices = nullptr;
    }
    if (skstate.vertices) {
        delete[] skstate.vertices;
        skstate.vertices = nullptr;
    }
    sgl_shutdown();
    sg_shutdown();
    delete mapstate.session;
    delete mapstate.d2rProcess;
}

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

sapp_desc sokol_main(int argc, char *argv[]) {
    loadCfg();
    mapstate.language = lngFromString(cfg->language);
    return sapp_desc{
        .init_cb = init,
        .frame_cb = frame,
        .cleanup_cb = cleanup,
        .width = 0,
        .height = 0,
        .sample_count = 4,
        .swap_interval = 1,
        .alpha = true,
        .window_title = "D2RMH",
        .gl_force_gles2 = true,
    };
}

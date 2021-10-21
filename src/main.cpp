#include "d2map.h"
#include "session.h"
#include "d2rprocess.h"

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

#include <iostream>
#include <fstream>
#include <vector>

static sg_pass_action pass_action = {};
static struct {
    bool enable = false;
    sg_pipeline pip = {};
    sg_bindings bind = {};
    int count = 0;
} drawstate[2];

enum {
    WindowSize = 500,
};

struct MapExit {
    float x, y;
    const char *str;
};

static struct {
    hmm_mat4 mapMVP = {};
    float dpiScale = .0f;
    FONScontext *fonsCtx = nullptr;
    unsigned char *fontBuf = nullptr;
    size_t fontBufSize = 0;
    int font = FONS_INVALID;
    std::vector<MapExit> mapExits;

    uint16_t *indices = nullptr;
    float *vertices = nullptr;
    int drawArrayCapacity = 0;
} skstate;

enum EObjType {
    TypeNone,
    TypeWP,
};

static struct {
    Session *session = nullptr;
    CollisionMap *currMap = nullptr;
    D2RProcess *d2rProcess = nullptr;
    int lastMapId = -1;
    uint32_t lastSeed = 0;
    uint8_t lastDifficulty = 0xFF;
    uint16_t lastPosX = 0, lastPosY = 0;
#define RGBA(r, g, b, a) (uint32_t(r) | (uint32_t(g) << 8) | (uint32_t(b) << 8) | (uint32_t(a) << 24))
    std::map<int, uint32_t> MapColor = {
        {0, RGBA(50, 50, 50, 255)},
        {2, RGBA(10, 51, 23, 255)},
        {3, RGBA(255, 0, 255, 255)},
        {4, RGBA(0, 255, 255, 255)},
        {6, RGBA(80, 51, 33, 255)},
        {7, RGBA(180, 180, 180, 255)},
        {8, RGBA(180, 180, 180, 255)},
        {16, RGBA(50, 50, 50, 255)},
        {17, RGBA(255, 51, 255, 255)},
        {19, RGBA(0, 51, 255, 255)},
        {20, RGBA(70, 51, 41, 255)},
        {21, RGBA(255, 0, 255, 255)},
        {23, RGBA(0, 0, 255, 255)},
        {33, RGBA(0, 0, 255, 255)},
        {37, RGBA(50, 51, 23, 255)},
        {39, RGBA(20, 11, 33, 255)},
        {53, RGBA(10, 11, 43, 255)},
    };
#undef RGBA
    std::map<int, EObjType> ObjectType = {
        {119, TypeWP},
        {145, TypeWP},
        {156, TypeWP},
        {157, TypeWP},
        {237, TypeWP},
        {238, TypeWP},
        {288, TypeWP},
        {323, TypeWP},
        {324, TypeWP},
        {398, TypeWP},
        {402, TypeWP},
        {429, TypeWP},
        {494, TypeWP},
        {496, TypeWP},
        {511, TypeWP},
        {539, TypeWP},
   };
} mapstate;

static int round_pow2(float v) {
    uint32_t vi = ((uint32_t) v) - 1;
    for (uint32_t i = 0; i < 5; i++) {
        vi |= (vi >> (1<<i));
    }
    return (int) (vi + 1);
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
    ifs.read((char*)buf, size);
    ifs.close();
    return buf;
}

static void init() {
    mapstate.d2rProcess = new D2RProcess;
    HWND hwnd = (HWND)sapp_win32_get_hwnd();
    DWORD style = WS_POPUP;
    DWORD exStyle = WS_EX_LAYERED | WS_EX_TRANSPARENT;
    SetWindowLong(hwnd, GWL_STYLE, style);
    SetWindowLong(hwnd, GWL_EXSTYLE, exStyle);
    SetLayeredWindowAttributes(hwnd, 0, 185, LWA_COLORKEY | LWA_ALPHA);

    HMONITOR hm = MonitorFromPoint(POINT {1, 1}, MONITOR_DEFAULTTONEAREST);
    MONITORINFO mi;
    mi.cbSize = sizeof(MONITORINFO);
    GetMonitorInfo(hm, &mi);
    ShowWindow(hwnd, SW_SHOW);
    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

    sg_setup(sg_desc {
        .context = sapp_sgcontext()
    });
    sgl_setup(sgl_desc_t {
        .sample_count = sapp_sample_count()
    });

    skstate.dpiScale = sapp_dpi_scale();
    const int atlas_dim = round_pow2(512.0f * skstate.dpiScale);
    skstate.fonsCtx = sfons_create(atlas_dim, atlas_dim, FONS_ZERO_TOPLEFT);
    skstate.fontBuf = loadFromFile("normal.ttf", skstate.fontBufSize);
    if (skstate.fontBuf) {
        skstate.font = fonsAddFontMem(skstate.fonsCtx, "normal", skstate.fontBuf, skstate.fontBufSize, 0);
    }

    pass_action = sg_pass_action {
        .colors = {
            {.action = SG_ACTION_CLEAR, .value = {0, 0, 0, .5}}
        },
    };
    const uint16_t indices[] = {
         0,  1,  2,  0,  2,  3,
    };
    drawstate[0].bind.index_buffer = sg_make_buffer(sg_buffer_desc {
        .type = SG_BUFFERTYPE_INDEXBUFFER,
        .data = SG_RANGE(indices),
        .label = "draw-indices"
    });
    drawstate[0].bind.vertex_buffers[0] = sg_make_buffer(sg_buffer_desc{
        .size = sizeof(float) * 5 * 4,
        .usage = SG_USAGE_DYNAMIC,
        .label = "draw-vertices"
    });
    sg_shader image_shd = sg_make_shader(sg_shader_desc {
        .vs = {
            .source =
            "#version 330\n"
            "uniform mat4 mvp;\n"
            "layout(location=0) in vec4 position;\n"
            "layout(location=1) in vec2 texcoord0;\n"
            "out vec2 uv;\n"
            "void main() {\n"
            "  gl_Position = mvp * position;\n"
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
            .images = {{ .name="tex", .image_type=SG_IMAGETYPE_2D }},
        }
    });
    drawstate[0].pip = sg_make_pipeline(sg_pipeline_desc {
        .shader = image_shd,
        .layout = {
            .attrs = {
                { .format = SG_VERTEXFORMAT_FLOAT3 },
                { .format = SG_VERTEXFORMAT_FLOAT2 },
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

    drawstate[1].bind.index_buffer = sg_make_buffer(sg_buffer_desc {
        .size = sizeof(uint16_t) * 6 * 96,
        .type = SG_BUFFERTYPE_INDEXBUFFER,
        .usage = SG_USAGE_DYNAMIC,
        .label = "color-indices"
    });
    drawstate[1].bind.vertex_buffers[0] = sg_make_buffer(sg_buffer_desc{
        .size = sizeof(float) * 6 * 4 * 96,
        .usage = SG_USAGE_DYNAMIC,
        .label = "color-vertices"
    });
    sg_shader color_shd = sg_make_shader(sg_shader_desc {
        .vs = {
            .source =
            "#version 330\n"
            "uniform mat4 mvp;\n"
            "layout(location=0) in vec4 position;\n"
            "layout(location=1) in vec3 color;\n"
            "out vec4 outColor;\n"
            "void main() {\n"
            "  gl_Position = mvp * position;\n"
            "  outColor = vec4(color.rgb, 1);\n"
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
    drawstate[1].pip = sg_make_pipeline(sg_pipeline_desc {
        .shader = color_shd,
        .layout = {
            .attrs = {
                { .format = SG_VERTEXFORMAT_FLOAT3 },
                { .format = SG_VERTEXFORMAT_FLOAT3 },
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

static ULONGLONG nextTick = 0, nextSearchTick = 0;

static void updateWindowPosition() {
    if (!mapstate.currMap) { return; }
    int x0 = mapstate.currMap->cropX, y0 = mapstate.currMap->cropY, x1 = mapstate.currMap->cropX2, y1 = mapstate.currMap->cropY2;
    int width = x1 - x0;
    int height = y1 - y0;

    HWND hwnd = (HWND)sapp_win32_get_hwnd();
    auto windowSize = (width + height) * 3 / 4;
    auto d2rhwnd = (HWND)mapstate.d2rProcess->hwnd();
    RECT rc;
    POINT pt;
    if (GetClientRect(d2rhwnd, &rc)) {
        pt = {rc.right, rc.top};
        ClientToScreen(d2rhwnd, &pt);
    } else {
        HMONITOR hm = MonitorFromPoint(POINT {1, 1}, MONITOR_DEFAULTTONEAREST);
        MONITORINFO mi;
        mi.cbSize = sizeof(MONITORINFO);
        GetMonitorInfo(hm, &mi);
        pt = { mi.rcWork.right, mi.rcWork.top };
    }
    MoveWindow(hwnd, pt.x - windowSize, pt.y, windowSize, windowSize / 2, FALSE);
    auto w = (float)windowSize;
    auto h = w / 2;
    skstate.mapMVP = HMM_Orthographic(-w / 2, w / 2, h / 2, -h / 2, -1, 1)
        * (HMM_Scale(HMM_Vec3(1, 0.5, 1)) * HMM_Rotate(45.f, HMM_Vec3(0, 0, 1)));
}

static void updatePlayerPos(uint16_t posX, uint16_t posY) {
    mapstate.lastPosX = posX;
    mapstate.lastPosY = posY;

    int x0 = mapstate.currMap->cropX, y0 = mapstate.currMap->cropY, x1 = mapstate.currMap->cropX2,
        y1 = mapstate.currMap->cropY2;
    auto originX = mapstate.currMap->levelOrigin.x, originY = mapstate.currMap->levelOrigin.y;
    posX -= originX + x0;
    posY -= originY + y0;
    auto oxf = float(posX) - float(x1 - x0) * .5f;
    auto oyf = float(posY) - float(y1 - y0) * .5f;

    float vertices[] = {
        oxf - 3, oyf - 3, .0f, .4f, .4f, 1,
        oxf + 3, oyf - 3, .0f, .4f, .4f, 1,
        oxf + 3, oyf + 3, .0f, .4f, .4f, 1,
        oxf - 3, oyf + 3, .0f, .4f, .4f, 1,
    };
    memcpy(skstate.vertices + 6 * 4 * (drawstate[1].count - 1), &vertices, sizeof(vertices));
    sg_update_buffer(drawstate[1].bind.vertex_buffers[0], sg_range { skstate.vertices, sizeof(float) * 6 * 4 * drawstate[1].count });
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
    if (!mapstate.session || seed != mapstate.lastSeed || difficulty != mapstate.lastDifficulty) {
        mapstate.lastSeed = seed;
        mapstate.lastDifficulty = difficulty;
        changed = true;
        delete mapstate.session;
        mapstate.session = new Session(seed, difficulty);
    }
    auto levelId = mapstate.d2rProcess->levelId();
    if (changed || levelId != mapstate.lastMapId) {
        mapstate.lastMapId = levelId;
        drawstate[0].enable = drawstate[1].enable = levelId > 0;
        if (levelId <= 0) { drawstate[0].enable = drawstate[1].enable = false; return; }
        mapstate.currMap = mapstate.session->getMap(levelId);
        if (!mapstate.currMap) { drawstate[0].enable = drawstate[1].enable = false; return; }
        drawstate[0].count = 1;
        sg_destroy_image(drawstate[0].bind.fs_images[0]);
        drawstate[0].bind.fs_images[0] = sg_alloc_image();
        int x0 = mapstate.currMap->cropX, y0 = mapstate.currMap->cropY, x1 = mapstate.currMap->cropX2, y1 = mapstate.currMap->cropY2;
        int width = x1 - x0;
        int height = y1 - y0;
        auto *pixels = new uint32_t[width * height];
        auto *ptr = pixels;
        for (int y = y0; y < y1; ++y) {
            for (int x = x0; x < x1; ++x) {
                auto ite = mapstate.MapColor.find(mapstate.currMap->map[y][x]);
                *ptr++ = ite == mapstate.MapColor.end() ? 0u : ite->second;
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
            -widthf, -heightf, .0f, 0, 0,
            widthf, -heightf, .0f, 1, 0,
            widthf, heightf, .0f, 1, 1,
            -widthf, heightf, .0f, 0, 1,
        };
        sg_update_buffer(drawstate[0].bind.vertex_buffers[0], SG_RANGE(vertices));

        skstate.mapExits.clear();
        int count = 0;
        auto originX = mapstate.currMap->levelOrigin.x, originY = mapstate.currMap->levelOrigin.y;
        for (auto &p: mapstate.currMap->adjacentLevels) {
            if (p.second.exits.empty()) { ++count; continue; }
            auto px = float(p.second.exits[0].x - originX - x0) - widthf;
            auto py = float(p.second.exits[0].y - originY - y0) - heightf;
            skstate.mapExits.emplace_back(MapExit{px, py, count ? "out" : "in"});
            ++count;
        }
        struct ObjectData {
            float x, y;
            float r, g, b;
        };
        std::vector<ObjectData> objects;
        for (auto &p: mapstate.currMap->objects) {
            auto ite = mapstate.ObjectType.find(p.first);
            if (ite == mapstate.ObjectType.end()) { continue; }
            for (auto &pt: p.second) {
                auto ptx = float(pt.x - originX - x0) - widthf;
                auto pty = float(pt.y - originY - y0) - heightf;
                switch (ite->second) {
                case TypeWP:
                    objects.emplace_back(ObjectData {
                        ptx, pty, 1, 1, 0
                    });
                    break;
                default:
                    break;
                }
            }
        }
        auto drawCount = 1 + skstate.mapExits.size() + objects.size();
        drawstate[1].count = drawCount;
        if (drawCount > skstate.drawArrayCapacity) {
            skstate.drawArrayCapacity = drawCount;
            delete[] skstate.indices;
            skstate.indices = new uint16_t[6 * drawCount];
            delete[] skstate.vertices;
            skstate.vertices = new float[6 * 4 * drawCount];
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
        sg_update_buffer(drawstate[1].bind.index_buffer, sg_range { indices, sizeof(uint16_t) * 6 * drawCount });

        auto posX = mapstate.d2rProcess->posX(), posY = mapstate.d2rProcess->posY();
        auto *vertices2 = skstate.vertices;
        index = 0;
        for (auto &e: skstate.mapExits) {
            vertices2[index++] = e.x - 4;
            vertices2[index++] = e.y - 4;
            vertices2[index++] = 0;
            vertices2[index++] = 1;
            vertices2[index++] = .6f;
            vertices2[index++] = 1;
            vertices2[index++] = e.x + 4;
            vertices2[index++] = e.y - 4;
            vertices2[index++] = 0;
            vertices2[index++] = 1;
            vertices2[index++] = .6f;
            vertices2[index++] = 1;
            vertices2[index++] = e.x + 4;
            vertices2[index++] = e.y + 4;
            vertices2[index++] = 0;
            vertices2[index++] = 1;
            vertices2[index++] = .6f;
            vertices2[index++] = 1;
            vertices2[index++] = e.x - 4;
            vertices2[index++] = e.y + 4;
            vertices2[index++] = 0;
            vertices2[index++] = 1;
            vertices2[index++] = .6f;
            vertices2[index++] = 1;
        }
        for (auto &e: objects) {
            vertices2[index++] = e.x - 4;
            vertices2[index++] = e.y - 4;
            vertices2[index++] = 0;
            vertices2[index++] = e.r;
            vertices2[index++] = e.g;
            vertices2[index++] = e.b;
            vertices2[index++] = e.x + 4;
            vertices2[index++] = e.y - 4;
            vertices2[index++] = 0;
            vertices2[index++] = e.r;
            vertices2[index++] = e.g;
            vertices2[index++] = e.b;
            vertices2[index++] = e.x + 4;
            vertices2[index++] = e.y + 4;
            vertices2[index++] = 0;
            vertices2[index++] = e.r;
            vertices2[index++] = e.g;
            vertices2[index++] = e.b;
            vertices2[index++] = e.x - 4;
            vertices2[index++] = e.y + 4;
            vertices2[index++] = 0;
            vertices2[index++] = e.r;
            vertices2[index++] = e.g;
            vertices2[index++] = e.b;
        }
        updatePlayerPos(posX, posY);
    } else {
        if (mapstate.currMap) {
            auto posX = mapstate.d2rProcess->posX(), posY = mapstate.d2rProcess->posY();
            if (posX != mapstate.lastPosX || posY != mapstate.lastPosY) {
                updatePlayerPos(posX, posY);
            }
        }
    }
    if (needUpdate) {
        updateWindowPosition();
    }
}

static void frame() {
    checkForUpdate();
    sg_begin_default_pass(&pass_action, sapp_width(), sapp_height());
    if (mapstate.d2rProcess->available() && !mapstate.d2rProcess->mapEnabled()) {
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
            sgl_load_matrix((const float *)&skstate.mapMVP.Elements[0][0]);
            fonsSetFont(skstate.fonsCtx, skstate.font);
            fonsSetSize(skstate.fonsCtx, 24.0f * skstate.dpiScale);
            fonsSetColor(skstate.fonsCtx, sfons_rgba(224, 224, 224, 255));
            fonsSetAlign(skstate.fonsCtx, FONS_ALIGN_CENTER);
            for (auto &t: skstate.mapExits) {
                fonsDrawText(skstate.fonsCtx, t.x, t.y - 8.f, t.str, nullptr);
            }
//            fonsDrawText(skstate.fonsCtx, 10, 50, "Test Test", nullptr);
            sfons_flush(skstate.fonsCtx);
            sgl_draw();
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

sapp_desc sokol_main(int argc, char *argv[]) {
    init(".");
    return sapp_desc {
        .init_cb = init,
        .frame_cb = frame,
        .cleanup_cb = cleanup,
        .width = WindowSize * 2,
        .height = WindowSize,
        .sample_count = 4,
        .swap_interval = 1,
        .alpha = true,
        .window_title = "D2RMH",
        .icon = {
            .sokol_default = true,
        },
        .gl_force_gles2 = true,
    };
}

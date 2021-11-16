/*
 * Copyright (c) 2021 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#include "renderer.h"

#include "window.h"

#include "HandmadeMath.h"
#include <glad/glad_wgl.h>
#include <windows.h>
#include <chrono>
#include <thread>
#include <stdexcept>

typedef HGLRC (WINAPI * PFNWGLCREATECONTEXTPROC) (HDC hDc);
typedef BOOL (WINAPI * PFNWGLDELETECONTEXTPROC) (HGLRC oldContext);
typedef BOOL (WINAPI * PFNWGLMAKECURRENTPROC) (HDC hDc, HGLRC newContext);

struct RendererCtx {
    Window *owner = nullptr;

    /* WGL functions */
    HMODULE lib = nullptr;
    PFNWGLCREATECONTEXTPROC wglCreateContext = nullptr;
    PFNWGLDELETECONTEXTPROC wglDeleteContext = nullptr;
    PFNWGLMAKECURRENTPROC wglMakeCurrent = nullptr;

    /* OpenGL-Context related */
    HDC dc = nullptr;
    HGLRC glCtx = nullptr;

    std::chrono::steady_clock::time_point nextRenderTime;
    std::chrono::steady_clock::duration renderInterval = {};
    bool fpsLimit = false;

    /* Render related */
    int width = 0, height = 0;
    Texture *background = nullptr;

    float clearColor[4] = {.0, .0, .0, 1.};
};

Renderer::Renderer(Window *wnd): ctx_(new RendererCtx) {
    ctx_->owner = wnd;
    ctx_->lib = LoadLibraryW(L"opengl32.dll");
    if (!ctx_->lib) {
        throw std::runtime_error("Unable to load opengl32.dll");
    }
    ctx_->wglCreateContext = (PFNWGLCREATECONTEXTPROC)GetProcAddress(ctx_->lib, "wglCreateContext");
    ctx_->wglDeleteContext = (PFNWGLDELETECONTEXTPROC)GetProcAddress(ctx_->lib, "wglDeleteContext");
    ctx_->wglMakeCurrent = (PFNWGLMAKECURRENTPROC)GetProcAddress(ctx_->lib, "wglMakeCurrent");
    PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR), 1, PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA, 32, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0,
        24, 8, 0, PFD_MAIN_PLANE,
        0, 0, 0, 0
    };
    ctx_->dc = GetDC((HWND)wnd->hwnd());
    int choose = ChoosePixelFormat(ctx_->dc, &pfd);
    SetPixelFormat(ctx_->dc, choose, &pfd);
    ctx_->glCtx = ctx_->wglCreateContext(ctx_->dc);
    ctx_->wglMakeCurrent(ctx_->dc, ctx_->glCtx);

    gladLoadGL();
    gladLoadWGL(ctx_->dc);
    wnd->getDimension(ctx_->width, ctx_->height);
    wnd->setSizeCallback([this](int w, int h) {
        ctx_->width = w;
        ctx_->height = h;
    });
}

Renderer::~Renderer() {
    if (ctx_->glCtx) {
        ctx_->wglDeleteContext(ctx_->glCtx);
    }
    FreeLibrary(ctx_->lib);
    delete ctx_;
}

Window *Renderer::owner() {
    return ctx_->owner;
}

void Renderer::setSwapInterval(int interval) {
    wglSwapIntervalEXT(interval);
}
void Renderer::setViewport(int x, int y, int w, int h) {
    glViewport(x, y, w, h);
}
void Renderer::limitFPS(uint32_t fps) {
    if (fps) {
        ctx_->renderInterval = std::chrono::nanoseconds (1000 * 1000 * 1000) / fps;
        ctx_->fpsLimit = true;
    } else {
        ctx_->fpsLimit = false;
    }
}
void Renderer::setClearColor(float r, float g, float b, float a) {
    auto *c = ctx_->clearColor;
    c[0] = r;
    c[1] = g;
    c[2] = b;
    c[3] = a;
}
void Renderer::prepare() {
    if (ctx_->fpsLimit) {
        do {
            auto now = std::chrono::steady_clock::now();
            if (ctx_->nextRenderTime > now) {
                std::this_thread::sleep_for(ctx_->nextRenderTime - now);
                continue;
            }
            ctx_->nextRenderTime += ctx_->renderInterval;
            if (ctx_->nextRenderTime < now) { ctx_->nextRenderTime = now + ctx_->renderInterval; }
            break;
        } while(true);
    }
}
void Renderer::begin() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_MULTISAMPLE);

    auto *c = ctx_->clearColor;
    glClearColor(c[0], c[1], c[2], c[3]);
    glClear(GL_COLOR_BUFFER_BIT);
}
void Renderer::end() {
    SwapBuffers(ctx_->dc);
}
void Renderer::getDimension(int &width, int &height) const {
    width = ctx_->width;
    height = ctx_->height;
}

Shader::Shader(Shader::Type type): tp(type) {
    uint32_t stp = 0;
    switch (type) {
    case VertexShader:
        stp = GL_VERTEX_SHADER;
        break;
    case FragmentShader:
        stp = GL_FRAGMENT_SHADER;
        break;
    }
    uid = glCreateShader(stp);
}
Shader::~Shader() {
    if (uid) {
        glDeleteShader(uid);
    }
}
bool Shader::compile(const char *src) {
    if (uid == 0) {
        return false;
    }
    glShaderSource(uid, 1, &src, nullptr);
    glCompileShader(uid);
    GLint compiled = GL_FALSE;
    glGetShaderiv(uid, GL_COMPILE_STATUS, &compiled);
    if (compiled == GL_FALSE) {
        glDeleteShader(uid);
        uid = 0;
        return false;
    }
    return true;
}

ShaderProgram::~ShaderProgram() {
    if (uid) {
        glDeleteProgram(uid);
    }
}
bool ShaderProgram::compileAndLink(const char *vsSrc, const char *fsSrc) {
    Shader vShader(Shader::VertexShader), fShader(Shader::FragmentShader);
    if (!vShader.compile(vsSrc)) { return false; }
    if (!fShader.compile(fsSrc)) { return false; }
    const Shader *shaders[] = {&vShader, &fShader, nullptr};
    return link(shaders);
}
bool ShaderProgram::link(const Shader **shaders) {
    if (uid == 0 && (uid = glCreateProgram()) == 0) {
        return false;
    }
    for (const auto *s = shaders; *s != nullptr; ++s) {
        glAttachShader(uid, (uint32_t)**s);
    }
    glLinkProgram(uid);
    GLint linked = GL_FALSE;
    glGetProgramiv(uid, GL_LINK_STATUS, &linked);
    if (linked == GL_FALSE) {
        glDeleteProgram(uid);
        uid = 0;
        return false;
    }
    return true;
}
void ShaderProgram::use() const {
    glUseProgram(uid);
}
void ShaderProgram::unuse() {
    glUseProgram(0);
}
int ShaderProgram::uniformIndex(const char *name) const {
    return glGetUniformLocation(uid, name);
}
void ShaderProgram::setInt(int id, int value) {
    glUniform1i(id, value);
}
void ShaderProgram::setInt(const char *name, int value) const {
    glUseProgram(uid);
    auto id = glGetUniformLocation(uid, name);
    if (id >= 0) {
        glUniform1i(id, value);
    }
}
void ShaderProgram::setFloat(int id, float value) {
    glUniform1f(id, value);
}
void ShaderProgram::setFloat(const char *name, float value) const {
    glUseProgram(uid);
    auto id = glGetUniformLocation(uid, name);
    if (id >= 0) {
        glUniform1f(id, value);
    }
}
void ShaderProgram::setMat4(int id, const float *data) {
    glUniformMatrix4fv(id, 1, GL_FALSE, data);
}
void ShaderProgram::setMat4(const char *name, const float *data) const {
    glUseProgram(uid);
    auto id = glGetUniformLocation(uid, name);
    if (id >= 0) {
        glUniformMatrix4fv(id, 1, GL_FALSE, data);
    }
}

static int roundup(int v) {
    auto vi = uint32_t(v > 1 ? v - 1 : 1);
    for (uint32_t i = 0; i < 5; i++) {
        vi |= (vi >> (1 << i));
    }
    return int(vi + 1);
}

Texture::Texture() {
    glGenTextures(1, &uid_);
}
Texture::~Texture() {
    if (uid_) {
        glDeleteTextures(1, &uid_);
    }
}
void Texture::create(int width, int height) {
    width_ = roundup(width);
    height_ = roundup(height);
    widthReal_ = width;
    heightReal_ = height;
    glBindTexture(GL_TEXTURE_2D, uid_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_, height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
}
void Texture::setData(int width, int height, const uint32_t *data) {
    width_ = roundup(width);
    height_ = roundup(height);
    widthReal_ = width;
    heightReal_ = height;
    glBindTexture(GL_TEXTURE_2D, uid_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_, height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
}
void Texture::updateSubData(int x, int y, int width, int height, const uint32_t *data) const {
    glBindTexture(GL_TEXTURE_2D, uid_);
    glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
    int err = glGetError();
    glBindTexture(GL_TEXTURE_2D, 0);
}
void Texture::bind() const {
    glBindTexture(GL_TEXTURE_2D, uid_);
}
void Texture::unbind() {
    glBindTexture(GL_TEXTURE_2D, 0);
}

struct VertexAttribPointerLayout {
    int size;
    uint32_t type;
    uint8_t normalized;
    const void *pointer;
};

struct PipelineCtx {
    const void *host = nullptr;
    bool renderToTarget = false;

    uint32_t uidVAO = 0;
    uint32_t uidVBO = 0;
    uint32_t uidEBO = 0;
    uint32_t uidFB = 0;
    int viewport[4] = {};

    ShaderProgram *program = nullptr;
    bool freeProgram = false;
    int stride = 0;
    std::vector<VertexAttribPointerLayout> vertexAttrPtrLayouts;

    bool built = false;
    std::vector<uint8_t> vertices;
    std::vector<uint16_t> indices;

    hmm_mat4 mvpBase = HMM_Orthographic(-1, 1, -1, 1, -1, 1);
    hmm_mat4 transform = HMM_Mat4d(1);
    hmm_mat4 mvp;
    bool mvpDirty = false;
};

Pipeline::Pipeline(const Texture &texture, ShaderProgram *prog, int stri, const VertexAttribPointer *vap): ctx_(new PipelineCtx) {
    init(prog, stri, vap);
    ctx_->renderToTarget = true;
    ctx_->host = &texture;
}
Pipeline::Pipeline(Renderer &renderer, ShaderProgram *prog, int stri, const VertexAttribPointer *vap): ctx_(new PipelineCtx) {
    init(prog, stri, vap);
    ctx_->renderToTarget = false;
    ctx_->host = &renderer;
}
Pipeline::~Pipeline() {
    if (ctx_->freeProgram) {
        delete ctx_->program;
    }
    if (ctx_->uidFB) {
        glDeleteFramebuffers(1, &ctx_->uidFB);
    }
    glDeleteBuffers(1, &ctx_->uidEBO);
    glDeleteBuffers(1, &ctx_->uidVBO);
    glDeleteVertexArrays(1, &ctx_->uidVAO);
}
void Pipeline::setViewport(int x, int y, int w, int h) {
    ctx_->viewport[0] = x;
    ctx_->viewport[1] = y;
    ctx_->viewport[2] = w;
    ctx_->viewport[3] = h;
}
void Pipeline::setOrtho(float left, float right, float bottom, float top, float nearf, float farf) {
    ctx_->mvpBase = HMM_Orthographic(left, right, bottom, top, nearf, farf);
    ctx_->mvpDirty = true;
}
void Pipeline::resetTransform() {
    ctx_->transform = HMM_Mat4d(1.0f);
    ctx_->mvpDirty = true;
}
void Pipeline::setTransform(const float *mat) {
    memcpy(&ctx_->transform.Elements[0][0], mat, sizeof(float) * 4 * 4);
    ctx_->mvpDirty = true;
}
void Pipeline::translate(float x, float y, float z) {
    ctx_->transform = HMM_Translate(HMM_Vec3(x, y, z)) * ctx_->transform;
    ctx_->mvpDirty = true;
}
void Pipeline::scale(float x, float y, float z) {
    ctx_->transform = HMM_Scale(HMM_Vec3(x, y, z)) * ctx_->transform;
    ctx_->mvpDirty = true;
}
void Pipeline::rotate(float angle, float x, float y, float z) {
    ctx_->transform = HMM_Rotate(angle, HMM_Vec3(x, y, z)) * ctx_->transform;
    ctx_->mvpDirty = true;
}
void Pipeline::render() {
    if (ctx_->indices.empty()) { return; }
    if (!ctx_->built) {
        ctx_->built = true;
        if (ctx_->renderToTarget && ctx_->uidFB == 0) {
            glGenFramebuffers(1, &ctx_->uidFB);
            glBindFramebuffer(GL_FRAMEBUFFER, ctx_->uidFB);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, (uint32_t)*(Texture*)ctx_->host, 0);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        glBindVertexArray(ctx_->uidVAO);
        glBindBuffer(GL_ARRAY_BUFFER, ctx_->uidVBO);
        glBufferData(GL_ARRAY_BUFFER, ctx_->vertices.size(), &ctx_->vertices[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ctx_->uidEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * ctx_->indices.size(), &ctx_->indices[0], GL_STATIC_DRAW);
        int count = ctx_->vertexAttrPtrLayouts.size();
        for (int i = 0; i < count; ++i) {
            auto &vapp = ctx_->vertexAttrPtrLayouts[i];
            glVertexAttribPointer(i, vapp.size, vapp.type, vapp.normalized, ctx_->stride, vapp.pointer);
            glEnableVertexAttribArray(i);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    if (ctx_->renderToTarget) {
        int vw, vh;
        glBindFramebuffer(GL_FRAMEBUFFER, ctx_->uidFB);
        const auto *tex = (const Texture *)ctx_->host;
        vw = tex->width();
        vh = tex->height();
        glViewport(0, 0, vw, vh);
    } else {
        glViewport(ctx_->viewport[0], ctx_->viewport[1], ctx_->viewport[2], ctx_->viewport[3]);
    }
    doRender();
    if (ctx_->renderToTarget) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}
void Pipeline::push(int vertexCountIn, const void *verticesIn, int indexCountIn, const uint16_t *indicesIn) {
    auto delta = uint16_t(ctx_->vertices.size() / ctx_->stride);
    ctx_->vertices.insert(ctx_->vertices.end(), (const uint8_t*)verticesIn, (const uint8_t*)verticesIn + vertexCountIn * ctx_->stride);
    ctx_->indices.reserve(ctx_->indices.size() + indexCountIn);
    for (int i = 0; i < indexCountIn; ++i) {
        ctx_->indices.push_back(indicesIn[i] + delta);
    }
    ctx_->built = false;
}
void Pipeline::reset() {
    ctx_->vertices.clear();
    ctx_->indices.clear();
    ctx_->built = false;
}
void Pipeline::init(ShaderProgram *prog, int stri, const VertexAttribPointer *vap) {
    ctx_->program = prog ? prog : new ShaderProgram;
    ctx_->freeProgram = !prog;
    ctx_->stride = stri;
    glGenVertexArrays(1, &ctx_->uidVAO);
    glGenBuffers(1, &ctx_->uidVBO);
    glGenBuffers(1, &ctx_->uidEBO);
    int total = 0;
    for (; vap->fmt != VERTEXFORMAT_NONE; ++vap) {
        VertexAttribPointerLayout vapp;
        switch (vap->fmt) {
        case VERTEXFORMAT_FLOAT:
            vapp = { 1, GL_FLOAT, GL_FALSE, (const void*)(vap->offset == 0 ? total : vap->offset) };
            total += 4;
            break;
        case VERTEXFORMAT_FLOAT2:
            vapp = { 2, GL_FLOAT, GL_FALSE, (const void*)(vap->offset == 0 ? total : vap->offset) };
            total += 8;
            break;
        case VERTEXFORMAT_FLOAT3:
            vapp = { 3, GL_FLOAT, GL_FALSE, (const void*)(vap->offset == 0 ? total : vap->offset) };
            total += 12;
            break;
        case VERTEXFORMAT_FLOAT4:
            vapp = { 4, GL_FLOAT, GL_FALSE, (const void*)(vap->offset == 0 ? total : vap->offset) };
            total += 16;
            break;
        case VERTEXFORMAT_BYTE4:
            vapp = { 4, GL_BYTE, GL_FALSE, (const void*)(vap->offset == 0 ? total : vap->offset) };
            total += 4;
            break;
        case VERTEXFORMAT_BYTE4N:
            vapp = { 4, GL_BYTE, GL_TRUE, (const void*)(vap->offset == 0 ? total : vap->offset) };
            total += 4;
            break;
        case VERTEXFORMAT_UBYTE4:
            vapp = { 4, GL_UNSIGNED_BYTE, GL_FALSE, (const void*)(vap->offset == 0 ? total : vap->offset) };
            total += 4;
            break;
        case VERTEXFORMAT_UBYTE4N:
            vapp = { 4, GL_UNSIGNED_BYTE, GL_TRUE, (const void*)(vap->offset == 0 ? total : vap->offset) };
            total += 4;
            break;
        case VERTEXFORMAT_SHORT2:
            vapp = { 2, GL_SHORT, GL_FALSE, (const void*)(vap->offset == 0 ? total : vap->offset) };
            total += 4;
            break;
        case VERTEXFORMAT_SHORT2N:
            vapp = { 2, GL_SHORT, GL_TRUE, (const void*)(vap->offset == 0 ? total : vap->offset) };
            total += 4;
            break;
        case VERTEXFORMAT_USHORT2N:
            vapp = { 2, GL_UNSIGNED_SHORT, GL_TRUE, (const void*)(vap->offset == 0 ? total : vap->offset) };
            total += 4;
            break;
        case VERTEXFORMAT_SHORT4:
            vapp = { 4, GL_SHORT, GL_FALSE, (const void*)(vap->offset == 0 ? total : vap->offset) };
            total += 8;
            break;
        case VERTEXFORMAT_SHORT4N:
            vapp = { 4, GL_SHORT, GL_TRUE, (const void*)(vap->offset == 0 ? total : vap->offset) };
            total += 8;
            break;
        case VERTEXFORMAT_USHORT4N:
            vapp = { 4, GL_UNSIGNED_SHORT, GL_TRUE, (const void*)(vap->offset == 0 ? total : vap->offset) };
            total += 8;
            break;
        case VERTEXFORMAT_UINT10_N2:
            vapp = { 4, GL_UNSIGNED_INT_2_10_10_10_REV, GL_TRUE, (const void*)(vap->offset == 0 ? total : vap->offset) };
            break;
        default:
            continue;
        }
        ctx_->vertexAttrPtrLayouts.emplace_back(vapp);
    }
    if (!ctx_->stride) { ctx_->stride = total; }
}
void Pipeline::doRender() {
    if (ctx_->mvpDirty) {
        ctx_->mvp = ctx_->mvpBase * ctx_->transform;
        ctx_->mvpDirty = false;
    }
    ctx_->program->use();
    setMVP(&ctx_->mvp.Elements[0][0]);
    glBindVertexArray(ctx_->uidVAO);
    glDrawElements(GL_TRIANGLES, ctx_->indices.size(), GL_UNSIGNED_SHORT, nullptr);
    glBindVertexArray(0);
    ShaderProgram::unuse();
}

struct Texture2DVertexData {
    float x, y;
    float u, v;
    uint32_t color;
};
static constexpr VertexAttribPointer texture2DVAP[] = {
    {VERTEXFORMAT_FLOAT2, 0},
    {VERTEXFORMAT_FLOAT2, 8},
    {VERTEXFORMAT_UBYTE4N, 16},
    {VERTEXFORMAT_NONE},
};

PipelineTexture2D::PipelineTexture2D(Renderer &renderer): Pipeline(renderer, nullptr, 0, texture2DVAP) {
    init();
}
PipelineTexture2D::PipelineTexture2D(const Texture &tex): Pipeline(tex, nullptr, 0, texture2DVAP) {
    init();
}
void PipelineTexture2D::setTexture(const Texture &tex) {
    tex_ = &tex;
}
void PipelineTexture2D::pushQuad(float x0, float y0, float x1, float y1, uint32_t color) {
    float u1 = float(tex_->widthReal()) / float(tex_->width());
    float v1 = float(tex_->heightReal()) / float(tex_->height());
    pushQuad(x0, y0, x1, y1, 0, 0, u1, v1, color);
}
void PipelineTexture2D::pushQuad(float x0, float y0, float x1, float y1, float u0, float v0, float u1, float v1, uint32_t color) {
    Texture2DVertexData v[] = {
        {x0, y0, u0, v0, color},
        {x0, y1, u0, v1, color},
        {x1, y1, u1, v1, color},
        {x1, y0, u1, v0, color},
    };
    const uint16_t i[6] = {0, 1, 2, 0, 2, 3};
    push(4, v, 6, i);
}
void PipelineTexture2D::setMVP(const float *mvp) {
    ctx_->program->setMat4(uniformMVP_, mvp);
}
void PipelineTexture2D::doRender() {
    glActiveTexture(GL_TEXTURE0);
    tex_->bind();
    Pipeline::doRender();
}
void PipelineTexture2D::init() {
    if (ctx_->freeProgram) {
        ctx_->program->compileAndLink(R"(
            #version 330
            uniform mat4 mvp;
            layout(location=0) in vec2 position;
            layout(location=1) in vec2 texcoord0;
            layout(location=2) in vec4 color;
            out vec2 uv;
            out vec4 pass_color;
            void main() {
              gl_Position = mvp * vec4(position.xy, 0, 1);
              uv = texcoord0;
              pass_color = color;
            }
        )", R"(
            #version 330
            uniform sampler2D tex;
            in vec2 uv;
            in vec4 pass_color;
            out vec4 frag_color;
            void main() {
              frag_color = texture(tex, uv) * pass_color;
            }
        )"
        );
        ctx_->program->setInt("tex", 0);
    }
    uniformMVP_ = ctx_->program->uniformIndex("mvp");
}

struct Squad2DVertexData {
    float x, y;
    uint32_t color;
};
static constexpr VertexAttribPointer squad2DVAP[] = {
    {VERTEXFORMAT_FLOAT2, 0},
    {VERTEXFORMAT_UBYTE4N, 8},
    {VERTEXFORMAT_NONE},
};

PipelineSquad2D::PipelineSquad2D(Renderer &renderer): Pipeline(renderer, nullptr, 0, squad2DVAP) {
    init();
}
PipelineSquad2D::PipelineSquad2D(const Texture &tex): Pipeline(tex, nullptr, 0, squad2DVAP) {
    init();
}
void PipelineSquad2D::pushQuad(float x0, float y0, float x1, float y1, uint32_t color) {
    const Squad2DVertexData v[4] = {
        {x0, y0, color},
        {x0, y1, color},
        {x1, y1, color},
        {x1, y0, color},
    };
    const uint16_t i[6] = {0, 1, 2, 0, 2, 3};
    push(4, v, 6, i);
}
void PipelineSquad2D::pushQuad(float x0, float y0, float x1, float y1,
                               float x2, float y2, float x3, float y3,
                               uint32_t color) {
    const Squad2DVertexData v[4] = {
        {x0, y0, color},
        {x1, y1, color},
        {x2, y2, color},
        {x3, y3, color},
    };
    const uint16_t i[6] = {0, 1, 2, 0, 2, 3};
    push(4, v, 6, i);
}
void PipelineSquad2D::drawLine(float x0, float y0, float x1, float y1, float width, uint32_t color) {
    auto fromPt = HMM_Vec2(x0, y0);
    auto toPt = HMM_Vec2(x1, y1);
    auto delta = toPt - fromPt;
    auto perp = HMM_Normalize(HMM_Vec2(-delta.Y, delta.X)) * width * .5f;
    auto A = fromPt + perp;
    auto B = fromPt - perp;
    auto C = toPt - perp;
    auto D = toPt + perp;
    const Squad2DVertexData v[] = {
        {A.X, A.Y, color},
        {B.X, B.Y, color},
        {C.X, C.Y, color},
        {D.X, D.Y, color},
    };
    const uint16_t i[6] = {0, 1, 2, 0, 2, 3};
    push(4, v, 6, i);
}
void PipelineSquad2D::setMVP(const float *mvp) {
    ctx_->program->setMat4(uniformMVP_, mvp);
}
void PipelineSquad2D::init() {
    if (ctx_->freeProgram) {
        ctx_->program->compileAndLink(R"(
            #version 330
            uniform mat4 mvp;
            layout(location=0) in vec2 position;
            layout(location=1) in vec4 color;
            out vec4 outColor;
            void main() {
              gl_Position = mvp * vec4(position.xy, 0, 1);
              outColor = color;
            }
        )", R"(
            #version 330
            in vec4 outColor;
            out vec4 frag_color;
            void main() {
              frag_color = outColor;
            }
        )");
    }
    uniformMVP_ = ctx_->program->uniformIndex("mvp");
}

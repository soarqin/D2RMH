/*
 * Copyright (c) 2021 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#pragma once

#include <vector>
#include <cstdint>

class Window;
struct RendererCtx;

class Pipeline;

class Renderer final {
public:
    explicit Renderer(Window*);
    ~Renderer();

    [[nodiscard]] Window *owner();

    static void setSwapInterval(int interval);
    static void setViewport(int x, int y, int w, int h);
    void limitFPS(uint32_t fps);
    void setClearColor(float r, float g, float b, float a);
    void prepare();
    void begin();
    void end();

    void getDimension(int &width, int &height) const;

private:
    RendererCtx *ctx_;
};

struct Shader final {
    enum Type {
        VertexShader,
        FragmentShader,
    };
    explicit Shader(Type type);
    ~Shader();
    bool compile(const char *src);
    [[nodiscard]] inline explicit operator uint32_t() const { return uid; }
    [[nodiscard]] inline Type type() { return tp; }

private:
    uint32_t uid = 0;
    Type tp = VertexShader;
};

struct ShaderProgram final {
    ~ShaderProgram();
    bool link(const Shader **shaders);
    bool compileAndLink(const char *vsSrc, const char *fsSrc);
    void use() const;
    static void unuse();
    [[nodiscard]] int uniformIndex(const char *name) const;
    static void setInt(int id, int value);
    void setInt(const char *name, int value) const;
    static void setFloat(int id, float value);
    void setFloat(const char *name, float value) const;
    static void setMat4(int id, const float *data);
    void setMat4(const char *name, const float *data) const;
    [[nodiscard]] inline explicit operator uint32_t() const { return uid; }

private:
    uint32_t uid = 0;
};

class Texture final {
public:
    Texture();
    ~Texture();
    void create(int width, int height);
    void setData(int width, int height, const uint32_t *data);
    void updateSubData(int x, int y, int width, int height, const uint32_t *data) const;
    void bind() const;
    static void unbind();
    [[nodiscard]] inline explicit operator uint32_t() const { return uid_; }
    [[nodiscard]] int width() const { return width_; }
    [[nodiscard]] int height() const { return height_; }
    [[nodiscard]] int widthReal() const { return widthReal_; }
    [[nodiscard]] int heightReal() const { return heightReal_; }

private:
    uint32_t uid_ = 0;
    int width_ = 0, height_ = 0, widthReal_ = 0, heightReal_ = 0;
};

enum VertexFormat {
    VERTEXFORMAT_NONE,
    VERTEXFORMAT_FLOAT,
    VERTEXFORMAT_FLOAT2,
    VERTEXFORMAT_FLOAT3,
    VERTEXFORMAT_FLOAT4,
    VERTEXFORMAT_BYTE4,
    VERTEXFORMAT_BYTE4N,
    VERTEXFORMAT_UBYTE4,
    VERTEXFORMAT_UBYTE4N,
    VERTEXFORMAT_SHORT2,
    VERTEXFORMAT_SHORT2N,
    VERTEXFORMAT_USHORT2N,
    VERTEXFORMAT_SHORT4,
    VERTEXFORMAT_SHORT4N,
    VERTEXFORMAT_USHORT4N,
    VERTEXFORMAT_UINT10_N2,
    VERTEXFORMAT_MAX,
};

struct VertexAttribPointer {
    VertexFormat fmt = VERTEXFORMAT_FLOAT;
    int offset = 0;
};

struct PipelineCtx;

class Pipeline {
public:
    Pipeline(const Texture &texture, ShaderProgram *prog, int stri, const VertexAttribPointer *vap);
    Pipeline(Renderer &renderer, ShaderProgram *prog, int stri, const VertexAttribPointer *vap);
    virtual ~Pipeline();
    void setViewport(int x, int y, int w, int h);
    void setOrtho(float left, float right, float bottom, float top, float nearf = -1, float far = 1);
    void resetTransform();
    void setTransform(const float *mat);
    void translate(float x, float y, float z);
    void scale(float x, float y, float z);
    void rotate(float angle, float x, float y, float z);
    void render();
    void push(int vertexSizeIn, const void *verticesIn, int indexCountIn, const uint16_t *indicesIn);
    void reset();

protected:
    void init(ShaderProgram *prog, int stri, const VertexAttribPointer *vap);
    virtual void setMVP(const float *mvp) {}
    virtual void doRender();

protected:
    PipelineCtx *ctx_;
};

class PipelineTexture2D: public Pipeline {
public:
    explicit PipelineTexture2D(Renderer &renderer);
    explicit PipelineTexture2D(const Texture &tex);
    void setTexture(const Texture &tex);
    void pushQuad(float x0, float y0, float x1, float y1, uint32_t color = 0xFFFFFFFFu);
    void pushQuad(float x0, float y0, float x1, float y1, float u0, float v0, float u1, float v1, uint32_t color = 0xFFFFFFFFu);

protected:
    void setMVP(const float *mvp) override;
    void doRender() override;
    void init();

private:
    const Texture *tex_ = nullptr;
    int uniformMVP_ = -1;
};

class PipelineSquad2D: public Pipeline {
public:
    explicit PipelineSquad2D(Renderer &renderer);
    explicit PipelineSquad2D(const Texture &tex);
    void pushQuad(float x0, float y0, float x1, float y1, uint32_t color);
    void pushQuad(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3, uint32_t color);
    void drawLine(float x0, float y0, float x1, float y1, float width, uint32_t color);

protected:
    void setMVP(const float *mvp) override;
    void init();

private:
    int uniformMVP_ = -1;
};

#pragma once //

#include "Stallout/containers.h"

#include "Stallout/graphics/graphics_commands.h"
#include "Stallout/graphics/renderer_base.h"

#include <mz_matrix.hpp>

NS_BEGIN(stallout);
NS_BEGIN(graphics);

struct Graphics_Driver;

struct ST_API Primitive_Renderer2D {
    struct Line_Vertex {
        mz::fvec2 pos = 0;
        mz::color color = mz::COLOR_WHITE;
    };
    struct Line {
        Line_Vertex a, b;
    };

    struct Context {
        Camera_Transform2D camera;
        mz::ivec4 viewport;

        Array<Line> _lines;
        Array<u32> _line_indices;
        size_t _line_vbo_size = 0;
        size_t _line_ibo_size = 0;

        Resource_Handle _line_vbo, _line_ibo, _line_vao, _line_shader;

        bool _ready = false;
        
        std::mutex _mutex;
    };

    Graphics_Driver* driver;

    Primitive_Renderer2D(Graphics_Driver* driver);

    Context* make_context(mz::ivec4 viewport, size_t hint_lines = 500);
    void free_context(Context*& ctx);

    void new_frame(Context* ctx);

    void draw_line(Context* ctx, mz::fvec2 a, mz::fvec2 b, mz::color color = mz::COLOR_WHITE);
    void draw_line(Context* ctx, mz::fvec4 line, mz::color color = mz::COLOR_WHITE);
    void draw_aabb(Context* ctx, mz::frect aabb, mz::color color = mz::COLOR_WHITE);
    void draw_circle(Context* ctx, mz::fvec2 center, f32 radius, mz::color color = mz::COLOR_WHITE, f32 complexity = 64);

    void render(Context* ctx);
};

NS_END(graphics);
NS_END(stallout);
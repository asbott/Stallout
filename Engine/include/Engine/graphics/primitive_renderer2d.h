#pragma once

#include "Engine/containers.h"

#include "Engine/graphics/graphics_commands.h"
#include "Engine/graphics/renderer_base.h"

#include <mz_matrix.hpp>

NS_BEGIN(engine);
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

        Array<Line> _lines;
        Array<u32> _line_indices;
        size_t _line_vbo_size = 0;

        Resource_Handle _line_vbo, _line_ibo, _line_vao, _line_shader;

        bool _ready = false;
        
        std::mutex _mutex;
    };

    Graphics_Driver* driver;

    Primitive_Renderer2D(Graphics_Driver* driver);

    Context* make_context(size_t hint_lines = 500);

    void new_frame(Context* ctx);

    void draw_line(Context* ctx, mz::fvec2 a, mz::fvec2 b, mz::color color = mz::COLOR_WHITE);
    void draw_line(Context* ctx, mz::fvec4 line, mz::color color = mz::COLOR_WHITE);

    void render(Context* ctx);
};

NS_END(graphics);
NS_END(engine);
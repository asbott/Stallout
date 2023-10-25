#include "pch.h"

#include "graphics/primitive_renderer2d.h"

#include "Engine/graphics/graphics_driver.h"

#include <mz_algorithms.hpp>
#include <mz_matrix.hpp>

const char line_vert_shader[] = R"(
layout(location = 0) in vec2 a_pos;
layout(location = 1) in vec4 a_color;

out vec4 vs_color;

void main() {
    gl_Position = vec4(a_pos, 0.0, 1.0);

    vs_color = a_color;
}
)";
const char line_frag_shader[] = R"(
out vec4 output_color;
in vec4 vs_color;

void main() {
    output_color = vs_color;
}
)";

NS_BEGIN(engine);
NS_BEGIN(graphics);

using Context = Primitive_Renderer2D::Context;

Primitive_Renderer2D::Primitive_Renderer2D(Graphics_Driver* driver) {
    this->driver = driver;
}

Context* Primitive_Renderer2D::make_context(size_t hint_lines) {
    Context* ctx = stnew (Context);

    ctx->_lines.reserve(hint_lines);
    ctx->_line_indices.reserve(hint_lines);
    ctx->_line_vbo_size = hint_lines * sizeof(Line);

    ctx->_line_shader = driver->create_shader(line_vert_shader, line_frag_shader);

    ctx->_line_vbo = driver->create_buffer(BUFFER_TYPE_ARRAY_BUFFER, BUFFER_USAGE_DYNAMIC_DRAW);
    ctx->_line_ibo = driver->create_buffer(BUFFER_TYPE_ELEMENT_ARRAY_BUFFER, BUFFER_USAGE_DYNAMIC_DRAW);

    driver->set_buffer(ctx->_line_vbo, 0, ctx->_line_vbo_size);
    driver->set_buffer(ctx->_line_ibo, 0, (ctx->_line_vbo_size / sizeof(Line)) * sizeof(u32) * 6);

    ctx->_line_vao = driver->create_buffer_layout({
        { 2, DATA_TYPE_FLOAT },
        { 4, DATA_TYPE_FLOAT }
    });    

    return ctx;
}

void Primitive_Renderer2D::new_frame(Context* ctx) {
    std::lock_guard l(ctx->_mutex);

    ST_DEBUG_ASSERT(!ctx->_ready, "Renderer already in use");

    ctx->_lines.resize(0);
    ctx->_line_indices.resize(0);

    ctx->_ready = true;
}

void Primitive_Renderer2D::draw_line(Context* ctx, mz::fvec2 a, mz::fvec2 b, mz::color color) {
    std::lock_guard l(ctx->_mutex);

    ST_DEBUG_ASSERT(ctx->_ready, "Renderer not ready");

    mz::fmat4 cam_transform = ctx->camera.get_transform();

    Line line;

    line.a.pos = cam_transform * a;
    line.a.color = color;

    line.b.pos = cam_transform * b;
    line.b.color = color;

    ctx->_line_indices.push_back((u32)(2 * ctx->_lines.size()));
    ctx->_line_indices.push_back((u32)(2 * ctx->_lines.size() + 1));

    ctx->_lines.push_back(line);


}
void Primitive_Renderer2D::draw_line(Context* ctx, mz::fvec4 line, mz::color color) {
    draw_line(ctx, line.xy, line.zw, color);
}

void Primitive_Renderer2D::render(Context* ctx) {
    std::lock_guard l(ctx->_mutex);

    ST_DEBUG_ASSERT(ctx->_ready, "Renderer not ready");


    bool realloc = ctx->_lines.size() * sizeof(Line) > ctx->_line_vbo_size;

    if (realloc) {
        size_t nsize = (size_t)(ctx->_line_vbo_size * 1.5);

        ctx->_lines.reserve(nsize / sizeof(Line));
        ctx->_line_indices.reserve(nsize / sizeof(Line) * 2);

        driver->set_buffer(ctx->_line_vbo, ctx->_lines.data(), nsize);
        driver->set_buffer(ctx->_line_ibo, ctx->_line_indices.data(), nsize);

        ctx->_line_vbo_size = nsize;
    } else {
        driver->append_to_buffer(ctx->_line_vbo, 0, ctx->_lines.data(), ctx->_lines.size() * sizeof(Line));
        driver->append_to_buffer(ctx->_line_ibo, 0, ctx->_line_indices.data(), ctx->_line_indices.size() * sizeof(u32));
    }

    driver->draw_indexed(
        ctx->_line_vao, 
        ctx->_line_vbo,
        ctx->_line_ibo,
        ctx->_line_shader,
        ctx->_line_indices.size(),
        DATA_TYPE_UINT,
        0,
        DRAW_MODE_LINES
    );

    ctx->_ready = false;
}

NS_END(graphics);
NS_END(engine);
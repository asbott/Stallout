#include "pch.h"

#include "graphics/primitive_renderer2d.h"

#include "Stallout/graphics/graphics_driver.h"

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

NS_BEGIN(stallout);
NS_BEGIN(graphics);

using Context = Primitive_Renderer2D::Context;

Primitive_Renderer2D::Primitive_Renderer2D(Graphics_Driver* driver) {
    this->driver = driver;
}

Context* Primitive_Renderer2D::make_context(mz::ivec4 vp, size_t hint_lines) {
    Context* ctx = stnew (Context);

    ctx->viewport = vp;

    ctx->_lines.reserve(hint_lines);
    ctx->_line_indices.reserve(hint_lines);
    ctx->_line_vbo_size = hint_lines * sizeof(Line);
    ctx->_line_ibo_size = hint_lines * sizeof(u32);

    ctx->_line_shader = driver->create_shader(line_vert_shader, line_frag_shader);

    ctx->_line_vbo = driver->create_buffer(BUFFER_TYPE_ARRAY_BUFFER, BUFFER_USAGE_DYNAMIC_DRAW);
    ctx->_line_ibo = driver->create_buffer(BUFFER_TYPE_ELEMENT_ARRAY_BUFFER, BUFFER_USAGE_DYNAMIC_DRAW);

    driver->set_buffer(ctx->_line_vbo, 0, ctx->_line_vbo_size);
    driver->set_buffer(ctx->_line_ibo, 0, ctx->_line_ibo_size);

    ctx->_line_vao = driver->create_buffer_layout({
        { 2, DATA_TYPE_FLOAT },
        { 4, DATA_TYPE_FLOAT }
    });    

    return ctx;
}

void Primitive_Renderer2D::free_context(Context*& ctx) {
    driver->destroy(ctx->_line_shader);
    driver->destroy(ctx->_line_vbo);
    driver->destroy(ctx->_line_ibo);
    driver->destroy(ctx->_line_vao);
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
void Primitive_Renderer2D::draw_aabb(Context* ctx, mz::frect aabb, mz::color color) {
    draw_line(ctx, mz::fvec2(aabb.left, aabb.top), mz::fvec2(aabb.right, aabb.top), color);
    draw_line(ctx, mz::fvec2(aabb.right, aabb.top), mz::fvec2(aabb.right, aabb.bottom), color);
    draw_line(ctx, mz::fvec2(aabb.right, aabb.bottom), mz::fvec2(aabb.left, aabb.bottom), color);
    draw_line(ctx, mz::fvec2(aabb.left, aabb.bottom), mz::fvec2(aabb.left, aabb.top), color);
}
void Primitive_Renderer2D::draw_circle(Context* ctx, mz::fvec2 center, f32 radius, mz::color color, f32 complexity) {
    if (complexity <= 0) complexity = 1;
    f32 increment = 2.0f * math::PI32 / complexity;
    for (int i = 0; i < complexity; ++i) {
        float theta = i * increment;
        float nextTheta = (i + 1) * increment;

        mz::fvec2 start = mz::fvec2(center.x + radius * math::cos(theta), center.y + radius * math::sin(theta));
        mz::fvec2 end = mz::fvec2(center.x + radius * math::cos(nextTheta), center.y + radius * math::sin(nextTheta));

        draw_line(ctx, start, end, color);
    }

    draw_line(ctx, center + mz::fvec2(0, radius), center + mz::fvec2(0, -radius));
    draw_line(ctx, center + mz::fvec2(radius, 0), center + mz::fvec2(-radius, 0));
}

void Primitive_Renderer2D::render(Context* ctx) {
    std::lock_guard l(ctx->_mutex);

    ST_DEBUG_ASSERT(ctx->_ready, "Renderer not ready");


    size_t new_vbo_size = ctx->_lines.size() * sizeof(Line);
    size_t new_ibo_size = ctx->_line_indices.size() * sizeof(u32);

    bool vbo_resize = ctx->_line_vbo_size < new_vbo_size;
    bool ibo_resize = ctx->_line_ibo_size < new_ibo_size;

    {
        if (vbo_resize) {
            size_t nsize = (size_t)(ctx->_lines.size() * sizeof(Line) * 1.5);

            ctx->_lines.reserve((size_t)(nsize) / sizeof(Line));
            driver->set_buffer(ctx->_line_vbo, ctx->_lines.data(), (size_t)(nsize));
            ctx->_line_vbo_size = (size_t)(nsize);
        } else {
            driver->append_to_buffer(ctx->_line_vbo, 0, ctx->_lines.data(), new_vbo_size);
        }
    }
    {
        if (ibo_resize) {
            size_t nsize = (size_t)(ctx->_line_indices.size() * sizeof(u32) * 1.5);

            ctx->_line_indices.reserve(nsize / sizeof(u32));
            driver->set_buffer(ctx->_line_ibo, ctx->_line_indices.data(), nsize);
            ctx->_line_ibo_size = nsize;
        } else {
            driver->append_to_buffer(ctx->_line_ibo, 0, ctx->_line_indices.data(), new_ibo_size);
        }
    }

    driver->set_viewport(ctx->viewport.xy, ctx->viewport.zw);

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
NS_END(stallout);
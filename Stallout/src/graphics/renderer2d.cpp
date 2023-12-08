#include "pch.h"

#include "graphics/renderer2d.h"

#include <mz_algorithms.hpp>

struct Camera_Block {
    mz::fmat4 ortho;
    mz::fmat4 view;
    mz::fvec2 viewport_size;
};

constexpr char vert_shader_source[] = R"(

layout (location = 0) in vec2 pos;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec4 color;
layout (location = 3) in float texture_index;
layout (location = 4) in vec2 center;
layout (location = 5) in float sampling;
layout (location = 6) in float is_text;

layout (std140) uniform Camera_Block {
	mat4 ortho;
    mat4 view;
    vec2 viewport_size;
};

out vec2 vs_uv;
out vec4 vs_color;
out flat int vs_texture_index;
out flat int vs_sampling;
out flat int vs_is_text;

vec4 snap_point(vec4 p, vec2 pixel_size) {
    p.x = floor(p.x / pixel_size.x) * pixel_size.x + pixel_size.x * 0.5;
    p.y = floor(p.y / pixel_size.y) * pixel_size.y + pixel_size.y * 0.5;

    return p;
}

void main() {
    vs_uv = uv;
    vs_color = color;
    vs_texture_index = int(round(texture_index));
    vs_sampling = int(round(sampling));
    vs_is_text = int(round(is_text));

    // Snap to pixel grid
    vec4 norm_pos = ortho * view * vec4((pos.x), (pos.y), 0.0, 1.0);
    vec2 pixel_scale = (1.0 / viewport_size);
    gl_Position = snap_point(norm_pos, pixel_scale);
}

)";

constexpr char frag_shader_source[] = R"(

out vec4 output_color;

in vec2 vs_uv;
in vec4 vs_color;
in flat int vs_texture_index;
in flat int vs_sampling;
in flat int vs_is_text;

void main()
{
    if (vs_is_text > 0) {
        vec4 texture_color = sample_texture(vs_texture_index, vs_uv, vs_sampling);

        output_color = vec4(1.0, 1.0, 1.0, texture_color.r) * vs_color;

        //if (texture_color.r < 0.5) discard;

    } else {
        if (vs_texture_index == -1) {
            output_color = vs_color;
        } else {
            vec4 texture_color = sample_texture(vs_texture_index, vs_uv, vs_sampling);
            
            output_color = vs_color * texture_color;
        }
    }
}

)";

NS_BEGIN(stallout);
NS_BEGIN(graphics);

using Context = Renderer2D::Context;


Renderer2D::Renderer2D(Graphics_Driver* gfx)
    : gfx(gfx), max_texture_slots(gfx->get_environment().max_texture_slots) {
        
}
Renderer2D::~Renderer2D() {

}

Context* Renderer2D::make_context(mz::ivec4 vp, size_t hint_quad_count) {
    Context* ctx = stnew (Context);

    ctx->viewport = vp;

    ctx->_quad_buffer.reserve(hint_quad_count);
    ctx->_indices.reserve(hint_quad_count * 6);
    ctx->_texture_slots.resize(max_texture_slots);

    ctx->_default_shader = gfx->create_shader(vert_shader_source, frag_shader_source);
    ctx->shader = ctx->_default_shader;
    
    std::lock_guard gfx_lock(gfx_mutex);

    ctx->_current_vbo_size = hint_quad_count * sizeof(Quad);
    ctx->_vbo = gfx->create_buffer(BUFFER_TYPE_ARRAY_BUFFER, BUFFER_USAGE_DYNAMIC_DRAW);
    gfx->set_buffer(ctx->_vbo, 0, ctx->_current_vbo_size);

    ctx->_current_ibo_size = hint_quad_count * sizeof(u32) * 6;
    ctx->_ibo = gfx->create_buffer(BUFFER_TYPE_ELEMENT_ARRAY_BUFFER, BUFFER_USAGE_DYNAMIC_DRAW);
    gfx->set_buffer(ctx->_ibo, 0, ctx->_current_ibo_size);

    ctx->_ubo = gfx->create_buffer(BUFFER_TYPE_UNIFORM_BUFFER, BUFFER_USAGE_DYNAMIC_DRAW);
    gfx->set_buffer(ctx->_ubo, 0, sizeof(Camera_Block));

    ctx->_default_layout = gfx->create_buffer_layout({
        { 2, DATA_TYPE_FLOAT }, // Position
        { 2, DATA_TYPE_FLOAT, true }, // UV
        { 4, DATA_TYPE_UBYTE, true }, // Color
        { 1, DATA_TYPE_FLOAT }, // Texture index
        { 2, DATA_TYPE_FLOAT }, // Center
        { 1, DATA_TYPE_FLOAT }, // Sampling Type
        { 1, DATA_TYPE_FLOAT } // Is Text
    });
    ctx->layout = ctx->_default_layout;

    ctx->camera.width = 1280;
    ctx->camera.height = 720;

    return ctx;
}
void Renderer2D::free_context(Context*& ctx) {
    ctx->_mutex.lock();

    std::lock_guard gfx_lock(gfx_mutex);

    gfx->destroy(ctx->layout);
    gfx->destroy(ctx->_vbo);
    gfx->destroy(ctx->_ibo);
    gfx->destroy(ctx->_ubo);
    gfx->destroy(ctx->_default_shader);

    ctx->_mutex.unlock();

    ST_FREE(ctx, sizeof(Context));
    ctx = NULL;
}

void Renderer2D::_begin(Context* ctx) {
    

    ctx->_quad_buffer.resize(0);
    for (size_t i = 0; i < ctx->_texture_slots.size(); i++) 
        ctx->_texture_slots[i] = 0;
    ctx->_indices.resize(0);

    ctx->_next_index = 0;
    ctx->_next_texture_index = 0;
}
void Renderer2D::new_frame(Context* ctx) {
    tm_func();
    ctx->_mutex.lock();

    ST_DEBUG_ASSERT(!ctx->_active, "2D Rendering context already active");
    ctx->_active = true;

    _begin(ctx);

    ctx->stats.draw_calls = 0;
    ctx->stats.quads = 0;
    ctx->stats.textures = 0;

    ctx->_mutex.unlock();
}

void Renderer2D::draw(
        Context* ctx, 
        Resource_Handle texture, 
        mz::fvec2 position, 
        mz::fvec2 size, 
        float rotation, 
        mz::fvec2 pivot, 
        const mz::color& color, 
        const mz::fvec4& uv,
        const AABB& skew,
        Texture2D_Sampling_Type sampling,
        bool is_text) {
    //tm_func();
    ctx->_mutex.lock();
    ST_DEBUG_ASSERT(ctx->_active, "2D Rendering context not active, call begin()");

    s32 texture_index = -1;

    if (texture) {
        bool bound = false;
        for (u32 slot = 0; slot < ctx->_next_texture_index; slot++) {
            auto htex = ctx->_texture_slots[slot];

            if (htex == texture) {
                bound = true;
                texture_index = slot;
                break;
            }
        }
        if (!bound) {
            if (ctx->_next_texture_index >= ctx->_texture_slots.size()) {
                this->_flush(ctx);
                this->_begin(ctx);
            }

            ctx->stats.textures++;
            texture_index = ctx->_next_texture_index;
            ctx->_texture_slots[ctx->_next_texture_index++] = texture;
        }
    }

    mz::fvec2 uvs[4] = {
        { 0, 1 }, // Bottom left
        { 0, 0 }, // Top left
        { 1, 0 }, // Top right
        { 1, 1 }, // Bottom right
    };

    if (uv.magnitude() != 0) {
        uvs[0] = { uv.x1, uv.y2 };
        uvs[1] = { uv.x1, uv.y1 };
        uvs[2] = { uv.x2, uv.y1 };
        uvs[3] = { uv.x2, uv.y2 };
    }

    ctx->stats.quads++;
    auto& quad = ctx->_quad_buffer.emplace_back();

    for (u32 i = 0; i < 4; i++) {
        Vertex& vert = quad.vertices[i];

        mz::fvec2 offset = 0; // Offset from BL
        if (i == 1) {
            offset = { 0, size.y }; // TL
        } else if (i == 2) {
            offset = { size.x, size.y }; // TR
        } else if (i == 3) {
            offset = { size.x, 0 }; // BR
        }

        auto transform = mz::transformation::translation<f32>((position));
        transform.rotate(rotation, { 0, 0, -1 });
        transform.translate(offset-pivot+skew.ptr[i]);

        vert.position = transform.get_translation();

        mz::u8vec4 icol(
            math::clamp(color.r, 0.f, 1.f) * 255,
            math::clamp(color.g, 0.f, 1.f) * 255,
            math::clamp(color.b, 0.f, 1.f) * 255,
            math::clamp(color.a, 0.f, 1.f) * 255
        );
        vert.color = *((u32*)icol.ptr);
        vert.texture_coords = uvs[i];
        vert.texture_index = (f32)texture_index;
        vert.center = (position - pivot) + size / 2.f;
        vert.sampling_type = (f32)sampling;
        vert.is_text = (f32)is_text;
    }

    auto i = ctx->_next_index;
    ctx->_indices.push_back(i + 3);
    ctx->_indices.push_back(i + 1);
    ctx->_indices.push_back(i + 0);
    ctx->_indices.push_back(i + 3);
    ctx->_indices.push_back(i + 2);
    ctx->_indices.push_back(i + 1);

    ctx->_next_index += 4;

    ctx->_mutex.unlock();
}
void Renderer2D::draw_quad(Context* ctx, mz::fvec2 position, mz::fvec2 size, float rotation, mz::fvec2 pivot, const mz::color& color, const AABB& skew) {
    draw(ctx, 0, position, size, rotation, pivot, color, mz::fvec4(0, 0, 1, 1), skew);
}

void Renderer2D::_flush(Context* ctx) {
    tm_func();

    
    Camera_Block cam;
    cam.ortho = ctx->camera.get_ortho().transpose();
    cam.view = ctx->camera.get_view().transpose();
    cam.viewport_size = ctx->viewport.zw;

    std::lock_guard gfx_lock(gfx_mutex);

    gfx->set_viewport(ctx->viewport.xy, ctx->viewport.zw);

    gfx->set_uniform_block(ctx->shader ? ctx->shader : ctx->_default_shader, ctx->_ubo, "Camera_Block", 0);
    gfx->append_to_buffer(ctx->_ubo, 0, &cam);

    size_t new_vbo_size = ctx->_quad_buffer.size() * sizeof(Quad);
    size_t new_ibo_size = ctx->_indices.size() * sizeof(u32);

    bool vbo_resize = ctx->_current_vbo_size < new_vbo_size;
    bool ibo_resize = ctx->_current_ibo_size < new_ibo_size;

    {
        if (vbo_resize) {
            size_t nsize = (size_t)(ctx->_quad_buffer.size() * sizeof(Quad) * 1.5);

            ctx->_quad_buffer.reserve((size_t)(nsize) / sizeof(Quad));
            nsize = ctx->_quad_buffer.size() * sizeof(Quad);
            gfx->set_buffer(ctx->_vbo, ctx->_quad_buffer.data(), (size_t)(nsize));
            ctx->_current_vbo_size = (size_t)(nsize);
            ctx->stats.total_reallocations++;
        } else {
            gfx->append_to_buffer(ctx->_vbo, 0, ctx->_quad_buffer.data(), new_vbo_size);
        }
    }
    {
        if (ibo_resize) {
            size_t nsize = (size_t)(ctx->_indices.size() * sizeof(u32) * 1.5);
            ctx->_indices.reserve(nsize / sizeof(u32));
            nsize = ctx->_indices.size() * sizeof(u32);
            gfx->set_buffer(ctx->_ibo, ctx->_indices.data(), nsize);
            ctx->_current_ibo_size = nsize;
            ctx->stats.total_reallocations++;
        } else {
            gfx->append_to_buffer(ctx->_ibo, 0, ctx->_indices.data(), new_ibo_size);
        }
    }

    for (u32 i = 0; i < ctx->_texture_slots.size(); i++) {
        if (!ctx->_texture_slots[i]) break;

        gfx->set_texture_slot(i, ctx->_texture_slots[i]);
    }

    ctx->stats.draw_calls++;
    gfx->draw_indexed(
        ctx->layout, 
        ctx->_vbo, 
        ctx->_ibo, 
        ctx->shader,
        ctx->_indices.size(),
        DATA_TYPE_UINT, 0, DRAW_MODE_TRIANGLES, 
        ctx->target_render_texture
    );
}

void Renderer2D::render(Context* ctx) {
    tm_func();
    ctx->_mutex.lock();
    ST_DEBUG_ASSERT(ctx->_active, "2D Rendering context not active, call begin()");
    ctx->_active = false;
    _flush(ctx);
    ctx->_mutex.unlock();
}

NS_END(graphics);
NS_END(stallout);
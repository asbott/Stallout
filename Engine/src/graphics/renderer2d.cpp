#include "pch.h"

#include "graphics/renderer2d.h"

#include <mz_algorithms.hpp>

struct Camera_Block {
    mz::fmat4 cam_transform;
};

constexpr char vert_shader_source[] = R"(

layout (location = 0) in vec2 pos;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec4 color;
layout (location = 3) in float texture_index;

layout (std140) uniform Camera_Block {
	mat4 cam_transform;
};

out vec2 vs_uv;
out vec4 vs_color;
out flat int vs_texture_index;

void main() {
    vs_uv = uv;
    vs_color = color;
    vs_texture_index = int(texture_index);

    gl_Position = transpose(cam_transform) * vec4(floor(pos.x), floor(pos.y), 0.0, 1.0);
}

)";

constexpr char frag_shader_source[] = R"(

out vec4 output_color;

in vec2 vs_uv;
in vec4 vs_color;
in flat int vs_texture_index;

void main()
{
	if (vs_texture_index == -1) {
        output_color = vs_color;
    } else {
		vec4 texture_color = texture(textures[vs_texture_index], vs_uv);
		output_color = vs_color * texture_color;
        
	}
}

)";

NS_BEGIN(engine);
NS_BEGIN(graphics);

using Context = Renderer2D::Context;

std::unordered_map<Graphics_Driver*, Resource_Handle> default_shaders;

Renderer2D::Renderer2D(Graphics_Driver* gfx)
    : gfx(gfx), max_texture_slots(gfx->get_environment().max_texture_slots) {

    if (!default_shaders.contains(gfx)) {
        default_shaders[gfx] = gfx->create_shader(vert_shader_source, frag_shader_source);
    }

}
Renderer2D::~Renderer2D() {

}

Context* Renderer2D::make_context(size_t hint_quad_count) {
    Context* ctx = stnew (Context);

    ctx->_quad_buffer.reserve(hint_quad_count);
    ctx->_indices.reserve(hint_quad_count * 6);
    ctx->_texture_slots.resize(max_texture_slots);

    ctx->shader = default_shaders[gfx];
    
    ctx->_current_vbo_size = hint_quad_count * sizeof(Quad);
    ctx->_vbo = gfx->create_buffer(BUFFER_TYPE_ARRAY_BUFFER, BUFFER_USAGE_DYNAMIC_DRAW);
    gfx->set_buffer(ctx->_vbo, 0, ctx->_current_vbo_size);

    ctx->_current_ibo_size = hint_quad_count * sizeof(u32) * 6;
    ctx->_ibo = gfx->create_buffer(BUFFER_TYPE_ELEMENT_ARRAY_BUFFER, BUFFER_USAGE_DYNAMIC_DRAW);
    gfx->set_buffer(ctx->_ibo, 0, ctx->_current_ibo_size);

    ctx->_ubo = gfx->create_buffer(BUFFER_TYPE_UNIFORM_BUFFER, BUFFER_USAGE_DYNAMIC_DRAW);
    gfx->set_buffer(ctx->_ubo, 0, sizeof(Camera_Block));

    ctx->_vao = gfx->create_buffer_layout({
        { 2, DATA_TYPE_FLOAT }, // Position
        { 2, DATA_TYPE_FLOAT }, // UV
        { 4, DATA_TYPE_FLOAT }, // Color
        { 1, DATA_TYPE_FLOAT } // Texture index
    });

    ctx->camera.width = 1280;
    ctx->camera.height = 720;

    return ctx;
}
void Renderer2D::free_context(Context*& ctx) {
    ctx->_mutex.lock();

    gfx->destroy(ctx->_vao);
    gfx->destroy(ctx->_vbo);
    gfx->destroy(ctx->_ibo);
    gfx->destroy(ctx->_ubo);

    ctx->_mutex.unlock();

    ST_FREE(ctx, sizeof(Context));
    ctx = NULL;
}

void Renderer2D::new_frame(Context* ctx) {
    tm_func();
    ctx->_mutex.lock();
    ST_DEBUG_ASSERT(!ctx->_active, "2D Rendering context already active");
    ctx->_active = true;

    ctx->_quad_buffer.resize(0);
    for (size_t i = 0; i < ctx->_texture_slots.size(); i++) 
        ctx->_texture_slots[i] = 0;
    ctx->_indices.resize(0);

    ctx->_next_index = 0;
    ctx->_next_texture_index = 0;
    ctx->_bound_textures.clear();

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
        const mz::fvec4& uv) {
    //tm_func();
    ctx->_mutex.lock();
    ST_DEBUG_ASSERT(ctx->_active, "2D Rendering context not active, call begin()");

    s32 texture_index = -1;
    
    if (texture) {
        if (ctx->_bound_textures.contains(texture)) {
            texture_index = ctx->_bound_textures[texture];
        } else {
            if (ctx->_next_texture_index >= ctx->_texture_slots.size()) {

                auto dc = ctx->stats.draw_calls;
                auto q = ctx->stats.quads;
                auto t = ctx->stats.textures;

                this->_flush(ctx);
                ctx->_active = false;
                new_frame(ctx);

                ctx->stats.draw_calls = dc;
                ctx->stats.quads = q;
                ctx->stats.textures = t;
            }

            ctx->stats.textures++;
            texture_index = ctx->_next_texture_index;
            ctx->_texture_slots[ctx->_next_texture_index++] = texture;
            ctx->_bound_textures[texture] = texture_index;
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

        // TODO: #performance Non-trivially slow?
        auto transform = mz::transformation::translation<f32>(position + offset);
        transform.rotate(rotation, { 0, 0, -1 });
        //transform.scale(scale - fvec2(1));
        transform.translate(-pivot);

        vert.position = transform.get_translation();
        vert.color = color;
        vert.texture_coords = (mz::fvec2)uvs[i];
        vert.texture_index = (f32)texture_index;
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
void Renderer2D::draw_quad(Context* ctx, mz::fvec2 position, mz::fvec2 size, float rotation, mz::fvec2 pivot, const mz::color& color) {
    draw(ctx, 0, position, size, rotation, pivot, color);
}

void Renderer2D::_flush(Context* ctx) {
    tm_func();

    Camera_Block cam;
    cam.cam_transform = ctx->camera.get_transform();

    gfx->set_uniform_block(ctx->shader, ctx->_ubo, "Camera_Block", 0);
    gfx->append_to_buffer(ctx->_ubo, 0, &cam);

    size_t new_vbo_size = ctx->_quad_buffer.size() * sizeof(Quad);
    size_t new_ibo_size = ctx->_indices.size() * sizeof(u32);

    bool vbo_resize = ctx->_current_vbo_size < new_vbo_size;
    bool ibo_resize = ctx->_current_ibo_size < new_ibo_size;

    {
        if (vbo_resize) {
            gfx->set_buffer(ctx->_vbo, ctx->_quad_buffer.data(), (size_t)(new_vbo_size * 1.5));
            ctx->_current_vbo_size = (size_t)(new_vbo_size * 1.5);
            ctx->_quad_buffer.reserve((size_t)(new_vbo_size * 1.5) / sizeof(Quad));
            ctx->stats.total_reallocations++;
        } else {
            gfx->append_to_buffer(ctx->_vbo, 0, ctx->_quad_buffer.data(), new_vbo_size);
        }
    }
    {
        if (ibo_resize) {
            gfx->set_buffer(ctx->_ibo, ctx->_indices.data(), (size_t)(new_ibo_size * 1.5));
            ctx->_current_ibo_size = (size_t)(new_ibo_size * 1.5);
            ctx->_indices.reserve((size_t)(new_ibo_size * 1.5) / sizeof(u32));
            ctx->stats.total_reallocations++;
        } else {
            gfx->append_to_buffer(ctx->_ibo, 0, ctx->_indices.data(), new_ibo_size);
        }
    }

    for (u32 i = 0; i < ctx->_texture_slots.size(); i++) {
        if (!ctx->_texture_slots[i]) break;

        gfx->set_texture_slot(i, ctx->_texture_slots[i]);
    }

    // TODO: #unfinished Set state & toggles which may have been changed

    ctx->stats.draw_calls++;
    gfx->draw_indexed(
        ctx->_vao, 
        ctx->_vbo, 
        ctx->_ibo, 
        ctx->shader,
        ctx->_indices.size()
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
NS_END(engine);
#pragma once


#include "Engine/graphics/graphics_driver.h"
#include "Engine/memory.h"
#include "Engine/containers.h"
#include "Engine/graphics/renderer_base.h"

NS_BEGIN(engine);
NS_BEGIN(graphics);

struct ST_API Renderer2D {

    struct Vertex {
        mz::fvec2 position;
        mz::fvec2 texture_coords;
        mz::color color = mz::COLOR_WHITE;
        f32 texture_index;
    };
    struct Quad {
        Vertex vertices[4];
    };
    struct Statistics {
        u32 draw_calls = 0;
        u32 total_reallocations = 0;
        u32 quads = 0;
        u32 textures = 0;
    };
    struct Context {
        Camera_Transform2D camera;
        Resource_Handle shader;
        Statistics stats;

        bool _active = false;

        Resource_Handle _vao, _vbo, _ibo, _ubo;

        size_t _current_vbo_size;
        size_t _current_ibo_size;

        Array<Quad> _quad_buffer;
        Array<Resource_Handle> _texture_slots;
        Hash_Map<Resource_Handle, u32> _bound_textures;
        u32 _next_texture_index = 0;
        Array<u32> _indices;
        u32 _next_index = 0;

        std::mutex _mutex;
    };

    Graphics_Driver* gfx;
    const u32 max_texture_slots;

    Renderer2D(Graphics_Driver* gfx);
    ~Renderer2D();

    // Hint how many quads will be used to avoid buffer reallocations
    Context* make_context(size_t hint_quad_count = 1024);
    void free_context(Context*& ctx);

    void new_frame(Context* ctx);

    void draw(
        Context* ctx, 
        Resource_Handle texture, 
        mz::fvec2 position, 
        mz::fvec2 size, 
        float rotation = 0, 
        mz::fvec2 pivot = 0, 
        const mz::color& color = mz::COLOR_WHITE, 
        const mz::fvec4& uv = mz::fvec4(0, 0, 1, 1)
    );
    void draw_quad(
        Context* ctx, 
        mz::fvec2 position, 
        mz::fvec2 size, 
        float rotation = 0, 
        mz::fvec2 pivot = 0, 
        const mz::color& color = mz::COLOR_WHITE
    );

    void _flush(Context* ctx); // Only for use in render(), not thread safe in itself
    void render(Context* ctx);
};

NS_END(graphics);
NS_END(engine);
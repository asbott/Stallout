#pragma once //


#include "Stallout/graphics/graphics_driver.h"
#include "Stallout/memory.h"
#include "Stallout/containers.h"
#include "Stallout/graphics/renderer_base.h"


NS_BEGIN(stallout);

struct Font;

NS_BEGIN(graphics);

enum Texture2D_Sampling_Type : u8 {
    TEXTURE2D_SAMPLING_TYPE_FLOAT = 0,
    TEXTURE2D_SAMPLING_TYPE_SIGNED = 1,
    TEXTURE2D_SAMPLING_TYPE_UNSIGNED = 2
};

struct ST_API Renderer2D {

    struct Vertex {
        mz::fvec2 position;
        mz::fvec2 texture_coords;
        u32 color;
        f32 texture_index;
        mz::fvec2 center;
        f32 sampling_type;
        f32 is_text;
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
        Resource_Handle layout;
        Resource_Handle target_render_texture = 0;
        Statistics stats;
        mz::ivec4 viewport;

        bool _active = false;

        Resource_Handle _default_layout, _vbo, _ibo, _ubo, _default_shader;
        
        size_t _current_vbo_size;
        size_t _current_ibo_size;

        Array<Quad> _quad_buffer;
        Array<Resource_Handle> _texture_slots;
        u32 _next_texture_index = 0;
        Array<u32> _indices;
        u32 _next_index = 0;

        std::mutex _mutex;
    };

    Graphics_Driver* gfx;
    std::mutex gfx_mutex;
    const u32 max_texture_slots;

    Renderer2D(Graphics_Driver* gfx);
    ~Renderer2D();

    // Hint how many quads will be used to avoid buffer reallocations
    Context* make_context(mz::ivec4 viewport, size_t hint_quad_count = 1024);
    void free_context(Context*& ctx);

    void _begin(Context* ctx);
    void new_frame(Context* ctx);

    void draw(
        Context* ctx, 
        Resource_Handle texture, 
        mz::fvec2 position, 
        mz::fvec2 size, 
        float rotation = 0, 
        mz::fvec2 pivot = 0, 
        const mz::color& color = mz::COLOR_WHITE, 
        const mz::fvec4& uv = mz::fvec4(0, 0, 1, 1),
        const AABB& skew = AABB(),
        Texture2D_Sampling_Type = TEXTURE2D_SAMPLING_TYPE_FLOAT,
        bool is_text = false
    );
    void draw_quad(
        Context* ctx, 
        mz::fvec2 position, 
        mz::fvec2 size, 
        float rotation = 0, 
        mz::fvec2 pivot = 0, 
        const mz::color& color = mz::COLOR_WHITE,
        const AABB& skew = AABB()
    );

    void _flush(Context* ctx); // Only for use in render(), not thread safe in itself
    void render(Context* ctx);
};

NS_END(graphics);
NS_END(stallout);
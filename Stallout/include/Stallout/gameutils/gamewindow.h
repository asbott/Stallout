#pragma once //

#include "Stallout/os/oswindow.h"
#include "Stallout/graphics/renderer2d.h"
#include "Stallout/graphics/primitive_renderer2d.h"
#include "Stallout/gameutils/drawing.h"

NS_BEGIN(stallout);

NS_BEGIN(graphics);
struct ImGui_Renderer;
NS_END(graphics);

struct Texture2D;

struct ST_API Game_Window {

    os::Window* _os_window;
    graphics::Graphics_Driver* _gfx_driver;
    graphics::Renderer2D* _renderer;
    graphics::Renderer2D::Context* _rend_ctx;
    graphics::Primitive_Renderer2D* _prim_rend;
    graphics::Primitive_Renderer2D::Context* _prim_rend_ctx;
    gfx::ImGui_Renderer* _imgui_rend;
    gfx::Resource_Handle _rend_tex;

    mz::color clear_color = mz::color(0.686f, 0.275f, 0.353f, 1.f);
    gfx::Camera_Transform2D camera;
    bool render_to_texture = false;
    

    Game_Window(const char* title = "Stallout Game", u32 width = 1280, u32 height = 720);
    Game_Window(const os::Window_Init_Spec& spec);
    void _init();

    ~Game_Window();

    void new_frame(f32 delta_seconds);
    void render();

    void draw_texture(
        Texture2D* texture, 
        mz::fvec2 position, 
        mz::fvec2 size = -1, 
        float rotation = 0, 
        mz::fvec2 pivot = 0, 
        const mz::color& color = mz::COLOR_WHITE,
        const mz::fvec4& source_rect = mz::fvec4(0, 0, 1, 1),
        gfx::Texture2D_Sampling_Type sampling = gfx::TEXTURE2D_SAMPLING_TYPE_FLOAT
    );
    void draw_texture(
        gfx::Resource_Handle htexture, 
        mz::fvec2 position, 
        mz::fvec2 size, 
        float rotation = 0, 
        mz::fvec2 pivot = 0, 
        const mz::color& color = mz::COLOR_WHITE,
        const mz::fvec4& source_rect = mz::fvec4(0, 0, 1, 1),
        gfx::Texture2D_Sampling_Type sampling = gfx::TEXTURE2D_SAMPLING_TYPE_FLOAT
    );
    
    void draw_sprite(
        Sprite* sprite, 
        mz::fvec2 position, 
        mz::fvec2 size = 0, 
        float rotation = 0, 
        mz::fvec2 pivot = 0, 
        const mz::color& color = mz::COLOR_WHITE
    );
    void draw_quad(
        mz::fvec2 position, 
        mz::fvec2 size, 
        float rotation = 0,
        mz::fvec2 pivot = 0, 
        const mz::color& color = mz::COLOR_WHITE
    );

    template <typename char_t>
    void draw_text(
        Font* font, 
        const char_t* text, 
        mz::fvec2 position,
        mz::fvec2 scale = 1, 
        float rotation = 0, 
        mz::fvec2 pivot = 0, 
        const mz::color& color = mz::COLOR_WHITE,
        bool kerning = true
    ) {
        const char_t* p = text;
        (void)rotation; // TODO
        LString lstr(text);
        f32 x = position.x;
        while (*p != 0) {
            char_t c = *p++;

            auto cinfo = font->_get_char_info(c);
            auto atlas = font->_get_atlas(c);

            if (!cinfo.has_bitmap) {
                cinfo = font->_get_char_info(font->PLACEHOLDER_CHAR);
                atlas = font->_get_atlas(font->PLACEHOLDER_CHAR);
            }

            mz::fvec2 kern = 0;

            if (kerning && *p) {
                kern = font->get_kerning(c, *p) * scale;
            }

            mz::fvec2 offset = (mz::fvec2)cinfo.offset * scale;

            mz::fvec2 pos = {
                x + offset.x + kern.x,
                position.y + offset.y + kern.y
            };

            _renderer->draw(_rend_ctx, atlas.texture, pos, (mz::fvec2)cinfo.size * scale, 0, pivot, color, font->get_uv(c), gfx::AABB(), gfx::TEXTURE2D_SAMPLING_TYPE_FLOAT, true);
            x += cinfo.advance * scale.x;
        }
    }

    void draw_text(
        Font* font, 
        const LString& text, 
        mz::fvec2 position,
        mz::fvec2 scale = 1, 
        float rotation = 0, 
        mz::fvec2 pivot = 0, 
        const mz::color& color = mz::COLOR_WHITE,
        bool kerning = true
    ) {
        draw_text<lchar>(font, text.str, position, scale, rotation, pivot, color, kerning);
    }
    void draw_text(
        Font* font, 
        const String& text, 
        mz::fvec2 position,
        mz::fvec2 scale = 1, 
        float rotation = 0, 
        mz::fvec2 pivot = 0, 
        const mz::color& color = mz::COLOR_WHITE,
        bool kerning = true
    ) {
        draw_text<char>(font, text.str, position, scale, rotation, pivot, color, kerning);
    }

    void debug_draw_line(mz::fvec2 a, mz::fvec2 b, const mz::color& color = mz::COLOR_WHITE);
    void debug_draw_arrow(mz::fvec2 origin, mz::fvec2 dir, const mz::color& color = mz::COLOR_WHITE);
    void debug_draw_wire_frame(mz::frect rect, const mz::color& color = mz::COLOR_WHITE);
    void debug_draw_wire_triangle(mz::fvec2 tri[3], const mz::color& color = mz::COLOR_WHITE);
    void debug_draw_circle(mz::fvec2 center, f32 radius, const mz::color& color = mz::COLOR_WHITE);

    bool is_input_down(os::Input_Code code);
    mz::fvec2 get_mouse_pos() const;
    mz::fvec2 get_size() const;
    mz::fvec4 get_viewport() const { 
        return mz::fvec4(mz::fvec2(0), get_size()); 
    }

    bool should_close() const { return _os_window->exit_flag(); }
};

NS_END(stallout);
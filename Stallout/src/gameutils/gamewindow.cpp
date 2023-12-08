#include "pch.h"

#include "Stallout/gameutils/gamewindow.h"

#include "os/graphics.h"

#include "graphics/imgui_renderer.h"

NS_BEGIN(stallout);

Game_Window::Game_Window(const char* title, u32 width, u32 height) {
    os::Window_Init_Spec spec;
    spec.title = title;
    spec.width = (size_t)width;
    spec.height = (size_t)height;
    _os_window = stnew(os::Window) (spec);
    _init();
}
Game_Window::Game_Window(const os::Window_Init_Spec& spec) {
    _os_window = stnew(os::Window) (spec);
    _init();
}
void Game_Window::_init() {
    _gfx_driver = stnew(graphics::Graphics_Driver) (_os_window->get_device_context());

    mz::ivec2 wnd_sz;
    _os_window->get_size(&wnd_sz.x, &wnd_sz.y);

    _renderer = stnew(graphics::Renderer2D) (_gfx_driver);
    _rend_ctx = _renderer->make_context({0, 0, wnd_sz.x, wnd_sz.y});

    _prim_rend = stnew(graphics::Primitive_Renderer2D) (_gfx_driver);
    _prim_rend_ctx = _prim_rend->make_context({0, 0, wnd_sz.x, wnd_sz.y});


    camera.size = wnd_sz;

    
    _gfx_driver->set_blending(
        gfx::BLEND_EQUATION_ADD,
        gfx::BLEND_FACTOR_SRC_ALPHA,
        gfx::BLEND_FACTOR_ONE_MINUS_SRC_ALPHA
    );


    _rend_tex = _gfx_driver->create_render_texture(
        wnd_sz.x, wnd_sz.y, 4, gfx::TEXTURE2D_FORMAT_RGBA,
        gfx::DATA_TYPE_FLOAT, gfx::TEXTURE2D_FORMAT_RGBA
    );

    imgui::init(_gfx_driver, _os_window);
    _imgui_rend = imgui::get_renderer();
}
Game_Window::~Game_Window() {
    _prim_rend->free_context(_prim_rend_ctx);
    _renderer->free_context(_rend_ctx);

    ST_DELETE(_prim_rend);
    ST_DELETE(_renderer);
    ST_DELETE(_gfx_driver);
    ST_DELETE(_os_window);
}

void Game_Window::new_frame(f32 delta_seconds) {
    tm_func();

    _gfx_driver->clear(graphics::CLEAR_FLAG_COLOR, clear_color, render_to_texture ? _rend_tex : 0);

    _renderer->new_frame(_rend_ctx);
    _prim_rend->new_frame(_prim_rend_ctx);

    imgui::set_renderer(_imgui_rend);
    imgui::new_frame(delta_seconds);

    _os_window->poll_events();
}
void Game_Window::render() {
    tm_func();

    _gfx_driver->set_blending(
        gfx::BLEND_EQUATION_ADD,
        gfx::BLEND_FACTOR_SRC_ALPHA,
        gfx::BLEND_FACTOR_ONE_MINUS_SRC_ALPHA
    );

    _prim_rend_ctx->camera = _rend_ctx->camera = this->camera;

    if (render_to_texture) {
        _rend_ctx->target_render_texture = _rend_tex;
        _renderer->render(_rend_ctx);

        _rend_ctx->target_render_texture = 0;
        _rend_ctx->camera.position = 0;
        _rend_ctx->camera.rotation = 0;
        _rend_ctx->camera.zoom = 0;
        _gfx_driver->clear(graphics::CLEAR_FLAG_COLOR, clear_color);
        _renderer->new_frame(_rend_ctx);
        _renderer->draw(_rend_ctx, _rend_tex, 0, get_size(), 0, get_size() / 2.f, mz::COLOR_WHITE, { 0, 1, 1, 0 });
    }
    _renderer->render(_rend_ctx);

    _prim_rend->render(_prim_rend_ctx);

    imgui::set_renderer(_imgui_rend);
    imgui::render();

    
    _gfx_driver->swap_render_buffers();

    _gfx_driver->sync();
    _gfx_driver->swap_command_buffers();
}

void Game_Window::draw_texture(
    Texture2D* texture, 
    mz::fvec2 position, 
    mz::fvec2 size, 
    float rotation, 
    mz::fvec2 pivot, 
    const mz::color& color,
    const mz::fvec4& source_rect,
    gfx::Texture2D_Sampling_Type sampling
) {
    _renderer->draw(_rend_ctx, texture->_handle, position, size.magnitude() >= 0 ? size : (mz::fvec2)texture->_size, rotation, pivot, color, source_rect, gfx::AABB(), sampling);
}
void Game_Window::draw_texture(
    gfx::Resource_Handle htexture, 
    mz::fvec2 position, 
    mz::fvec2 size, 
    float rotation, 
    mz::fvec2 pivot, 
    const mz::color& color,
    const mz::fvec4& source_rect,
    gfx::Texture2D_Sampling_Type sampling
) {
    _renderer->draw(_rend_ctx, htexture, position, size, rotation, pivot, color, source_rect, gfx::AABB(), sampling);
}

void Game_Window::draw_sprite(
    Sprite* sprite, 
    mz::fvec2 position, 
    mz::fvec2 size, 
    float rotation, 
    mz::fvec2 pivot, 
    const mz::color& color
) {
    _renderer->draw(_rend_ctx, sprite->get_texture()->_handle, position, size.magnitude() > 0 ? size : sprite->get_size(), rotation, pivot, color, sprite->get_uv());
}

void Game_Window::draw_quad(
    mz::fvec2 position, 
    mz::fvec2 size, 
    float rotation,
    mz::fvec2 pivot, 
    const mz::color& color
) {
    _renderer->draw_quad(_rend_ctx, position, size, rotation, pivot, color);
}

void Game_Window::debug_draw_line(mz::fvec2 a, mz::fvec2 b, const mz::color& color) {
    _prim_rend->draw_line(_prim_rend_ctx, a, b, color);
}
void Game_Window::debug_draw_arrow(mz::fvec2 origin, mz::fvec2 dir, const mz::color& color) {
    _prim_rend->draw_line(_prim_rend_ctx, origin, origin + dir, color);

    mz::fvec2 end = origin + dir;

    f32 rad = (f32)math::atan2(dir.y, dir.x) + math::PI32;

    f32 divergence = math::PI32 / 6.f;

    f32 arrowhead_length = 10.0f;

    mz::fvec2 arrowhead_end1 = end + mz::fvec2(cos(rad + divergence) * arrowhead_length, sin(rad + divergence) * arrowhead_length);
    mz::fvec2 arrowhead_end2 = end + mz::fvec2(cos(rad - divergence) * arrowhead_length, sin(rad - divergence) * arrowhead_length);

    _prim_rend->draw_line(_prim_rend_ctx, end, arrowhead_end1, color);
    _prim_rend->draw_line(_prim_rend_ctx, end, arrowhead_end2, color);
}

void Game_Window::debug_draw_wire_frame(mz::frect rect, const mz::color& color) {
    auto L = rect.x;
    auto R = rect.z;
    auto T = rect.w;
    auto B = rect.y;

    _prim_rend->draw_line(_prim_rend_ctx, mz::fvec4(L, B, L, T), color);
    _prim_rend->draw_line(_prim_rend_ctx, mz::fvec4(L, T, R, T), color);
    _prim_rend->draw_line(_prim_rend_ctx, mz::fvec4(R, T, R, B), color);
    _prim_rend->draw_line(_prim_rend_ctx, mz::fvec4(R, B, L, B), color);

    _prim_rend->draw_line(_prim_rend_ctx, mz::fvec4(L, B, R, T), color);
}

void Game_Window::debug_draw_wire_triangle(mz::fvec2 tri[3], const mz::color& color) {
    for (size_t i = 0; i < 3; i++) {
        _prim_rend->draw_line(_prim_rend_ctx, mz::fvec4(tri[i], tri[i == 2 ? 0 : i + 1]), color);
    }
}

void Game_Window::debug_draw_circle(mz::fvec2 center, f32 radius, const mz::color& color) {
    _prim_rend->draw_circle(_prim_rend_ctx, center, radius, color);
}

bool Game_Window::is_input_down(os::Input_Code code) {
    return _os_window->input.get_state(code) == os::INPUT_STATE_DOWN;
}

mz::fvec2 Game_Window::get_mouse_pos() const {
    return mz::fvec2((f32)_os_window->input._mouse_x, get_size().y - (f32)_os_window->input._mouse_y);
}

mz::fvec2 Game_Window::get_size() const {
    mz::s32vec2 sz;
    _os_window->get_size(&sz.x, &sz.y);

    return sz;
}

NS_END(stallout);
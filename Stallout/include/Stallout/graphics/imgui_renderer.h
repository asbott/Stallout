#pragma once //

#include "Stallout/graphics/graphics_driver.h"
#include "os/oswindow.h"

#define ST_USE_CUSTOM_IMGUI_BACKEND

NS_BEGIN(stallout);
NS_BEGIN(os);
struct Window;
NS_END(os);
NS_END(stallout);

NS_BEGIN(stallout);
NS_BEGIN(graphics);

struct Mouse_Leave_Event {
    bool in_client;
};

struct _Renderer_State_Backup {
    Renderer_Setting_Flags settings;
    Blend_Equation blend_eq;
    Blend_Func_Factor src_col, dst_col, src_alpha, dst_alpha;
    Polygon_Mode poly_mode_front, poly_mode_back;
    Resource_Handle last_bound_tex = 0;
};

struct Graphics_Driver;

// Currently only works with one instance because it doesn't
// set imgui state also absolutely is not thread safe
struct ST_API ImGui_Renderer {
    Graphics_Driver* _context;
    void* _imgui_context;
    os::Window* _mouse_window;
#ifdef ST_USE_CUSTOM_IMGUI_BACKEND
    bool _want_update_monitors = false;
    s32 mouse_buttons_down;
    s32 _last_mouse_cursor = 0;
    int _mouse_tracked_area = 0;
    _Renderer_State_Backup _state_backup;
    os::Window* _main_window;
    size_t vbo_size = 0;
    size_t ibo_size = 0;

    Resource_Handle _hvbo    = 0, _hibo          = 0, _hlayout = 0;
    Resource_Handle _hshader = 0, _hfont_texture = 0, _hubo_frag = 0;
    Resource_Handle _hubo_vert = 0;

    void _update_monitors();
    void _update_mouse_data();

    bool _init_for_window_system();
    void _shutdown_for_window_system();
    void _new_frame_for_window_system(f32 delta_time);

    bool _init_for_renderer();
    void _shutdown_for_renderer();
    void _new_frame_for_renderer(f32 delta_time);
    void _render_draw_data(void* draw_data);
    void _prepare_draw_call(void* draw_data, s32 fb_width, s32 fb_height);
#endif

    ImGui_Renderer(Graphics_Driver* renderer, os::Window* main_window);
    ~ImGui_Renderer();
    void new_frame(f32 delta_time);
    void render();
    bool handle_window_event(os::Window* window, os::Window_Event_Type etype, void* param, void* );

    ////////////////// Widgets


};

NS_END(graphics);
NS_END(stallout);
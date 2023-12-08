#pragma once //

NS_BEGIN(stallout);
NS_BEGIN(os);
NS_BEGIN(env);

enum Mouse_Cursor {
    MOUSE_CURSOR_ARROW,
    MOUSE_CURSOR_TEXTINPUT,
    MOUSE_CURSOR_RESIZEALL,
    MOUSE_CURSOR_RESIZEEW,
    MOUSE_CURSOR_RESIZENS,
    MOUSE_CURSOR_RESIZENESW,
    MOUSE_CURSOR_RESIZENWSE,
    MOUSE_CURSOR_HAND,
    MOUSE_CURSOR_NOTALLOWED,
    MOUSE_CURSOR_NONE
};

struct Cache_Size {
    size_t L1, L2, L3;
};

struct Monitor {
    u32 main_left, main_top, main_right, main_bottom;
    u32 work_left, work_top, work_right, work_bottom;

    bool is_primary;

    void* handle;
};

// Per thread
ST_API Cache_Size get_cache_size();

void ST_API get_monitors(Monitor*& monitors, size_t* num_monitors);
float ST_API get_monitor_dpi(void* monitor);

void ST_API set_mouse_cursor(Mouse_Cursor cursor);

// Is any of the windows owned by this process focused?
bool ST_API is_app_focused();

void ST_API set_cursor_pos(s32 x, s32 y);

NS_END(env);
NS_END(os);
NS_END(stallout)
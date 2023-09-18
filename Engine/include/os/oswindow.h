#pragma once 

#include "Engine/containers.h"



NS_BEGIN(os)

// Custom keycodes, converted in backend inplementations
// Keys AND mouse buttons
enum Input_Code {
    INPUT_CODE_A,
    INPUT_CODE_B,
    INPUT_CODE_C,
    INPUT_CODE_D,
    INPUT_CODE_E,
    INPUT_CODE_F,
    INPUT_CODE_G,
    INPUT_CODE_H,
    INPUT_CODE_I,
    INPUT_CODE_J,
    INPUT_CODE_K,
    INPUT_CODE_L,
    INPUT_CODE_M,
    INPUT_CODE_N,
    INPUT_CODE_O,
    INPUT_CODE_P,
    INPUT_CODE_Q,
    INPUT_CODE_R,
    INPUT_CODE_S,
    INPUT_CODE_T,
    INPUT_CODE_U,
    INPUT_CODE_V,
    INPUT_CODE_W,
    INPUT_CODE_X,
    INPUT_CODE_Y,
    INPUT_CODE_Z,

    INPUT_CODE_0,
    INPUT_CODE_1,
    INPUT_CODE_2,
    INPUT_CODE_3,
    INPUT_CODE_4,
    INPUT_CODE_5,
    INPUT_CODE_6,
    INPUT_CODE_7,
    INPUT_CODE_8,
    INPUT_CODE_9,

    INPUT_CODE_F1,
    INPUT_CODE_F2,
    INPUT_CODE_F3,
    INPUT_CODE_F4,
    INPUT_CODE_F5,
    INPUT_CODE_F6,
    INPUT_CODE_F7,
    INPUT_CODE_F8,
    INPUT_CODE_F9,
    INPUT_CODE_F10,
    INPUT_CODE_F11,
    INPUT_CODE_F12,

    INPUT_CODE_SPACE,
    INPUT_CODE_TAB,
    INPUT_CODE_ENTER,
    INPUT_CODE_ESCAPE,
    INPUT_CODE_BACKSPACE,
    INPUT_CODE_SHIFT,
    INPUT_CODE_CTRL,
    INPUT_CODE_ALT,

    INPUT_CODE_COMMA,
    INPUT_CODE_PERIOD,
    INPUT_CODE_SEMICOLON,
    INPUT_CODE_APOSTROPHE,
    INPUT_CODE_BRACKET_OPEN,
    INPUT_CODE_BRACKET_CLOSE,
    INPUT_CODE_BACKSLASH,
    INPUT_CODE_SLASH,
    INPUT_CODE_MINUS,
    INPUT_CODE_EQUALS,

    INPUT_CODE_MOUSE_LEFT,
    INPUT_CODE_MOUSE_RIGHT,
    INPUT_CODE_MOUSE_MIDDLE,
    INPUT_CODE_MOUSE_BUTTON4,
    INPUT_CODE_MOUSE_BUTTON5,

    INPUT_CODE_COUNT
};

enum Input_State {
    INPUT_STATE_UP,
    INPUT_STATE_DOWN,
};

u64 to_os_input_code(Input_Code code); // Implemented in backend

struct Window_Init_Spec {
    size_t width = 1280;
    size_t height = 720;
    bool fullscreen = false;
    const char* title = "Stallout";
    bool visible = true;
};

struct Window {
    Window(const Window_Init_Spec& wnd_spec, Window* parent = NULL);
    ~Window();

    void resize(size_t width, size_t height);
    void move_to(size_t x, size_t y);

    void set_fullscreen(bool fullscreen);
    void set_title(const char* title);
    void set_focus(bool focused);
    void set_visibility(bool visible);

    Window* add_child(const Window_Init_Spec& wnd_spec);
    Window* get_child(size_t index);
    size_t get_child_count();

    bool exit_flag() const;

    void swap_buffers();

    struct Input {
        Input_State get_state(Input_Code code) const;

        double _mouse_x = 0;
        double _mouse_y = 0;

        Input_State _input_states[INPUT_CODE_COUNT];
    } input;

    void* _os_handle;
    bool _is_main;
    engine::Array<Window*> _children;
    void* __internal;
};

NS_END(os);
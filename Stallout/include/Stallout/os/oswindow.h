#pragma once // 

#include "Stallout/containers.h"

// TODO (2023-12-03) #bug #window
// When running multiple windows it seems like event
// callbacks get messed up. Maybe static variable in
// the wnd proc function ?

NS_BEGIN(stallout);
NS_BEGIN(os);

NS_BEGIN(graphics);
struct Device_Context;
NS_END(graphics);

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

    INPUT_CODE_NUMPAD0,
    INPUT_CODE_NUMPAD1,
    INPUT_CODE_NUMPAD2,
    INPUT_CODE_NUMPAD3,
    INPUT_CODE_NUMPAD4,
    INPUT_CODE_NUMPAD5,
    INPUT_CODE_NUMPAD6,
    INPUT_CODE_NUMPAD7,
    INPUT_CODE_NUMPAD8,
    INPUT_CODE_NUMPAD9,

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
    INPUT_CODE_NUMPAD_ENTER,
    INPUT_CODE_ESCAPE,
    INPUT_CODE_BACKSPACE,
    INPUT_CODE_CTRL,
    INPUT_CODE_ALT,

    INPUT_CODE_LSHIFT,
    INPUT_CODE_LCTRL,
    INPUT_CODE_LALT,
    INPUT_CODE_LSUPER,
    INPUT_CODE_RSHIFT,
    INPUT_CODE_RCTRL,
    INPUT_CODE_RALT,
    INPUT_CODE_RSUPER,

    INPUT_CODE_COMMA,
    INPUT_CODE_PERIOD,
    INPUT_CODE_SEMICOLON,
    INPUT_CODE_APOSTROPHE,
    INPUT_CODE_BRACKET_OPEN,
    INPUT_CODE_BRACKET_CLOSE,
    INPUT_CODE_BACKSLASH,
    INPUT_CODE_SLASH,
    INPUT_CODE_MINUS,
    INPUT_CODE_PLUS,
    INPUT_CODE_EQUALS,

    INPUT_CODE_DECIMAL,
    INPUT_CODE_DIVIDE,
    INPUT_CODE_MULTIPLY,
    INPUT_CODE_SUBTRACT,
    INPUT_CODE_ADD,

    INPUT_CODE_MOUSE_LEFT,
    INPUT_CODE_MOUSE_RIGHT,
    INPUT_CODE_MOUSE_MIDDLE,
    INPUT_CODE_MOUSE_BUTTON4,
    INPUT_CODE_MOUSE_BUTTON5,

    INPUT_CODE_LEFT_ARROW,
    INPUT_CODE_RIGHT_ARROW,
    INPUT_CODE_UP_ARROW,
    INPUT_CODE_DOWN_ARROW,

    INPUT_CODE_PAGEUP,
    INPUT_CODE_PAGEDOWN,

    INPUT_CODE_HOME,
    INPUT_CODE_END,
    INPUT_CODE_INSERT,
    INPUT_CODE_DELETE,
    INPUT_CODE_BACK,

    INPUT_CODE_GRAVEACCENT,
    INPUT_CODE_LEFT_BRACKET,
    INPUT_CODE_RIGHT_BRACKET,

    INPUT_CODE_CAPSLOCK,
    INPUT_CODE_SCROLL,
    INPUT_CODE_NUMLOCK,
    INPUT_CODE_SNAPSHOT,
    INPUT_CODE_PAUSE,



    INPUT_CODE_COUNT,

    INPUT_CODE_MOUSE_SIDE1 = INPUT_CODE_MOUSE_BUTTON4,
    INPUT_CODE_MOUSE_SIDE2 = INPUT_CODE_MOUSE_BUTTON5,
    INPUT_CODE_OEM_1 = INPUT_CODE_SEMICOLON,
    INPUT_CODE_OEM_2 = INPUT_CODE_SLASH,
    INPUT_CODE_OEM_3 = INPUT_CODE_GRAVEACCENT,
    INPUT_CODE_OEM_4 = INPUT_CODE_LEFT_BRACKET,
    INPUT_CODE_OEM_5 = INPUT_CODE_BACKSLASH,
    INPUT_CODE_OEM_6 = INPUT_CODE_RIGHT_BRACKET,
    INPUT_CODE_OEM_7 = INPUT_CODE_APOSTROPHE,

    INPUT_CODE_UNKNOWN
};

enum Input_State {
    INPUT_STATE_UP,
    INPUT_STATE_DOWN
};

bool ST_API is_input_down(Input_Code input);

enum Window_Style {
    WINDOW_STYLE_UNSET = 0,
    WINDOW_STYLE_OVERLAPPED = BIT(1),
    WINDOW_STYLE_POPUP = BIT(2),
    WINDOW_STYLE_CHILD = BIT(3),
    WINDOW_STYLE_MINIMIZE = BIT(4),
    WINDOW_STYLE_MAXIMIZE = BIT(5),
    WINDOW_STYLE_BORDER = BIT(6),
    WINDOW_STYLE_CAPTION = BIT(7),
    WINDOW_STYLE_SYSMENU = BIT(8),
    WINDOW_STYLE_THICKFRAME = BIT(9),
    WINDOW_STYLE_MINIMIZEBOX = BIT(10),
    WINDOW_STYLE_MAXIMIZEBOX = BIT(11),
    WINDOW_STYLE_VISIBLE = BIT(12),
    WINDOW_STYLE_DISABLED = BIT(13),
    WINDOW_STYLE_OVERLAPPEDWINDOW = BIT(14),
};
FLAGIFY(Window_Style);

enum Window_Style_Ex {
    WINDOW_STYLE_EX_UNSET = 0,
    WINDOW_STYLE_EX_TOOLWINDOW = BIT(1),
    WINDOW_STYLE_EX_APPWINDOW = BIT(2),
    WINDOW_STYLE_EX_TOPMOST = BIT(3),
};
FLAGIFY(Window_Style_Ex);

typedef u32 Window_Event_Type;
enum Window_Event_Type_ : Window_Event_Type {
    WINDOW_EVENT_TYPE_UNIMPLEMENTED = 0,


    WINDOW_EVENT_TYPE_CLOSE,
    WINDOW_EVENT_TYPE_MOVE,
    WINDOW_EVENT_TYPE_SIZE,
    WINDOW_EVENT_TYPE_MOUSEACTIVATE,
    WINDOW_EVENT_TYPE_NCHITTEST,

    WINDOW_EVENT_TYPE_MOUSE_SCROLL,
    WINDOW_EVENT_TYPE_MOUSEMOVE,
    /*WINDOW_EVENT_TYPE_MOUSELEAVE,
    WINDOW_EVENT_TYPE_NCMOUSELEAVE,*/

    /*WINDOW_EVENT_TYPE_LBUTTONDOWN,
    WINDOW_EVENT_TYPE_RBUTTONDOWN,
    WINDOW_EVENT_TYPE_MBUTTONDOWN,
    WINDOW_EVENT_TYPE_XBUTTONDOWN,
    WINDOW_EVENT_TYPE_LBUTTONDBLCLK,
    WINDOW_EVENT_TYPE_RBUTTONDBLCLK,
    WINDOW_EVENT_TYPE_MBUTTONDBLCLK,
    WINDOW_EVENT_TYPE_XBUTTONDBLCLK,
    WINDOW_EVENT_TYPE_LBUTTONUP,
    WINDOW_EVENT_TYPE_RBUTTONUP,
    WINDOW_EVENT_TYPE_MBUTTONUP,
    WINDOW_EVENT_TYPE_XBUTTONUP,*/
    WINDOW_EVENT_TYPE_MOUSE_BUTTON,

    WINDOW_EVENT_TYPE_KEY,

    WINDOW_EVENT_TYPE_CHAR,

    WINDOW_EVENT_TYPE_SETFOCUS,
    WINDOW_EVENT_TYPE_KILLFOCUS,

    WINDOW_EVENT_TYPE_SETCURSOR,

    WINDOW_EVENT_TYPE_DEVICECHANGE,
    WINDOW_EVENT_TYPE_DISPLAYCHANGE,



    WINDOW_EVENT_TYPE_MAX,
};

enum Mouse_Source {
    MOUSE_SOURCE_PEN,
    MOUSE_SOURCE_TOUCHSCREEN,
    MOUSE_SOURCE_MOUSE
};

enum Scroll_Axis {
    SCROLL_AXIS_VERTICAL,
    SCROLL_AXIS_HORIZONTAL
};

/*
LPARAM extra_info = ::GetMessageExtraInfo();
if ((extra_info & 0xFFFFFF80) == 0xFF515700)
    return ImGuiMouseSource_Pen;
if ((extra_info & 0xFFFFFF80) == 0xFF515780)
    return ImGuiMouseSource_TouchScreen;
return ImGuiMouseSource_Mouse;
*/

struct Mouse_Move_Event{
    s32 window_mouse_x, window_mouse_y;
    s32 screen_mouse_x, screen_mouse_y;
    Mouse_Source mouse_source;
    bool in_window;
};
struct Mouse_Button_Event{
    Input_Code button;
    Input_State state;
    bool double_click;
    Mouse_Source mouse_source;
};

struct Mouse_Scroll_Event {
    float delta;
    Scroll_Axis axis;
    Mouse_Source mouse_source;
};

struct Key_Event {
    Input_Code key;
    Input_State state;
    bool is_sys_key;
};

struct Char_Event {
    char value;
};

struct Setcursor_Event {
    bool in_client;
};

u64 to_os_input_code(Input_Code code); // Implemented in backend


// TODO: #performance #memory
// Should do custom allocation. It's fine here
// but in os implementations this is used to
// create every single event sent from the OS
// and free after dispatching is done. Could
// probably sync this with swap_buffers or a
// different function
/*#define __NEW_EVENT(x, ...) ST_NEW(x) (__VA_ARGS__)
#define __DELETE_EVENT(x) ST_DELETE(x)
#define __COPY_EVENT(e, t) __NEW_EVENT(t, *(t*)e)*/

// TODO: #unfinished
// Should handle a lot more events than this.
// copy_window_event, free_window event and
// event_from_**** in os implementations

// Caller is responsible for freeing with free_window_event()
/*inline void* copy_window_event(void* event, Window_Event_Type etype) {
    switch(etype) {
        case WINDOW_EVENT_TYPE_MOUSEMOVE: 
            return __COPY_EVENT(event, Mouse_Move_Event);
        case WINDOW_EVENT_TYPE_MOUSE_BUTTON:
            return __COPY_EVENT(event, Mouse_Button_Event);
        case WINDOW_EVENT_TYPE_MOUSE_SCROLL:
            return __COPY_EVENT(event, Mouse_Scroll_Event);
        case WINDOW_EVENT_TYPE_KEY:
            return __COPY_EVENT(event, Key_Event);
        case WINDOW_EVENT_TYPE_CHAR:
            return __COPY_EVENT(event, Char_Event);
        case WINDOW_EVENT_TYPE_SETCURSOR:
            return __COPY_EVENT(event, Setcursor_Event);

        default: return 0;
    }
}*/



struct Window_Init_Spec {
    s32 x = 0, y = 0;
    size_t width = 970;
    size_t height = 540;
    bool fullscreen = false;
    const char* title = "Stallout";
    bool visible = true;

    bool size_includes_styles = true;


    Window_Style style = WINDOW_STYLE_CAPTION | WINDOW_STYLE_SYSMENU | WINDOW_STYLE_MINIMIZEBOX;
    Window_Style_Ex style_ex = WINDOW_STYLE_EX_UNSET;
};

struct Window;

typedef std::function<bool(Window*, Window_Event_Type, void* param, void* userdata)> window_event_callback_t;




// TODO: #testing #threading
// Const functions are meant to be thread-safe
// but this is not tested
struct ST_API Window {

    struct _Event {
        Window_Event_Type etype;
        void* param;
    };

    Window(const Window_Init_Spec& wnd_spec, Window* parent = NULL);
    ~Window();

    void add_event_callback(window_event_callback_t, void* userdata = NULL);    

    void set_size(s32 width, s32 height);
    bool get_size(s32* width, s32* height) const;
    void set_position(s32 x, s32 y);
    bool get_position(s32* x, s32* u) const;

    void set_fullscreen(bool fullscreen);
    void set_title(const char* title);
    void set_focus(bool focused);
    void set_visibility(bool visible);
    void set_alpha(float alpha);

    void set_parent(Window* parent);
    Window* add_child(const Window_Init_Spec& wnd_spec);
    Window* get_child(size_t index);
    size_t get_child_count();

    bool exit_flag() const;
    bool is_focused() const;
    bool is_visible() const;
    bool is_minimized() const;
    bool is_hovered() const;
    void* get_monitor() const; 
    graphics::Device_Context* get_device_context() const;

    bool screen_to_client(s32* x, s32* y) const;
    bool client_to_screen(s32* x, s32* y) const;

    bool has_captured_mouse() const;
    void capture_mouse();
    void release_mouse();    

    bool dispatch_event(Window_Event_Type etype, void* param);

    void poll_events();
    void swap_buffers();

    struct ST_API Input {
        Input_State get_state(Input_Code code) const;

        double _mouse_x = 0;
        double _mouse_y = 0;

        Input_State _input_states[INPUT_CODE_COUNT];
    } input;

    void* _os_handle;
    graphics::Device_Context* _device_context;
    bool _is_main;
    Window* _parent;
    stallout::Array<Window*> _children;
    Window_Style _current_style;
    Window_Style_Ex _current_style_ex;
    void* __internal;

    struct Callback {
        window_event_callback_t fn;
        void* ud = NULL;
    };
    stallout::Array<Callback> event_callbacks;
};

NS_END(os);
NS_END(stallout);
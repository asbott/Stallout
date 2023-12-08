#include "pch.h"

#include "Stallout/graphics/imgui_renderer.h"
#include "os/env.h"
#include "os/oswindow.h"



#include <imgui.h>



#include <mz_matrix.hpp>
#include <mz_algorithms.hpp>

NS_BEGIN(stallout);
NS_BEGIN(graphics);

constexpr os::Window_Event_Type WINDOW_EVENT_TYPE_MOUSELEAVE = os::WINDOW_EVENT_TYPE_MAX + 1;

struct Vert_Data {
    mz::fmat4 projection;
};
const char* vert_src =R"(
        layout (location = 0) in vec2 Position;
        layout (location = 1) in vec2 UV;
        layout (location = 2) in vec4 Color;
        layout(std140) uniform Vert_Data
		{
			uniform mat4 ProjMtx;
		};
        out vec2 Frag_UV;
        out vec4 Frag_Color;
        void main()
        {
            Frag_UV = UV;
            Frag_Color = Color;
            gl_Position = transpose(ProjMtx) * vec4(Position.xy,0,1);
        };
)";

struct Frag_Data {
    int texture_index;
};

const char* frag_src = R"(
        in vec2 Frag_UV;
        in vec4 Frag_Color;
        
        layout(std140) uniform Frag_Data
		{
			int texture_index;
		};

        layout (location = 0) out vec4 Out_Color;
        void main()
        {
            Out_Color = Frag_Color * texture(textures[texture_index], Frag_UV.st);
            //Out_Color = vec4(0.0, texture_index, 0.0, 1.0);
        };

)";




ImGuiMouseSource to_imgui_enum(os::Mouse_Source e) {
    switch (e) {
        case os::MOUSE_SOURCE_PEN: return ImGuiMouseSource_Pen;
        case os::MOUSE_SOURCE_TOUCHSCREEN: return ImGuiMouseSource_TouchScreen;
        case os::MOUSE_SOURCE_MOUSE: return ImGuiMouseSource_Mouse;
        default: INTENTIONAL_CRASH("Unhandled enum"); return ImGuiMouseSource_Mouse;
    }
}

static ImGuiKey to_imgui_enum(os::Input_Code code)
{
    switch (code)
    {
        case os::INPUT_CODE_TAB: return ImGuiKey_Tab;
        case os::INPUT_CODE_LEFT_ARROW: return ImGuiKey_LeftArrow;
        case os::INPUT_CODE_RIGHT_ARROW: return ImGuiKey_RightArrow;
        case os::INPUT_CODE_UP_ARROW: return ImGuiKey_UpArrow;
        case os::INPUT_CODE_DOWN_ARROW: return ImGuiKey_DownArrow;
        case os::INPUT_CODE_PAGEUP: return ImGuiKey_PageUp;
        case os::INPUT_CODE_PAGEDOWN: return ImGuiKey_PageDown;
        case os::INPUT_CODE_HOME: return ImGuiKey_Home;
        case os::INPUT_CODE_END: return ImGuiKey_End;
        case os::INPUT_CODE_INSERT: return ImGuiKey_Insert;
        case os::INPUT_CODE_DELETE: return ImGuiKey_Delete;
        case os::INPUT_CODE_BACKSPACE: return ImGuiKey_Backspace;
        case os::INPUT_CODE_SPACE: return ImGuiKey_Space;
        case os::INPUT_CODE_ENTER: return ImGuiKey_Enter;
        case os::INPUT_CODE_ESCAPE: return ImGuiKey_Escape;
        case os::INPUT_CODE_OEM_7: return ImGuiKey_Apostrophe;
        case os::INPUT_CODE_COMMA: return ImGuiKey_Comma;
        case os::INPUT_CODE_MINUS: return ImGuiKey_Minus;
        case os::INPUT_CODE_PERIOD: return ImGuiKey_Period;
        case os::INPUT_CODE_OEM_2: return ImGuiKey_Slash;
        case os::INPUT_CODE_OEM_1: return ImGuiKey_Semicolon;
        case os::INPUT_CODE_PLUS: return ImGuiKey_Equal;
        case os::INPUT_CODE_OEM_4: return ImGuiKey_LeftBracket;
        case os::INPUT_CODE_OEM_5: return ImGuiKey_Backslash;
        case os::INPUT_CODE_OEM_6: return ImGuiKey_RightBracket;
        case os::INPUT_CODE_OEM_3: return ImGuiKey_GraveAccent;
        case os::INPUT_CODE_CAPSLOCK: return ImGuiKey_CapsLock;
        case os::INPUT_CODE_SCROLL: return ImGuiKey_ScrollLock;
        case os::INPUT_CODE_NUMLOCK: return ImGuiKey_NumLock;
        case os::INPUT_CODE_SNAPSHOT: return ImGuiKey_PrintScreen;
        case os::INPUT_CODE_PAUSE: return ImGuiKey_Pause;
        case os::INPUT_CODE_NUMPAD0: return ImGuiKey_Keypad0;
        case os::INPUT_CODE_NUMPAD1: return ImGuiKey_Keypad1;
        case os::INPUT_CODE_NUMPAD2: return ImGuiKey_Keypad2;
        case os::INPUT_CODE_NUMPAD3: return ImGuiKey_Keypad3;
        case os::INPUT_CODE_NUMPAD4: return ImGuiKey_Keypad4;
        case os::INPUT_CODE_NUMPAD5: return ImGuiKey_Keypad5;
        case os::INPUT_CODE_NUMPAD6: return ImGuiKey_Keypad6;
        case os::INPUT_CODE_NUMPAD7: return ImGuiKey_Keypad7;
        case os::INPUT_CODE_NUMPAD8: return ImGuiKey_Keypad8;
        case os::INPUT_CODE_NUMPAD9: return ImGuiKey_Keypad9;
        case os::INPUT_CODE_DECIMAL: return ImGuiKey_KeypadDecimal;
        case os::INPUT_CODE_DIVIDE: return ImGuiKey_KeypadDivide;
        case os::INPUT_CODE_MULTIPLY: return ImGuiKey_KeypadMultiply;
        case os::INPUT_CODE_SUBTRACT: return ImGuiKey_KeypadSubtract;
        case os::INPUT_CODE_ADD: return ImGuiKey_KeypadAdd;
        case os::INPUT_CODE_NUMPAD_ENTER: return ImGuiKey_KeypadEnter;
        case os::INPUT_CODE_LSHIFT: return ImGuiKey_LeftShift;
        case os::INPUT_CODE_LCTRL: return ImGuiKey_LeftCtrl;
        case os::INPUT_CODE_LALT: return ImGuiKey_LeftAlt;
        case os::INPUT_CODE_LSUPER: return ImGuiKey_LeftSuper;
        case os::INPUT_CODE_RSHIFT: return ImGuiKey_RightShift;
        case os::INPUT_CODE_RCTRL: return ImGuiKey_RightCtrl;
        case os::INPUT_CODE_RALT: return ImGuiKey_RightAlt;
        case os::INPUT_CODE_RSUPER: return ImGuiKey_RightSuper;
        //case os::INPUT_CODE_APPS: return ImGuiKey_Menu;
        case os::INPUT_CODE_0: return ImGuiKey_0;
        case os::INPUT_CODE_1: return ImGuiKey_1;
        case os::INPUT_CODE_2: return ImGuiKey_2;
        case os::INPUT_CODE_3: return ImGuiKey_3;
        case os::INPUT_CODE_4: return ImGuiKey_4;
        case os::INPUT_CODE_5: return ImGuiKey_5;
        case os::INPUT_CODE_6: return ImGuiKey_6;
        case os::INPUT_CODE_7: return ImGuiKey_7;
        case os::INPUT_CODE_8: return ImGuiKey_8;
        case os::INPUT_CODE_9: return ImGuiKey_9;
        case os::INPUT_CODE_A: return ImGuiKey_A;
        case os::INPUT_CODE_B: return ImGuiKey_B;
        case os::INPUT_CODE_C: return ImGuiKey_C;
        case os::INPUT_CODE_D: return ImGuiKey_D;
        case os::INPUT_CODE_E: return ImGuiKey_E;
        case os::INPUT_CODE_F: return ImGuiKey_F;
        case os::INPUT_CODE_G: return ImGuiKey_G;
        case os::INPUT_CODE_H: return ImGuiKey_H;
        case os::INPUT_CODE_I: return ImGuiKey_I;
        case os::INPUT_CODE_J: return ImGuiKey_J;
        case os::INPUT_CODE_K: return ImGuiKey_K;
        case os::INPUT_CODE_L: return ImGuiKey_L;
        case os::INPUT_CODE_M: return ImGuiKey_M;
        case os::INPUT_CODE_N: return ImGuiKey_N;
        case os::INPUT_CODE_O: return ImGuiKey_O;
        case os::INPUT_CODE_P: return ImGuiKey_P;
        case os::INPUT_CODE_Q: return ImGuiKey_Q;
        case os::INPUT_CODE_R: return ImGuiKey_R;
        case os::INPUT_CODE_S: return ImGuiKey_S;
        case os::INPUT_CODE_T: return ImGuiKey_T;
        case os::INPUT_CODE_U: return ImGuiKey_U;
        case os::INPUT_CODE_V: return ImGuiKey_V;
        case os::INPUT_CODE_W: return ImGuiKey_W;
        case os::INPUT_CODE_X: return ImGuiKey_X;
        case os::INPUT_CODE_Y: return ImGuiKey_Y;
        case os::INPUT_CODE_Z: return ImGuiKey_Z;
        case os::INPUT_CODE_F1: return ImGuiKey_F1;
        case os::INPUT_CODE_F2: return ImGuiKey_F2;
        case os::INPUT_CODE_F3: return ImGuiKey_F3;
        case os::INPUT_CODE_F4: return ImGuiKey_F4;
        case os::INPUT_CODE_F5: return ImGuiKey_F5;
        case os::INPUT_CODE_F6: return ImGuiKey_F6;
        case os::INPUT_CODE_F7: return ImGuiKey_F7;
        case os::INPUT_CODE_F8: return ImGuiKey_F8;
        case os::INPUT_CODE_F9: return ImGuiKey_F9;
        case os::INPUT_CODE_F10: return ImGuiKey_F10;
        case os::INPUT_CODE_F11: return ImGuiKey_F11;
        case os::INPUT_CODE_F12: return ImGuiKey_F12;
        default: return ImGuiKey_None;
    }
}

struct Viewport_Data {
    os::Window_Style window_style = os::WINDOW_STYLE_UNSET;
    os::Window_Style_Ex window_ex_style = os::WINDOW_STYLE_EX_UNSET;

    os::Window* parent_window;
};

void imgui_viewport_flags_to_windown_style(ImGuiViewportFlags flags, os::Window_Style* out_style, os::Window_Style_Ex* out_ex_style) {
    if (flags & ImGuiViewportFlags_NoDecoration)
        *out_style = os::WINDOW_STYLE_POPUP;
    else
        *out_style = os::WINDOW_STYLE_OVERLAPPEDWINDOW;

    if (flags & ImGuiViewportFlags_NoTaskBarIcon)
        *out_ex_style = os::WINDOW_STYLE_EX_TOOLWINDOW;
    else
        *out_ex_style = os::WINDOW_STYLE_EX_APPWINDOW;

    if (flags & ImGuiViewportFlags_TopMost)
        *out_ex_style |= os::WINDOW_STYLE_EX_TOPMOST;
}

os::Window* imgui_get_window_from_viewport_id(ImGuiID viewport_id) {
    if (viewport_id)  {
        if (ImGuiViewport* viewport = ImGui::FindViewportByID(viewport_id))
            return (os::Window*)viewport->PlatformHandle;
    }
    return NULL;
}

static bool imgui_update_mouse_cursor()
{
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange)
        return false;

    ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
    if (imgui_cursor == ImGuiMouseCursor_None || io.MouseDrawCursor)
    {
        os::env::set_mouse_cursor(os::env::MOUSE_CURSOR_NONE);
    }
    else
    {
        os::env::Mouse_Cursor cursor = os::env::MOUSE_CURSOR_ARROW;
        
        switch (imgui_cursor)
        {
            case ImGuiMouseCursor_Arrow:        cursor = os::env::MOUSE_CURSOR_ARROW; break;
            case ImGuiMouseCursor_TextInput:    cursor = os::env::MOUSE_CURSOR_TEXTINPUT; break;
            case ImGuiMouseCursor_ResizeAll:    cursor = os::env::MOUSE_CURSOR_RESIZEALL; break;
            case ImGuiMouseCursor_ResizeEW:     cursor = os::env::MOUSE_CURSOR_RESIZEEW; break;
            case ImGuiMouseCursor_ResizeNS:     cursor = os::env::MOUSE_CURSOR_RESIZENS; break;
            case ImGuiMouseCursor_ResizeNESW:   cursor = os::env::MOUSE_CURSOR_RESIZENESW; break;
            case ImGuiMouseCursor_ResizeNWSE:   cursor = os::env::MOUSE_CURSOR_RESIZENWSE; break;
            case ImGuiMouseCursor_Hand:         cursor = os::env::MOUSE_CURSOR_HAND; break;
            case ImGuiMouseCursor_NotAllowed:   cursor = os::env::MOUSE_CURSOR_NOTALLOWED; break;
        }
        os::env::set_mouse_cursor(cursor);
    }
    return true;
}


bool imgui_handle_window_event(os::Window* window, os::Window_Event_Type etype, void* param, void* ud) {
    ImGui_Renderer* renderer = (ImGui_Renderer*)ud;
    if (ImGui::GetCurrentContext() == nullptr)
        return true;
    ST_DEBUG_ASSERT(window->get_child_count() == 0); // Debugging
    ImGuiIO& io = ImGui::GetIO();
    switch (etype)
    {
    case os::WINDOW_EVENT_TYPE_MOUSEMOVE:
    {
        const auto& e = *(os::Mouse_Move_Event*)param;
        
        ImGuiMouseSource mouse_source = to_imgui_enum(e.mouse_source);
        const int area = e.in_window ? 1 : 2;
        renderer->_mouse_window = window;
        if (renderer->_mouse_tracked_area != area)
        {

            Mouse_Leave_Event mle;
            mle.in_client = area == 1;

            window->dispatch_event(WINDOW_EVENT_TYPE_MOUSELEAVE, &mle);

            renderer->_mouse_tracked_area = area;
        }
        bool want_absolute_pos = (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) != 0;
        mz::ivec2 mouse_pos = { e.window_mouse_x, e.window_mouse_y };
        if (want_absolute_pos) {
            mouse_pos = { e.screen_mouse_x, e.screen_mouse_y };
        }
        
        io.AddMouseSourceEvent(mouse_source);
        io.AddMousePosEvent((float)mouse_pos.x, (float)mouse_pos.y);
        return !ImGui::GetIO().WantCaptureMouse;
    }
    case WINDOW_EVENT_TYPE_MOUSELEAVE:
    {
        const auto& e = *(Mouse_Leave_Event*)param;
        const int area = e.in_client ? 1 : 2;
        if (renderer->_mouse_tracked_area == area)
        {
            if (renderer->_mouse_window == window)
                renderer->_mouse_window = nullptr;
            renderer->_mouse_tracked_area = 0;
            io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
        }
        return !ImGui::GetIO().WantCaptureMouse;
    }
    case os::WINDOW_EVENT_TYPE_MOUSE_BUTTON:
    {
        ST_DEBUG_ASSERT(window->get_child_count() == 0); // Debugging
        const auto& e = *(os::Mouse_Button_Event*)param;
        ImGuiMouseSource mouse_source = to_imgui_enum(e.mouse_source);
        int button = 0;
        if (e.button == os::INPUT_CODE_MOUSE_LEFT) { button = 0; }
        if (e.button == os::INPUT_CODE_MOUSE_RIGHT) { button = 1; }
        if (e.button == os::INPUT_CODE_MOUSE_MIDDLE) { button = 2; }
        if (e.button == os::INPUT_CODE_MOUSE_SIDE1) { button = 3; }
        if (e.button == os::INPUT_CODE_MOUSE_SIDE2) { button = 4; }
        
        if (e.double_click) {
            return !ImGui::GetIO().WantCaptureMouse;
        } else if (e.state == os::INPUT_STATE_DOWN) {
            ST_DEBUG_ASSERT(window->get_child_count() == 0); // Debugging
            if (renderer->mouse_buttons_down == 0 && !window->has_captured_mouse()) {
                //window->capture_mouse();
            }
            ST_DEBUG_ASSERT(window->get_child_count() == 0); // Debugging
            renderer->mouse_buttons_down |= 1 << button;
            io.AddMouseSourceEvent(mouse_source);
            io.AddMouseButtonEvent(button, true);
            ST_DEBUG_ASSERT(window->get_child_count() == 0); // Debugging
            return !ImGui::GetIO().WantCaptureMouse;
        } else if (e.state == os::INPUT_STATE_UP) {
            renderer->mouse_buttons_down &= ~(1 << button);
            if (renderer->mouse_buttons_down == 0 && window->has_captured_mouse()) {
                window->release_mouse();
            }
            io.AddMouseSourceEvent(mouse_source);
            io.AddMouseButtonEvent(button, false);
            ST_DEBUG_ASSERT(window->get_child_count() == 0); // Debugging
            return !ImGui::GetIO().WantCaptureMouse;
        } else {
            INTENTIONAL_CRASH("Invalid mouse button event");
            return !ImGui::GetIO().WantCaptureMouse;
        }
        
    }
    case os::WINDOW_EVENT_TYPE_MOUSE_SCROLL:
    {
        const auto& e = *(os::Mouse_Scroll_Event*)param;
        if (e.axis == os::SCROLL_AXIS_HORIZONTAL) {
            io.AddMouseWheelEvent(-e.delta, 0.0f);
        } else if (e.axis == os::SCROLL_AXIS_VERTICAL) {
            io.AddMouseWheelEvent(0.0f, e.delta);
        } else {
            INTENTIONAL_CRASH("what");
        }
        
        return !ImGui::GetIO().WantCaptureMouse;
    }
    case os::WINDOW_EVENT_TYPE_KEY:
    {
        const auto& e = *(os::Key_Event*)param;
        const bool is_key_down = e.state == os::INPUT_STATE_DOWN;
        if (e.key < 256)
        {
            io.AddKeyEvent(ImGuiMod_Ctrl, os::is_input_down(os::INPUT_CODE_CTRL));
            io.AddKeyEvent(ImGuiMod_Shift, os::is_input_down(os::INPUT_CODE_LSHIFT) || window->input.get_state(os::INPUT_CODE_RSHIFT) == os::INPUT_STATE_DOWN);
            //io.AddKeyEvent(ImGuiMod_Alt, window->is_key_down(VK_MENU));
            //io.AddKeyEvent(ImGuiMod_Super, window->is_key_down(VK_APPS));

            const ImGuiKey key = to_imgui_enum(e.key);
            if (key != ImGuiKey_None)
                io.AddKeyEvent(key, is_key_down);

            if (e.key == os::INPUT_CODE_LSHIFT || e.key == os::INPUT_CODE_RSHIFT)
            {
                if (os::is_input_down(os::INPUT_CODE_LSHIFT) == is_key_down) { io.AddKeyEvent(ImGuiKey_LeftShift, is_key_down); }
                if (os::is_input_down(os::INPUT_CODE_RSHIFT) == is_key_down) { io.AddKeyEvent(ImGuiKey_RightShift, is_key_down); }
            }
            else if (e.key == os::INPUT_CODE_LCTRL || e.key == os::INPUT_CODE_RCTRL)
            {
                if (os::is_input_down(os::INPUT_CODE_LCTRL) == is_key_down) { io.AddKeyEvent(ImGuiKey_LeftCtrl, is_key_down); }
                if (os::is_input_down(os::INPUT_CODE_RCTRL) == is_key_down) { io.AddKeyEvent(ImGuiKey_RightCtrl, is_key_down); }
            }
        }
        return !ImGui::GetIO().WantCaptureKeyboard;
    }
    case os::WINDOW_EVENT_TYPE_SETFOCUS:
    case os::WINDOW_EVENT_TYPE_KILLFOCUS:
        io.AddFocusEvent(etype == os::WINDOW_EVENT_TYPE_SETFOCUS);
        return 0;
    case os::WINDOW_EVENT_TYPE_CHAR:
    {
        const auto& e = *(os::Char_Event*)param;

        {
            io.AddInputCharacter(e.value);
        }
        return !ImGui::GetIO().WantCaptureKeyboard;
    }
    case os::WINDOW_EVENT_TYPE_SETCURSOR:
        {
            const auto& e = *(os::Setcursor_Event*)param;
            if (e.in_client && imgui_update_mouse_cursor())
                return 1;
            return 0;
        }
    case os::WINDOW_EVENT_TYPE_DISPLAYCHANGE:
        renderer->_want_update_monitors = true;
        return 0;
    }
    return 0;
}
bool ImGui_Renderer::handle_window_event(os::Window* window, os::Window_Event_Type etype, void* param, void* ud) {
    if (imgui_handle_window_event(window, etype, param, ud))
        return true;

    if (ImGuiViewport* viewport = ImGui::FindViewportByPlatformHandle((void*)window))
    {
        switch (etype)
        {
        case os::WINDOW_EVENT_TYPE_CLOSE:
            viewport->PlatformRequestClose = true;
            return true;
        case os::WINDOW_EVENT_TYPE_MOVE:
            viewport->PlatformRequestMove = true;
            break;
        case os::WINDOW_EVENT_TYPE_SIZE:
            viewport->PlatformRequestResize = true;
            break;
        /*case os::WINDOW_EVENT_TYPE_MOUSEACTIVATE:
            if (viewport->Flags & ImGuiViewportFlags_NoFocusOnClick)
                return MA_NOACTIVATE;
            break;
        case os::WINDOW_EVENT_TYPE_NCHITTEST:
            // Let mouse pass-through the window. This will allow the backend to call io.AddMouseViewportEvent() correctly. (which is optional).
            // The ImGuiViewportFlags_NoInputs flag is set while dragging a viewport, as want to detect the window behind the one we are dragging.
            // If you cannot easily access those viewport flags from your windowing/event code: you may manually synchronize its state e.g. in
            // your main loop after calling UpdatePlatformWindows(). Iterate all viewports/platform windows and pass the flag to your windowing system.
            if (viewport->Flags & ImGuiViewportFlags_NoInputs)
                return HTTRANSPARENT;
            break;*/
        }
    }
    return false;
}

void imgui_callback_create_window(ImGuiViewport* viewport) {
    auto* vd = ST_NEW(Viewport_Data);
    viewport->PlatformUserData = vd; 
    imgui_viewport_flags_to_windown_style(viewport->Flags, &vd->window_style, &vd->window_ex_style);
    vd->parent_window = imgui_get_window_from_viewport_id(viewport->ParentViewportId);

    os::Window_Init_Spec ws;
    ws.title = "Untitled";
    ws.style = vd->window_style;
    ws.style_ex = vd->window_ex_style;
    ws.x = (s32)viewport->Pos.x;
    ws.y = (s32)viewport->Pos.y;
    ws.width = (s32)viewport->Size.x;
    ws.height = (s32)viewport->Size.y;

    auto renderer = (ImGui_Renderer*)ImGui::GetIO().BackendPlatformUserData;
    os::Window* new_window = vd->parent_window->add_child(ws);
    new_window->add_event_callback([&](os::Window* w, os::Window_Event_Type etype, void* param, void* ud) -> bool {
        return renderer->handle_window_event(w, etype, param, ud);
    }, renderer);

    viewport->PlatformRequestResize = false;
    viewport->PlatformHandle = new_window;
}

static void imgui_callback_destroy_window(ImGuiViewport* viewport)
{
    auto* bd = (ImGui_Renderer*)ImGui::GetIO().BackendPlatformUserData;
    if (Viewport_Data* vd = (Viewport_Data*)viewport->PlatformUserData)
    {
        auto* viewport_window = (os::Window*)viewport->PlatformHandle;
        if (viewport_window->has_captured_mouse())
        {
            viewport_window->capture_mouse();
        }
        if (viewport_window != bd->_main_window) // Not main window?
            ST_DELETE(viewport_window);
        ST_DELETE(vd);
    }
    viewport->PlatformUserData = viewport->PlatformHandle = nullptr;
}

static void imgui_callback_show_window(ImGuiViewport* viewport)
{
    ST_ASSERT(viewport->PlatformHandle);
    auto wnd = (os::Window*)viewport->PlatformHandle;
    wnd->set_visibility(true);
}

static void imgui_callback_update_window(ImGuiViewport* viewport) {
    Viewport_Data* vd = (Viewport_Data*)viewport->PlatformUserData;
    ST_ASSERT(viewport->PlatformHandle);

    auto wnd = (os::Window*)viewport->PlatformHandle;
    auto new_parent = imgui_get_window_from_viewport_id(viewport->ParentViewportId);
    if (new_parent != wnd)
    {
        vd->parent_window = new_parent;
        wnd->set_parent(new_parent);
    }

    
}

static ImVec2 imgui_callback_get_window_pos(ImGuiViewport* viewport) {
    auto wnd = (os::Window*)viewport->PlatformHandle;
    ST_ASSERT(wnd != 0);
    
    mz::s32vec2 wnd_pos {0, 0};
    wnd->get_position(&wnd_pos.x, &wnd_pos.y);

    return ImVec2((float)wnd_pos.x, (float)wnd_pos.y);
}

static void imgui_callback_set_window_pos(ImGuiViewport* viewport, ImVec2 pos) {
    auto wnd = (os::Window*)viewport->PlatformHandle;
    ST_ASSERT(wnd != 0);

    wnd->set_position((s32)pos.x, (s32)pos.y);
}

static ImVec2 imgui_callback_get_window_size(ImGuiViewport* viewport) {
    auto wnd = (os::Window*)viewport->PlatformHandle;
    ST_ASSERT(wnd != 0);

    mz::s32vec2 wnd_sz(0);
    wnd->get_size(&wnd_sz.x, &wnd_sz.y);

    return ImVec2((float)wnd_sz.x, (float)wnd_sz.y);
}

static void imgui_callback_set_window_size(ImGuiViewport* viewport, ImVec2 sz) {
    auto wnd = (os::Window*)viewport->PlatformHandle;
    ST_ASSERT(wnd != 0);

    wnd->set_size((s32)sz.x, (s32)sz.y);
}

static void imgui_callback_set_window_focus(ImGuiViewport* viewport) {
    auto wnd = (os::Window*)viewport->PlatformHandle;
    ST_ASSERT(wnd != 0);

    wnd->set_focus(true);
}

static bool imgui_callback_get_window_focus(ImGuiViewport* viewport) {
    auto wnd = (os::Window*)viewport->PlatformHandle;
    ST_ASSERT(wnd != 0);

   return wnd->is_focused();
}

static bool imgui_callback_get_window_minimized(ImGuiViewport* viewport) {
    auto wnd = (os::Window*)viewport->PlatformHandle;
    ST_ASSERT(wnd != 0);

   return wnd->is_minimized();
}

static void imgui_callback_set_window_title(ImGuiViewport* viewport, const char* title) {
    auto wnd = (os::Window*)viewport->PlatformHandle;
    ST_ASSERT(wnd != 0);
    
   wnd->set_title(title);
}

static void imgui_callback_set_window_alpha(ImGuiViewport* viewport, float alpha) {
    auto wnd = (os::Window*)viewport->PlatformHandle;
    ST_ASSERT(wnd != 0);
    ST_ASSERT(alpha >= 0.0f && alpha <= 1.0f);

    wnd->set_alpha(alpha);
}

static float imgui_callback_get_window_dpi_scale(ImGuiViewport* viewport) {
    auto wnd = (os::Window*)viewport->PlatformHandle;
    ST_ASSERT(wnd != 0);

    return os::env::get_monitor_dpi(wnd->get_monitor());
}


void ImGui_Renderer::_update_monitors() {
    os::env::Monitor* monitors;
    size_t num_monitors;
    os::env::get_monitors(monitors, &num_monitors);

    for (size_t i = 0; i < num_monitors; i++) {
        auto& monitor = monitors[i];
        ImGuiPlatformMonitor imgui_monitor;
        imgui_monitor.MainPos = ImVec2((float)monitor.main_left, (float)monitor.main_top);
        imgui_monitor.MainSize = ImVec2((float)(monitor.main_right - monitor.main_left), (float)(monitor.main_bottom - monitor.main_top));
        imgui_monitor.WorkPos = ImVec2((float)monitor.work_left, (float)monitor.work_top);
        imgui_monitor.WorkSize = ImVec2((float)(monitor.work_right - monitor.work_left), (float)(monitor.work_bottom - monitor.work_top));
        imgui_monitor.DpiScale = os::env::get_monitor_dpi(&monitor);
        imgui_monitor.PlatformHandle = (void*)&monitor;
        ImGuiPlatformIO& io = ImGui::GetPlatformIO();
        if (monitor.is_primary)/*info.dwFlags & MONITORINFOF_PRIMARY*/
            io.Monitors.push_front(imgui_monitor);
        else
            io.Monitors.push_back(imgui_monitor);
    }

    _want_update_monitors = false;
}

void ImGui_Renderer::_update_mouse_data() {
    ImGuiIO& io = ImGui::GetIO();

    auto wnd = _main_window;

    const bool is_app_focused = os::env::is_app_focused();//(focused_window && (focused_window == bd->hWnd || ::IsChild(focused_window, bd->hWnd) || ImGui::FindViewportByPlatformHandle((void*)focused_window)));
    if (is_app_focused)
    {
        std::function<os::Window*(os::Window*)> find_focused;
        find_focused = [&find_focused](os::Window* next) -> os::Window* {
            if (next->is_focused()) return next;

            for (auto child : next->_children) {
                if (auto focused = find_focused(child)) {
                    return focused;
                }
            }
            next->is_focused();
            return NULL;
        };
        os::Window* focused_window = find_focused(wnd);

        if (io.WantSetMousePos)
        {
            mz::s32vec2 pos = { io.MousePos.x, io.MousePos.y };
            if ((io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) == 0)
                focused_window->client_to_screen(&pos.x, &pos.y);
            os::env::set_cursor_pos(pos.x, pos.y);
        }

    }

    ImGuiID mouse_viewport_id = 0;
    std::function<os::Window*(os::Window*)> find_hovered;
    find_hovered = [&find_hovered](os::Window* next) -> os::Window* {
        if (next->is_hovered()) return next;

        for (auto child : next->_children) {
            if (auto focused = find_hovered(child)) {
                return focused;
            }
        }
        return NULL;
    };
    if (auto hovered_hwnd = find_hovered(wnd))
        if (ImGuiViewport* viewport = ImGui::FindViewportByPlatformHandle(hovered_hwnd))
            mouse_viewport_id = viewport->ID;
    io.AddMouseViewportEvent(mouse_viewport_id);
}

void imgui_callback_render_window(ImGuiViewport* viewport, void* rend) {
    auto wnd = (os::Window*)(viewport->PlatformHandle);
    auto context = (Graphics_Driver*)rend;
    context->set_target(wnd->get_device_context());
}

void imgui_callback_swap_buffers(ImGuiViewport* viewport, void* rend){
    auto wnd = (os::Window*)(viewport->PlatformHandle);
    auto context = (Graphics_Driver*)rend;
    context->set_target(wnd->get_device_context());
    wnd->swap_buffers();
}

bool ImGui_Renderer::_init_for_window_system() {

    ImGuiIO& io = ImGui::GetIO();
    io.BackendPlatformUserData = this;
    
    io.BackendPlatformName = "Stallout";
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;         // We can honor GetMouseCursor() values (optional)
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;          // We can honor io.WantSetMousePos requests (optional, rarely used)
    io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;    // We can create multi-viewports on the Platform side (optional)
    io.BackendFlags |= ImGuiBackendFlags_HasMouseHoveredViewport; // We can call io.AddMouseViewportEvent() with correct data (optional)

    _want_update_monitors = true;

    ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    main_viewport->PlatformHandle = main_viewport->PlatformHandleRaw = _main_window;
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        _update_monitors();

        ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
        platform_io.Platform_CreateWindow = imgui_callback_create_window;
        platform_io.Platform_DestroyWindow = imgui_callback_destroy_window;
        platform_io.Platform_ShowWindow = imgui_callback_show_window;
        platform_io.Platform_SetWindowPos = imgui_callback_set_window_pos;
        platform_io.Platform_GetWindowPos = imgui_callback_get_window_pos;
        platform_io.Platform_SetWindowSize = imgui_callback_set_window_size;
        platform_io.Platform_GetWindowSize = imgui_callback_get_window_size;
        platform_io.Platform_SetWindowFocus = imgui_callback_set_window_focus;
        platform_io.Platform_GetWindowFocus = imgui_callback_get_window_focus;
        platform_io.Platform_GetWindowMinimized = imgui_callback_get_window_minimized;
        platform_io.Platform_SetWindowTitle = imgui_callback_set_window_title;
        platform_io.Platform_SetWindowAlpha = imgui_callback_set_window_alpha;
        platform_io.Platform_UpdateWindow = imgui_callback_update_window;
        platform_io.Platform_GetWindowDpiScale = imgui_callback_get_window_dpi_scale; // FIXME-DPI
        platform_io.Platform_OnChangedViewport = [](ImGuiViewport*){};
        platform_io.Platform_RenderWindow = imgui_callback_render_window;
        platform_io.Platform_SwapBuffers = imgui_callback_swap_buffers;

        auto* vd = ST_NEW(Viewport_Data);
        
        main_viewport->PlatformHandle = _main_window;
        main_viewport->PlatformUserData = vd; 
    }

    return true;
}
void ImGui_Renderer::_shutdown_for_window_system() {
     ImGui::DestroyPlatformWindows();

    ImGuiIO& io = ImGui::GetIO();
    io.BackendPlatformName = nullptr;
    io.BackendPlatformUserData = nullptr;
    io.BackendFlags &= ~(ImGuiBackendFlags_HasMouseCursors | ImGuiBackendFlags_HasSetMousePos | ImGuiBackendFlags_HasGamepad | ImGuiBackendFlags_PlatformHasViewports | ImGuiBackendFlags_HasMouseHoveredViewport);
}
void ImGui_Renderer::_new_frame_for_window_system(f32 delta_time) {
    ImGuiIO& io = ImGui::GetIO();
    mz::s32vec2 sz(0);
    _main_window->get_size(&sz.x, &sz.y);
    io.DisplaySize = ImVec2((f32)sz.x, (f32)sz.y);

    if (_want_update_monitors)
        _update_monitors();

    io.DeltaTime = delta_time;

    _update_mouse_data();

    if (ImGui::IsKeyDown(ImGuiKey_LeftShift) && !os::is_input_down(os::INPUT_CODE_LSHIFT)) {
        io.AddKeyEvent(ImGuiKey_LeftShift, false);
    }
    if (ImGui::IsKeyDown(ImGuiKey_RightShift) && !os::is_input_down(os::INPUT_CODE_RSHIFT)) {
        io.AddKeyEvent(ImGuiKey_RightShift, false);
    }

    if (ImGui::IsKeyDown(ImGuiKey_LeftSuper) && !os::is_input_down(os::INPUT_CODE_LSUPER)) {
        io.AddKeyEvent(ImGuiKey_LeftSuper, false);
    }
    if (ImGui::IsKeyDown(ImGuiKey_RightSuper) && !os::is_input_down(os::INPUT_CODE_RSUPER)) {
        io.AddKeyEvent(ImGuiKey_RightSuper, false);
    }

    ImGuiMouseCursor mouse_cursor = io.MouseDrawCursor ? ImGuiMouseCursor_None : ImGui::GetMouseCursor();
    if (_last_mouse_cursor != mouse_cursor)
    {
        imgui_update_mouse_cursor();
        _last_mouse_cursor = mouse_cursor;
    }
}




void render_window(ImGuiViewport* viewport, void*) {
    ImGui_Renderer* renderer = (ImGui_Renderer*)ImGui::GetIO().BackendPlatformUserData;

    if (!(viewport->Flags & ImGuiViewportFlags_NoRendererClear))
    {
        renderer->_context->set_clear_color(mz::COLOR_GREEN);
        renderer->_context->clear(CLEAR_FLAG_COLOR);
    }
    renderer->_render_draw_data(viewport->DrawData);
}
void ImGui_Renderer::_prepare_draw_call(void* hdraw_data, s32 fb_width, s32 fb_height) {

    ImDrawData* draw_data = (ImDrawData*)hdraw_data;

    //_context->client_acquire_target_context();
    _context->query(QUERY_TYPE_RENDERER_SETTINGS_FLAGS, &_state_backup.settings);
    _context->query(QUERY_TYPE_BLENDING_EQUATION,  &_state_backup.blend_eq);
    _context->query(QUERY_TYPE_BLENDING_SRC_COLOR, &_state_backup.src_col);
    _context->query(QUERY_TYPE_BLENDING_DST_COLOR, &_state_backup.dst_col);
    _context->query(QUERY_TYPE_BLENDING_SRC_ALPHA, &_state_backup.src_alpha);
    _context->query(QUERY_TYPE_BLENDING_DST_ALPHA, &_state_backup.dst_alpha);
    _context->query(QUERY_TYPE_POLY_MODE_FRONT, &_state_backup.poly_mode_front);
    _context->query(QUERY_TYPE_POLY_MODE_BACK, &_state_backup.poly_mode_back);
    //_context->client_release_target_context();
    _context->query((Query_Type)(QUERY_TYPE_TEXTURE_SLOT + 0), &_state_backup.last_bound_tex);

    _context->set_blending(
        BLEND_EQUATION_ADD,
        BLEND_FACTOR_SRC_ALPHA, BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, 
        BLEND_FACTOR_ONE, BLEND_FACTOR_ONE_MINUS_SRC_ALPHA
    );

    _context->disable(
        RENDERER_SETTING_CULLING & 
        RENDERER_SETTING_DEPTH_TESTING &
        RENDERER_SETTING_STENCIL_TESTING &
        RENDERER_SETTING_PRIMITIVE_RESTART
    );
    _context->enable(RENDERER_SETTING_SCISSOR_TESTING);
    _context->set_polygon_mode(POLY_FACE_FRONT_AND_BACK, POLY_MODE_FILL);

    _context->set_viewport({0, 0}, {(s32)fb_width, (s32)fb_height});
    float L = draw_data->DisplayPos.x;
    float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
    float T = draw_data->DisplayPos.y;
    float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;

    Vert_Data vd;
    vd.projection = mz::projection::ortho(L, R, B, T, -100000000.f, 100000000.f); 
    Frag_Data fd;
    fd.texture_index = 0;

    _context->set_uniform_block(_hshader, _hubo_vert, "Vert_Data", 0);
    _context->set_uniform_block(_hshader, _hubo_frag, "Frag_Data", 1);
    _context->set_texture_slot(0, _hfont_texture);

    _context->set_buffer(_hubo_vert, &vd);
    _context->set_buffer(_hubo_frag, &fd);
}
void ImGui_Renderer::_render_draw_data(void* hdraw_data) {
    ImDrawData* draw_data = (ImDrawData*)hdraw_data;
    int fb_width = (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
    int fb_height = (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
    if (fb_width <= 0 || fb_height <= 0)
        return;

    _prepare_draw_call(draw_data, fb_width, fb_height);

    // Will project scissor/clipping rectangles into framebuffer space
    ImVec2 clip_off = draw_data->DisplayPos;         // (0,0) unless using multi-viewports
    ImVec2 clip_scale = draw_data->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

    // Render command lists
    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];

        // Upload vertex/index buffers
        const size_t vtx_buffer_size = (size_t)cmd_list->VtxBuffer.Size * (int)sizeof(ImDrawVert);
        const size_t idx_buffer_size = (size_t)cmd_list->IdxBuffer.Size * (int)sizeof(ImDrawIdx);
        
        if (vbo_size < vtx_buffer_size)
        {
            vbo_size = vtx_buffer_size;
            _context->set_buffer(_hvbo, 0, vbo_size);
        }
        if (ibo_size < idx_buffer_size)
        {
            ibo_size = idx_buffer_size;
            _context->set_buffer(_hibo, 0, ibo_size);
        }

        _context->append_to_buffer(_hvbo, 0, cmd_list->VtxBuffer.Data, vtx_buffer_size);
        _context->append_to_buffer(_hibo, 0, cmd_list->IdxBuffer.Data, idx_buffer_size);

        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback != nullptr)
            {
                if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
                    _prepare_draw_call(draw_data, fb_width, fb_height);
                else
                    pcmd->UserCallback(cmd_list, pcmd);
            }
            else
            {
                ImVec2 clip_min((pcmd->ClipRect.x - clip_off.x) * clip_scale.x, (pcmd->ClipRect.y - clip_off.y) * clip_scale.y);
                ImVec2 clip_max((pcmd->ClipRect.z - clip_off.x) * clip_scale.x, (pcmd->ClipRect.w - clip_off.y) * clip_scale.y);
                if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
                    continue;

                _context->set_scissor_box({(int)clip_min.x, (int)((float)fb_height - clip_max.y), (int)(clip_max.x - clip_min.x), (int)(clip_max.y - clip_min.y)});

                _context->set_texture_slot(0, (Resource_Handle)pcmd->GetTexID());

                _context->draw_indexed(
                    _hlayout, _hvbo, _hibo, _hshader, 
                    pcmd->ElemCount, 
                    sizeof(ImDrawIdx) == 2 ? DATA_TYPE_USHORT : DATA_TYPE_UINT, 
                    pcmd->IdxOffset
                );
            }
        }
    }

    if (_state_backup.settings & RENDERER_SETTING_CULLING) 
        _context->enable(RENDERER_SETTING_CULLING);
    if (_state_backup.settings & RENDERER_SETTING_DEPTH_TESTING) 
        _context->enable(RENDERER_SETTING_DEPTH_TESTING);
    if (_state_backup.settings & RENDERER_SETTING_STENCIL_TESTING) 
        _context->enable(RENDERER_SETTING_STENCIL_TESTING);
    if (_state_backup.settings & RENDERER_SETTING_PRIMITIVE_RESTART) 
        _context->enable(RENDERER_SETTING_PRIMITIVE_RESTART);
    if (!(_state_backup.settings & RENDERER_SETTING_SCISSOR_TESTING)) 
        _context->disable(RENDERER_SETTING_SCISSOR_TESTING);
    
    _context->set_blending(
        _state_backup.blend_eq,
        _state_backup.src_col, _state_backup.dst_col,
        _state_backup.src_alpha, _state_backup.dst_alpha
    );
    _context->set_polygon_mode(POLY_FACE_FRONT, _state_backup.poly_mode_front);
    _context->set_polygon_mode(POLY_FACE_BACK, _state_backup.poly_mode_back);

    if (_state_backup.last_bound_tex) {
        _context->set_texture_slot(0, _state_backup.last_bound_tex);
    }
}

bool ImGui_Renderer::_init_for_renderer() {

    auto& io = ImGui::GetIO();
    io.BackendRendererName = "StalloutRenderer";
    io.BackendRendererUserData = (void*)this;

    auto env = _context->get_environment();
    
    io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
        platform_io.Renderer_RenderWindow = render_window;
    }

    return true;
}
void ImGui_Renderer::_shutdown_for_renderer() {
    ImGuiIO& io = ImGui::GetIO();

    ImGui::DestroyPlatformWindows();
    
    _context->destroy(_hvbo);
    _context->destroy(_hibo);
    _context->destroy(_hlayout);
    _context->destroy(_hshader);
    _context->destroy(_hubo_vert);
    _context->destroy(_hubo_frag);
    if (_hfont_texture)
    {
        _context->destroy(_hfont_texture);
        io.Fonts->SetTexID(0);
        _hfont_texture = 0;
    }

    io.BackendRendererName = nullptr;
    io.BackendRendererUserData = nullptr;
    io.BackendFlags &= ~(ImGuiBackendFlags_RendererHasVtxOffset | ImGuiBackendFlags_RendererHasViewports);
}
void ImGui_Renderer::_new_frame_for_renderer(f32 delta_time) {
    auto& io = ImGui::GetIO();
    io.DeltaTime = delta_time;
    if (!_hshader) {
        // TODO: #renderer #unfinished
        // No way to check resources for errors
        _hshader = _context->create_shader(vert_src, frag_src);

        _hubo_vert = _context->create_buffer(BUFFER_TYPE_UNIFORM_BUFFER, BUFFER_USAGE_STREAM_DRAW);
        _hubo_frag = _context->create_buffer(BUFFER_TYPE_UNIFORM_BUFFER, BUFFER_USAGE_STREAM_DRAW);
        _hlayout = _context->create_buffer_layout({
            { 2, DATA_TYPE_FLOAT },
            { 2, DATA_TYPE_FLOAT },
            { 4, DATA_TYPE_UBYTE, true /* Normalized (color) */ },
        });

        _hvbo = _context->create_buffer(BUFFER_TYPE_ARRAY_BUFFER, BUFFER_USAGE_STREAM_DRAW);
        _hibo = _context->create_buffer(BUFFER_TYPE_ELEMENT_ARRAY_BUFFER, BUFFER_USAGE_STREAM_DRAW);

        unsigned char* pixels;
        int width, height;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

        _hfont_texture = _context->create_texture(
            width, height, 4, 
            TEXTURE2D_FORMAT_RGBA, 
            DATA_TYPE_UBYTE,
            TEXTURE2D_FORMAT_RGBA,
            TEXTURE_FILTER_MODE_LINEAR,
            TEXTURE_FILTER_MODE_LINEAR,
            MIPMAP_MODE_NONE
        );

        io.Fonts->SetTexID((ImTextureID)_hfont_texture);

        _context->set_texture2d(_hfont_texture, pixels, width * height * 4);

    }
}

ImGui_Renderer::ImGui_Renderer(Graphics_Driver* renderer, os::Window* main_window) 
    : _context(renderer), _main_window(main_window) {

    _imgui_context = ImGui::CreateContext();
    ST_ASSERT(_imgui_context, "Failed creating imgui context");
    ImGui::SetCurrentContext((ImGuiContext*)_imgui_context);
    ImGui::StyleColorsDark();

    _context->wait_ready();

    auto& io = ImGui::GetIO();

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    bool result = _init_for_window_system();
    ST_ASSERT(result, "Failed to initialize ImGui for window system");

    result = _init_for_renderer();
    ST_ASSERT(result, "Failed to initialize ImGui for renderer");
    
}
ImGui_Renderer::~ImGui_Renderer() {
    _shutdown_for_window_system();
    _shutdown_for_renderer();
    ImGui::DestroyContext();
}

void ImGui_Renderer::new_frame(f32 delta_time) {

    _new_frame_for_window_system(delta_time);
    _new_frame_for_renderer(delta_time);
    ImGui::GetIO().DeltaTime = delta_time;
    ImGui::NewFrame();
}

void ImGui_Renderer::render() {
    ImGui::Render();
    auto draw_data = ImGui::GetDrawData();

    auto backup_target = _context->get_current_target();

    _render_draw_data(draw_data);

    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault(_context, _context);

    _context->set_target(backup_target);
}

NS_END(graphics);
NS_END(stallout);

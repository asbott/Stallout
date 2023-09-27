#include "pch.h"

#include "Engine/renderer/imgui_renderer.h"
#include "os/env.h"
#include "os/oswindow.h"



#include <imgui.h>

#ifndef ST_USE_CUSTOM_IMGUI_BACKEND

    // Check for operating system
    #ifdef _ST_OS_WINDOWS
        #include <backends/imgui_impl_win32.h>
        #define _WND_IMPL ImGui_ImplWin32_
    #else
        #include <backends/imgui_impl_glfw.h>
        #define _WND_IMPL ImGui_ImplGlfw_
    #endif

    // Check for rendering backend
    #ifdef _ST_RENDER_BACKEND_OPENGL45
        #include <backends/imgui_impl_opengl3.h>
        #define _REND_IMPL ImGui_ImplOpenGL3_
    #elif defined(_ST_RENDER_BACKEND_VULKAN)
        #include <backends/imgui_impl_vulkan.h>
        #define _REND_IMPL ImGui_ImplVulkan_
    #elif defined(_ST_RENDER_BACKEND_DX11)
        #include <backends/imgui_impl_dx11.h>
        #define _REND_IMPL ImGui_ImplDX11_
    #elif defined(_ST_RENDER_BACKEND_DX12)
        #include <backends/imgui_impl_dx12.h>
        #define _REND_IMPL ImGui_ImplDX12_
    #endif

    // Define initialization function using token pasting
    #define TOKEN_PASTE_HELPER(x, y) x##y
    #define TOKEN_PASTE(x, y) TOKEN_PASTE_HELPER(x, y)

    #define _Rend_Init TOKEN_PASTE(_REND_IMPL, Init)
    #define _Rend_Shutdown TOKEN_PASTE(_REND_IMPL, Shutdown)
    #define _Rend_NewFrame TOKEN_PASTE(_REND_IMPL, NewFrame)
    #define _Rend_RenderDrawData TOKEN_PASTE(_REND_IMPL, RenderDrawData)

#ifdef _ST_OS_WINDOWS
    #define _Wnd_Init TOKEN_PASTE(_WND_IMPL, InitForOpenGL)
#endif
    #define _Wnd_Shutdown TOKEN_PASTE(_WND_IMPL, Shutdown)
    #define _Wnd_NewFrame TOKEN_PASTE(_WND_IMPL, NewFrame)

#endif



#include <mz_matrix.hpp>
#include <mz_algorithms.hpp>

NS_BEGIN(engine);
NS_BEGIN(renderer);

#ifndef ST_USE_CUSTOM_IMGUI_BACKEND

ImGui_Renderer::ImGui_Renderer(Render_Context* renderer) {

    _context = renderer;

    _context->wait_ready();

    IMGUI_CHECKVERSION();
    _imgui_context = ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    _Wnd_Init(renderer->get_window()->_backend->_os_handle);
    _Rend_Init();
}
ImGui_Renderer::~ImGui_Renderer() {
    ImGui::SetCurrentContext((ImGuiContext*)_imgui_context);
    _Wnd_Shutdown();
    _Rend_Shutdown();
    ImGui::DestroyContext();
}
void ImGui_Renderer::new_frame(f32 delta_time) {
    ImGui::SetCurrentContext((ImGuiContext*)_imgui_context);

    auto& io = ImGui::GetIO();
    io.DeltaTime = delta_time;

    auto wnd_size = _context->get_window()->get_size();
    io.DisplaySize = ImVec2((float)wnd_size.x, (float)wnd_size.y);

    _Wnd_NewFrame();
    _Rend_NewFrame();
    ImGui::NewFrame();
}
void ImGui_Renderer::render() {
    ImGui::SetCurrentContext((ImGuiContext*)_imgui_context);
    ImGui::Render();
    _Rend_RenderDrawData(ImGui::GetDrawData());
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
}

#else 

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
        case os::INPUT_CODE_BACK: return ImGuiKey_Backspace;
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

    Render_Window* parent_window;
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

Render_Window* imgui_get_window_from_viewport_id(ImGuiID viewport_id) {
    if (viewport_id)  {
        if (ImGuiViewport* viewport = ImGui::FindViewportByID(viewport_id))
            return (Render_Window*)viewport->PlatformHandle;
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
        // Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
        os::env::set_mouse_cursor(os::env::MOUSE_CURSOR_NONE);
    }
    else
    {
        // Show OS mouse cursor
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

//////////////////////////////////////////////////////////
// Callbacks

bool imgui_handle_window_event(Render_Window* window, os::Window_Event_Type etype, void* param, void* ud) {
    ImGui_Renderer* renderer = (ImGui_Renderer*)ud;
    if (ImGui::GetCurrentContext() == nullptr)
        return 0;

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
            /*if (renderer->_mouse_tracked_area) {
                Mouse_Leave_Event mse_cancel;
                mse_cancel.cancel = true;
                window->dispatch_event(WINDOW_EVENT_TYPE_MOUSELEAVE, &mse_cancel);
            }*/
            /*TRACKMOUSEEVENT tme_cancel = { sizeof(tme_cancel), TME_CANCEL, hwnd, 0 };
            TRACKMOUSEEVENT tme_track = { sizeof(tme_track), (DWORD)((area == 2) ? (TME_LEAVE | TME_NONCLIENT) : TME_LEAVE), hwnd, 0 };
            if (renderer->_mouse_tracked_area != 0)
                ::TrackMouseEvent(&tme_cancel);
            ::TrackMouseEvent(&tme_track);*/

            Mouse_Leave_Event mle;
            mle.in_client = area == 1;

            window->dispatch_event(WINDOW_EVENT_TYPE_MOUSELEAVE, &mle);

            renderer->_mouse_tracked_area = area;
        }
        bool want_absolute_pos = (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) != 0;
        mz::ivec2 mouse_pos = { e.window_mouse_x, e.window_mouse_y };
        
        /*if (e.in_window && want_absolute_pos)    // os::WINDOW_EVENT_TYPE_MOUSEMOVE are client-relative coordinates.
            mouse_pos = window->client_to_screen(mouse_pos);
        if (!e.in_window && !want_absolute_pos) // os::WINDOW_EVENT_TYPE_NCMOUSEMOVE are absolute coordinates.
            mouse_pos = window->screen_to_client(mouse_pos);*/

        if (want_absolute_pos) {
            mouse_pos = { e.screen_mouse_x, e.screen_mouse_y };
        }

        
        io.AddMouseSourceEvent(mouse_source);
        io.AddMousePosEvent((float)mouse_pos.x, (float)mouse_pos.y);
        break;
    }
    case WINDOW_EVENT_TYPE_MOUSELEAVE:
    //case os::WINDOW_EVENT_TYPE_NCMOUSELEAVE:
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
        break;
    }
    case os::WINDOW_EVENT_TYPE_MOUSE_BUTTON:
    {
        const auto& e = *(os::Mouse_Button_Event*)param;
        ImGuiMouseSource mouse_source = to_imgui_enum(e.mouse_source);
        int button = 0;
        if (e.button == os::INPUT_CODE_MOUSE_LEFT) { button = 0; }
        if (e.button == os::INPUT_CODE_MOUSE_RIGHT) { button = 1; }
        if (e.button == os::INPUT_CODE_MOUSE_MIDDLE) { button = 2; }
        if (e.button == os::INPUT_CODE_MOUSE_SIDE1) { button = 3; }
        if (e.button == os::INPUT_CODE_MOUSE_SIDE2) { button = 4; }
        
        if (e.double_click) {
            return 0;
        } else if (e.state == os::INPUT_STATE_DOWN) {
            if (renderer->mouse_buttons_down == 0 && !window->has_captured_mouse()) {
                //window->capture_mouse();
            }
            renderer->mouse_buttons_down |= 1 << button;
            io.AddMouseSourceEvent(mouse_source);
            io.AddMouseButtonEvent(button, true);
            return 0;
        } else if (e.state == os::INPUT_STATE_UP) {
            renderer->mouse_buttons_down &= ~(1 << button);
            if (renderer->mouse_buttons_down == 0 && window->has_captured_mouse()) {
                //::ReleaseCapture();
                window->release_mouse();
            }
            io.AddMouseSourceEvent(mouse_source);
            io.AddMouseButtonEvent(button, false);
            return 0;
        } else {
            INTENTIONAL_CRASH("Invalid mouse button event");
            return 0;
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
        
        return 0;   
    }
    case os::WINDOW_EVENT_TYPE_KEY:
    {
        const auto& e = *(os::Key_Event*)param;
        const bool is_key_down = e.state == os::INPUT_STATE_DOWN;
        if (e.key < 256)
        {
            // Submit modifiers
            io.AddKeyEvent(ImGuiMod_Ctrl, os::is_input_down(os::INPUT_CODE_CTRL));
            io.AddKeyEvent(ImGuiMod_Shift, os::is_input_down(os::INPUT_CODE_LSHIFT) || window->is_input_down(os::INPUT_CODE_RSHIFT));
            //io.AddKeyEvent(ImGuiMod_Alt, window->is_key_down(VK_MENU));
            //io.AddKeyEvent(ImGuiMod_Super, window->is_key_down(VK_APPS));

            // Submit key event
            const ImGuiKey key = to_imgui_enum(e.key);
            if (key != ImGuiKey_None)
                io.AddKeyEvent(key, is_key_down);

            // Submit individual left/right modifier events
            if (e.key == os::INPUT_CODE_LSHIFT || e.key == os::INPUT_CODE_RSHIFT)
            {
                // Important: Shift keys tend to get stuck when pressed together, missing key-up events are corrected in ImGui_ImplWin32_ProcessKeyEventsWorkarounds()
                if (os::is_input_down(os::INPUT_CODE_LSHIFT) == is_key_down) { io.AddKeyEvent(ImGuiKey_LeftShift, is_key_down); }
                if (os::is_input_down(os::INPUT_CODE_RSHIFT) == is_key_down) { io.AddKeyEvent(ImGuiKey_RightShift, is_key_down); }
            }
            else if (e.key == os::INPUT_CODE_LCTRL || e.key == os::INPUT_CODE_RCTRL)
            {
                if (os::is_input_down(os::INPUT_CODE_LCTRL) == is_key_down) { io.AddKeyEvent(ImGuiKey_LeftCtrl, is_key_down); }
                if (os::is_input_down(os::INPUT_CODE_RCTRL) == is_key_down) { io.AddKeyEvent(ImGuiKey_RightCtrl, is_key_down); }
            }
            /*else if (vk == VK_MENU)
            {
                if (IsVkDown(VK_LMENU) == is_key_down) { ImGui_ImplWin32_AddKeyEvent(ImGuiKey_LeftAlt, is_key_down, VK_LMENU, scancode); }
                if (IsVkDown(VK_RMENU) == is_key_down) { ImGui_ImplWin32_AddKeyEvent(ImGuiKey_RightAlt, is_key_down, VK_RMENU, scancode); }
            }*/
        }
        return 0;
    }
    case os::WINDOW_EVENT_TYPE_SETFOCUS:
    case os::WINDOW_EVENT_TYPE_KILLFOCUS:
        io.AddFocusEvent(etype == os::WINDOW_EVENT_TYPE_SETFOCUS);
        return 0;
    case os::WINDOW_EVENT_TYPE_CHAR:
    {
        const auto& e = *(os::Char_Event*)param;

        // TODO: #support #unicode #utf16
        // Fix imgui support for unicode characters
        /*if (::IsWindowUnicode(hwnd))
        {
            // You can also use ToAscii()+GetKeyboardState() to retrieve characters.
            if (wParam > 0 && wParam < 0x10000)
                io.AddInputCharacterUTF16((unsigned short)wParam);
        }
        else*/
        {
            io.AddInputCharacter(e.value);
        }
        return 0;
    }
    case os::WINDOW_EVENT_TYPE_SETCURSOR:
        {
            const auto& e = *(os::Setcursor_Event*)param;
            // This is required to restore cursor when transitioning from e.g resize borders to client area.
            if (e.in_client && imgui_update_mouse_cursor())
                return 1;
            return 0;
        }
        // TODO: #gamepad #support
/*    case os::WINDOW_EVENT_TYPE_DEVICECHANGE:
#ifndef IMGUI_IMPL_WIN32_DISABLE_GAMEPAD
        if ((UINT)wParam == DBT_DEVNODES_CHANGED)
            renderer->WantUpdateHasGamepad = true;
#endif
        return 0;*/
    case os::WINDOW_EVENT_TYPE_DISPLAYCHANGE:
        renderer->_want_update_monitors = true;
        return 0;
    }
    return 0;
}
bool imgui_window_event_callback(Render_Window* window, os::Window_Event_Type etype, void* param, void* ud) {
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
    return true;
}

void imgui_callback_create_window(ImGuiViewport* viewport) {
    // Select style and parent window
    auto* vd = ST_NEW(Viewport_Data);
    viewport->PlatformUserData = vd; 
    imgui_viewport_flags_to_windown_style(viewport->Flags, &vd->window_style, &vd->window_ex_style);
    vd->parent_window = imgui_get_window_from_viewport_id(viewport->ParentViewportId);

    // Create window
    /*RECT rect = { (LONG)viewport->Pos.x, (LONG)viewport->Pos.y, (LONG)(viewport->Pos.x + viewport->Size.x), (LONG)(viewport->Pos.y + viewport->Size.y) };
    ::djustWindowRectEx(&rect, vd->window_style, FALSE, vd->window_ex_style);*/
    os::Window_Init_Spec ws;
    ws.title = "Untitled";
    ws.style = vd->window_style;
    ws.style_ex = vd->window_ex_style;
    ws.x = (s32)viewport->Pos.x;
    ws.y = (s32)viewport->Pos.y;
    ws.width = (s32)viewport->Size.x;
    ws.height = (s32)viewport->Size.y;

    Render_Window* new_window = vd->parent_window->add_child(ws);
    new_window->add_event_callback(imgui_window_event_callback, (ImGui_Renderer*)ImGui::GetIO().BackendPlatformUserData);

    // vd->HwndOwned = true; // Only main window imgui does not own the window
    viewport->PlatformRequestResize = false;
    viewport->PlatformHandle = new_window;
}

static void imgui_callback_destroy_window(ImGuiViewport* viewport)
{
    auto* bd = (ImGui_Renderer*)ImGui::GetIO().BackendPlatformUserData;
    if (Viewport_Data* vd = (Viewport_Data*)viewport->PlatformUserData)
    {
        auto* viewport_window = (Render_Window*)viewport->PlatformHandle;
        if (viewport_window->has_captured_mouse())
        {
            // Transfer capture so if we started dragging from a window that later disappears, we'll still receive the MOUSEUP event.
            /*::ReleaseCapture();
            ::SetCapture(bd->hWnd);*/
            viewport_window->capture_mouse();
        }
        if (viewport_window != bd->_context->get_window()) // Not main window?
            ST_DELETE(viewport_window);
        ST_DELETE(vd);
    }
    viewport->PlatformUserData = viewport->PlatformHandle = nullptr;
}

static void imgui_callback_show_window(ImGuiViewport* viewport)
{
    ST_ASSERT(viewport->PlatformHandle);
    auto wnd = (Render_Window*)viewport->PlatformHandle;
    /*if (viewport->Flags & ImGuiViewportFlags_NoFocusOnAppearing)
        ::ShowWindow(vd->Hwnd, SW_SHOWNA);
    else
        ::ShowWindow(vd->Hwnd, SW_SHOW);*/
    wnd->set_visibility(true);
}

static void imgui_callback_update_window(ImGuiViewport* viewport) {
    Viewport_Data* vd = (Viewport_Data*)viewport->PlatformUserData;
    ST_ASSERT(viewport->PlatformHandle);

    auto wnd = (Render_Window*)viewport->PlatformHandle;
    // Update Win32 parent if it changed _after_ creation
    // Unlike style settings derived from configuration flags, this is more likely to change for advanced apps that are manipulating ParentViewportID manually.
    auto new_parent = imgui_get_window_from_viewport_id(viewport->ParentViewportId);
    if (new_parent != wnd)
    {
        // Win32 windows can either have a "Parent" (for WS_CHILD window) or an "Owner" (which among other thing keeps window above its owner).
        // Our Dear Imgui-side concept of parenting only mostly care about what Win32 call "Owner".
        // The parent parameter of CreateWindowEx() sets up Parent OR Owner depending on WS_CHILD flag. In our case an Owner as we never use WS_CHILD.
        // Calling ::SetParent() here would be incorrect: it will create a full child relation, alter coordinate system and clipping.
        // Calling ::SetWindowLongPtr() with GWLP_HWNDPARENT seems correct although poorly documented.
        // https://devblogs.microsoft.com/oldnewthing/20100315-00/?p=14613
        vd->parent_window = new_parent;
        wnd->set_parent(new_parent);
        //::SetWindowLongPtr(vd->Hwnd, GWLP_HWNDPARENT, (LONG_PTR)vd->HwndParent);
    }

    // (Optional) Update Win32 style if it changed _after_ creation.
    // Generally they won't change unless configuration flags are changed, but advanced uses (such as manually rewriting viewport flags) make this useful.
    os::Window_Style new_style;
    os::Window_Style_Ex new_ex_style;
    imgui_viewport_flags_to_windown_style(viewport->Flags, &new_style, &new_ex_style);
    //ImGui_ImplWin32_GetWin32StyleFromViewportFlags(viewport->Flags, &new_style, &new_ex_style);

    // Only reapply the flags that have been changed from our point of view (as other flags are being modified by Windows)
    if (vd->window_style != new_style || vd->window_ex_style != new_ex_style)
    {
        // TODO: #support #imgui #gui
        // (Optional) Update TopMost state if it changed _after_ creation
        /*bool top_most_changed = (vd->window_ex_style & os::WINDOW_STYLE_EX_TOPMOST) != (new_ex_style & os::WINDOW_STYLE_EX_TOPMOST);
        HWND insert_after = top_most_changed ? ((viewport->Flags & ImGuiViewportFlags_TopMost) ? HWND_TOPMOST : HWND_NOTOPMOST) : 0;
        UINT swp_flag = top_most_changed ? 0 : SWP_NOZORDER;

        // Apply flags and position (since it is affected by flags)
        vd->DwStyle = new_style;
        vd->DwExStyle = new_ex_style;
        ::SetWindowLong(vd->Hwnd, GWL_STYLE, vd->DwStyle);
        ::SetWindowLong(vd->Hwnd, GWL_EXSTYLE, vd->DwExStyle);
        RECT rect = { (LONG)viewport->Pos.x, (LONG)viewport->Pos.y, (LONG)(viewport->Pos.x + viewport->Size.x), (LONG)(viewport->Pos.y + viewport->Size.y) };
        ::AdjustWindowRectEx(&rect, vd->DwStyle, FALSE, vd->DwExStyle); // Client to Screen
        ::SetWindowPos(vd->Hwnd, insert_after, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, swp_flag | SWP_NOACTIVATE | SWP_FRAMECHANGED);
        ::ShowWindow(vd->Hwnd, SW_SHOWNA); // This is necessary when we alter the style
        viewport->PlatformRequestMove = viewport->PlatformRequestResize = true;*/
    }
}

static ImVec2 imgui_callback_get_window_pos(ImGuiViewport* viewport) {
    auto wnd = (Render_Window*)viewport->PlatformHandle;
    ST_ASSERT(wnd != 0);
    
    auto wnd_pos = wnd->get_position();

    return ImVec2((float)wnd_pos.x, (float)wnd_pos.y);
}

static void imgui_callback_set_window_pos(ImGuiViewport* viewport, ImVec2 pos) {
    auto wnd = (Render_Window*)viewport->PlatformHandle;
    ST_ASSERT(wnd != 0);
    ST_ASSERT(wnd != ((Render_Context*)ImGui::GetIO().BackendPlatformUserData)->get_window());

    wnd->set_position({ pos.x, pos.y });
}

static ImVec2 imgui_callback_get_window_size(ImGuiViewport* viewport) {
    auto wnd = (Render_Window*)viewport->PlatformHandle;
    ST_ASSERT(wnd != 0);

    auto wnd_sz = wnd->get_size();

    return ImVec2((float)wnd_sz.x, (float)wnd_sz.y);
}

static void imgui_callback_set_window_size(ImGuiViewport* viewport, ImVec2 sz) {
    auto wnd = (Render_Window*)viewport->PlatformHandle;
    ST_ASSERT(wnd != 0);

    wnd->set_size({ sz.x, sz.y });
}

static void imgui_callback_set_window_focus(ImGuiViewport* viewport) {
    auto wnd = (Render_Window*)viewport->PlatformHandle;
    ST_ASSERT(wnd != 0);
    /*
    ::BringWindowToTop(vd->Hwnd);
    ::SetForegroundWindow(vd->Hwnd);
    ::SetFocus(vd->Hwnd);
    */

   wnd->set_focus(true);
}

static bool imgui_callback_get_window_focus(ImGuiViewport* viewport) {
    auto wnd = (Render_Window*)viewport->PlatformHandle;
    ST_ASSERT(wnd != 0);
    
    /* return ::GetForegroundWindow() == vd->Hwnd; */

   return wnd->is_focused();
}

static bool imgui_callback_get_window_minimized(ImGuiViewport* viewport) {
    auto wnd = (Render_Window*)viewport->PlatformHandle;
    ST_ASSERT(wnd != 0);
    
    /*return ::IsIconic(vd->Hwnd) != 0;*/

   return wnd->is_minimized();
}

static void imgui_callback_set_window_title(ImGuiViewport* viewport, const char* title) {
    auto wnd = (Render_Window*)viewport->PlatformHandle;
    ST_ASSERT(wnd != 0);
    
   wnd->set_title(title);
}

static void imgui_callback_set_window_alpha(ImGuiViewport* viewport, float alpha) {
    auto wnd = (Render_Window*)viewport->PlatformHandle;
    ST_ASSERT(wnd != 0);
    ST_ASSERT(alpha >= 0.0f && alpha <= 1.0f);
    /*
    ST_ASSERT(alpha >= 0.0f && alpha <= 1.0f);
    if (alpha < 1.0f)
    {
        DWORD style = ::GetWindowLongW(vd->Hwnd, GWL_EXSTYLE) | WS_EX_LAYERED;
        ::SetWindowLongW(vd->Hwnd, GWL_EXSTYLE, style);
        ::SetLayeredWindowAttributes(vd->Hwnd, 0, (BYTE)(255 * alpha), LWA_ALPHA);
    }
    else
    {
        DWORD style = ::GetWindowLongW(vd->Hwnd, GWL_EXSTYLE) & ~WS_EX_LAYERED;
        ::SetWindowLongW(vd->Hwnd, GWL_EXSTYLE, style);
    }*/

   wnd->set_alpha(alpha);
}

static float imgui_callback_get_window_dpi_scale(ImGuiViewport* viewport) {
    auto wnd = (Render_Window*)viewport->PlatformHandle;
    ST_ASSERT(wnd != 0);

    /*
    HMONITOR monitor = ::MonitorFromWindow((HWND)hwnd, MONITOR_DEFAULTTONEAREST);;
    */

    return os::env::get_monitor_dpi(wnd->get_monitor());
}



//////////////////////////////////////////////////////////

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

    auto wnd = _context->get_window();

    //POINT mouse_screen_pos;
    //bool has_mouse_screen_pos = ::GetCursorPos(&mouse_screen_pos) != 0;

    //HWND focused_window = ::GetForegroundWindow();

    

    const bool is_app_focused = os::env::is_app_focused();//(focused_window && (focused_window == bd->hWnd || ::IsChild(focused_window, bd->hWnd) || ImGui::FindViewportByPlatformHandle((void*)focused_window)));
    if (is_app_focused)
    {
        std::function<Render_Window*(Render_Window*)> find_focused;
        find_focused = [&find_focused](Render_Window* next) -> Render_Window* {
            if (next->is_focused()) return next;

            for (auto child : next->_children) {
                if (auto focused = find_focused(child)) {
                    return focused;
                }
            }
            return NULL;
        };
        Render_Window* focused_window = find_focused(wnd);

        //ST_ASSERT(focused_window, "A window was created without os:: namespace");


        // (Optional) Set OS mouse position from Dear ImGui if requested (rarely used, only when ImGuiConfigFlags_NavEnableSetMousePos is enabled by user)
        // When multi-viewports are enabled, all Dear ImGui positions are same as OS positions.
        if (io.WantSetMousePos)
        {
            mz::ivec2 pos = { io.MousePos.x, io.MousePos.y };
            if ((io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) == 0)
                pos = focused_window->client_to_screen(pos);
            os::env::set_cursor_pos(pos.x, pos.y);
        }

        // (Optional) Fallback to provide mouse position when focused (WM_MOUSEMOVE already provides this when hovered or captured)
        // This also fills a short gap when clicking non-client area: WM_NCMOUSELEAVE -> modal OS move -> gap -> WM_NCMOUSEMOVE
        /*if (!io.WantSetMousePos && bd->MouseTrackedArea == 0 && has_mouse_screen_pos)
        {
            // Single viewport mode: mouse position in client window coordinates (io.MousePos is (0,0) when the mouse is on the upper-left corner of the app window)
            // (This is the position you can get with ::GetCursorPos() + ::ScreenToClient() or WM_MOUSEMOVE.)
            // Multi-viewport mode: mouse position in OS absolute coordinates (io.MousePos is (0,0) when the mouse is on the upper-left of the primary monitor)
            // (This is the position you can get with ::GetCursorPos() or WM_MOUSEMOVE + ::ClientToScreen(). In theory adding viewport->Pos to a client position would also be the same.)
            POINT mouse_pos = mouse_screen_pos;
            if (!(io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable))
                ::ScreenToClient(bd->hWnd, &mouse_pos);
            io.AddMousePosEvent((float)mouse_pos.x, (float)mouse_pos.y);
        }*/
    }

    // (Optional) When using multiple viewports: call io.AddMouseViewportEvent() with the viewport the OS mouse cursor is hovering.
    // If ImGuiBackendFlags_HasMouseHoveredViewport is not set by the backend, Dear imGui will ignore this field and infer the information using its flawed heuristic.
    // - [X] Win32 backend correctly ignore viewports with the _NoInputs flag (here using ::WindowFromPoint with WM_NCHITTEST + HTTRANSPARENT in WndProc does that)
    //       Some backend are not able to handle that correctly. If a backend report an hovered viewport that has the _NoInputs flag (e.g. when dragging a window
    //       for docking, the viewport has the _NoInputs flag in order to allow us to find the viewport under), then Dear ImGui is forced to ignore the value reported
    //       by the backend, and use its flawed heuristic to guess the viewport behind.
    // - [X] Win32 backend correctly reports this regardless of another viewport behind focused and dragged from (we need this to find a useful drag and drop target).
    ImGuiID mouse_viewport_id = 0;
    std::function<Render_Window*(Render_Window*)> find_hovered;
    find_hovered = [&find_hovered](Render_Window* next) -> Render_Window* {
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
    auto wnd = (Render_Window*)(viewport->PlatformHandle);
    auto context = (Render_Context*)rend;
    context->set_target(wnd);
}

void imgui_callback_swap_buffers(ImGuiViewport* viewport, void* rend){
    auto wnd = (Render_Window*)(viewport->PlatformHandle);
    auto context = (Render_Context*)rend;
    context->set_target(wnd);
    wnd->swap_buffers();
}

bool ImGui_Renderer::_init_for_window_system() {

    _context->get_window()->add_event_callback(imgui_window_event_callback, this);

    ImGuiIO& io = ImGui::GetIO();
    io.BackendPlatformUserData = this;
    
    io.BackendPlatformName = "Stallout";
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;         // We can honor GetMouseCursor() values (optional)
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;          // We can honor io.WantSetMousePos requests (optional, rarely used)
    io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;    // We can create multi-viewports on the Platform side (optional)
    io.BackendFlags |= ImGuiBackendFlags_HasMouseHoveredViewport; // We can call io.AddMouseViewportEvent() with correct data (optional)

    _want_update_monitors = true;

    ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    main_viewport->PlatformHandle = main_viewport->PlatformHandleRaw =_context->get_window();
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
        
        main_viewport->PlatformHandle = _context->get_window();
        main_viewport->PlatformUserData = vd; 
    }

    return true;
}
void ImGui_Renderer::_shutdown_for_window_system() {
     ImGui::DestroyPlatformWindows();

   /* // Unload XInput library
#ifndef IMGUI_IMPL_WIN32_DISABLE_GAMEPAD
    if (bd->XInputDLL)
        ::FreeLibrary(bd->XInputDLL);
#endif // IMGUI_IMPL_WIN32_DISABLE_GAMEPAD*/

    ImGuiIO& io = ImGui::GetIO();
    io.BackendPlatformName = nullptr;
    io.BackendPlatformUserData = nullptr;
    io.BackendFlags &= ~(ImGuiBackendFlags_HasMouseCursors | ImGuiBackendFlags_HasSetMousePos | ImGuiBackendFlags_HasGamepad | ImGuiBackendFlags_PlatformHasViewports | ImGuiBackendFlags_HasMouseHoveredViewport);
}
void ImGui_Renderer::_new_frame_for_window_system(f32 delta_time) {
    ImGuiIO& io = ImGui::GetIO();
    // Setup display size (every frame to accommodate for window resizing)
    auto sz = _context->get_window()->get_size();
    io.DisplaySize = ImVec2((f32)sz.x, (f32)sz.y);

    if (_want_update_monitors)
        _update_monitors();

    // Setup time step
/*    INT64 current_time = 0;
    ::QueryPerformanceCounter((LARGE_INTEGER*)&current_time);*/
    io.DeltaTime = delta_time;
    //bd->Time = current_time;

    // Update OS mouse position
    _update_mouse_data();

    // Process workarounds for known Windows key handling issues
    // Left & right Shift keys: when both are pressed together, Windows tend to not generate the WM_KEYUP event for the first released one.
    if (ImGui::IsKeyDown(ImGuiKey_LeftShift) && !os::is_input_down(os::INPUT_CODE_LSHIFT)) {
        io.AddKeyEvent(ImGuiKey_LeftShift, false);
    }
    if (ImGui::IsKeyDown(ImGuiKey_RightShift) && !os::is_input_down(os::INPUT_CODE_RSHIFT)) {
        io.AddKeyEvent(ImGuiKey_RightShift, false);
    }

    // Sometimes WM_KEYUP for Win key is not passed down to the app (e.g. for Win+V on some setups, according to GLFW).
    if (ImGui::IsKeyDown(ImGuiKey_LeftSuper) && !os::is_input_down(os::INPUT_CODE_LSUPER)) {
        io.AddKeyEvent(ImGuiKey_LeftSuper, false);
    }
    if (ImGui::IsKeyDown(ImGuiKey_RightSuper) && !os::is_input_down(os::INPUT_CODE_RSUPER)) {
        io.AddKeyEvent(ImGuiKey_RightSuper, false);
    }

    // Update OS mouse cursor with the cursor requested by imgui
    ImGuiMouseCursor mouse_cursor = io.MouseDrawCursor ? ImGuiMouseCursor_None : ImGui::GetMouseCursor();
    if (_last_mouse_cursor != mouse_cursor)
    {
        _last_mouse_cursor = mouse_cursor;
        imgui_update_mouse_cursor();
    }

    // Update game controllers (if enabled and available)
    //ImGui_ImplWin32_UpdateGamepads();
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

    _context->query(QUERY_TYPE_RENDERER_SETTINGS_FLAGS, &_state_backup.settings);
    _context->query(QUERY_TYPE_BLENDING_EQUATION,  &_state_backup.blend_eq);
    _context->query(QUERY_TYPE_BLENDING_SRC_COLOR, &_state_backup.src_col);
    _context->query(QUERY_TYPE_BLENDING_DST_COLOR, &_state_backup.dst_col);
    _context->query(QUERY_TYPE_BLENDING_SRC_ALPHA, &_state_backup.src_alpha);
    _context->query(QUERY_TYPE_BLENDING_DST_ALPHA, &_state_backup.dst_alpha);
    _context->query(QUERY_TYPE_POLY_MODE_FRONT, &_state_backup.poly_mode_front);
    _context->query(QUERY_TYPE_POLY_MODE_BACK, &_state_backup.poly_mode_back);

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

    // TODO: #unfinished #ub #bugprone
    // Not sure how to be able to reset these as you can't really
    // query opengl for the shader or name of the block which was
    // used...
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
        // TODO: #performance #rendering #unfinished
        // Instead of reallocation the buffer on the GPU each time
        // we can just check if this round of buffers will fit in
        // the currently allocator buffers on the GPU.
        // (glBufferSubdata in the backend)

        _context->set_buffer(_hvbo, cmd_list->VtxBuffer.Data, vtx_buffer_size);
        _context->set_buffer(_hibo, cmd_list->IdxBuffer.Data, idx_buffer_size);
        //GL_CALL(glBufferData(GL_ARRAY_BUFFER, vtx_buffer_size, (const GLvoid*)cmd_list->VtxBuffer.Data, GL_STREAM_DRAW));
        //GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx_buffer_size, (const GLvoid*)cmd_list->IdxBuffer.Data, GL_STREAM_DRAW));

        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback != nullptr)
            {
                // User callback, registered via ImDrawList::AddCallback()
                // (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
                if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
                    _prepare_draw_call(draw_data, fb_width, fb_height);
                else
                    pcmd->UserCallback(cmd_list, pcmd);
            }
            else
            {
                // Project scissor/clipping rectangles into framebuffer space
                ImVec2 clip_min((pcmd->ClipRect.x - clip_off.x) * clip_scale.x, (pcmd->ClipRect.y - clip_off.y) * clip_scale.y);
                ImVec2 clip_max((pcmd->ClipRect.z - clip_off.x) * clip_scale.x, (pcmd->ClipRect.w - clip_off.y) * clip_scale.y);
                if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
                    continue;

                // Apply scissor/clipping rectangle (Y is inverted in OpenGL)
                //GL_CALL(glScissor((int)clip_min.x, (int)((float)fb_height - clip_max.y), (int)(clip_max.x - clip_min.x), (int)(clip_max.y - clip_min.y)));
                _context->set_scissor_box({(int)clip_min.x, (int)((float)fb_height - clip_max.y), (int)(clip_max.x - clip_min.x), (int)(clip_max.y - clip_min.y)});

                // Bind texture, Draw
                //GL_CALL(glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->GetTexID()));
                //GL_CALL(glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, (void*)(intptr_t)(pcmd->IdxOffset * sizeof(ImDrawIdx))));
                _context->draw_indexed(
                    _hlayout, _hvbo, _hibo, _hshader, 
                    pcmd->ElemCount, 
                    sizeof(ImDrawIdx) == 2 ? DATA_TYPE_USHORT : DATA_TYPE_UINT, 
                    pcmd->IdxOffset
                );
            }
        }
    }

    /*_context->disable(
        RENDERER_SETTING_CULLING & 
        RENDERER_SETTING_DEPTH_TESTING &
        RENDERER_SETTING_STENCIL_TESTING &
        RENDERER_SETTING_PRIMITIVE_RESTART
    );
    _context->enable(RENDERER_SETTING_SCISSOR_TESTING);*/
    // Restore state
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
}

bool ImGui_Renderer::_init_for_renderer() {

    auto& io = ImGui::GetIO();
    io.BackendRendererName = "StalloutRenderer";
    io.BackendRendererUserData = (void*)this;

    auto env = _context->get_environment();
    
    /*if (env.version_major >= 3 && env.version_minor >= 2) {
        io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
    }*/
    io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;

    ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
    platform_io.Renderer_RenderWindow = render_window;

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

ImGui_Renderer::ImGui_Renderer(Render_Context* renderer) 
    : _context(renderer) {

    _imgui_context = ImGui::CreateContext();
    ST_ASSERT(_imgui_context, "Failed creating imgui context");
    ImGui::SetCurrentContext((ImGuiContext*)_imgui_context);
    ImGui::StyleColorsLight();

    // Need to use some rendering resources
    // in init (window) which are initialized
    // on the render thread so need to wait
    // to make sure it's ready.
    _context->wait_ready();

    auto& io = ImGui::GetIO();

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

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

#endif
NS_END(renderer);
NS_END(engine);

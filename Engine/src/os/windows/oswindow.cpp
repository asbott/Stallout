#pragma once


#include "pch.h"

#include "os/oswindow.h"

#include "Engine/logger.h"

NS_BEGIN(os);
u64 to_os_input_code(Input_Code input) {
    switch(input) {
        case INPUT_CODE_A: return 'A';
        case INPUT_CODE_B: return 'B';
        case INPUT_CODE_C: return 'C';
        case INPUT_CODE_D: return 'D';
        case INPUT_CODE_E: return 'E';
        case INPUT_CODE_F: return 'F';
        case INPUT_CODE_G: return 'G';
        case INPUT_CODE_H: return 'H';
        case INPUT_CODE_I: return 'I';
        case INPUT_CODE_J: return 'J';
        case INPUT_CODE_K: return 'K';
        case INPUT_CODE_L: return 'L';
        case INPUT_CODE_M: return 'M';
        case INPUT_CODE_N: return 'N';
        case INPUT_CODE_O: return 'O';
        case INPUT_CODE_P: return 'P';
        case INPUT_CODE_Q: return 'Q';
        case INPUT_CODE_R: return 'R';
        case INPUT_CODE_S: return 'S';
        case INPUT_CODE_T: return 'T';
        case INPUT_CODE_U: return 'U';
        case INPUT_CODE_V: return 'V';
        case INPUT_CODE_W: return 'W';
        case INPUT_CODE_X: return 'X';
        case INPUT_CODE_Y: return 'Y';
        case INPUT_CODE_Z: return 'Z';

        case INPUT_CODE_0: return '0';
        case INPUT_CODE_1: return '1';
        case INPUT_CODE_2: return '2';
        case INPUT_CODE_3: return '3';
        case INPUT_CODE_4: return '4';
        case INPUT_CODE_5: return '5';
        case INPUT_CODE_6: return '6';
        case INPUT_CODE_7: return '7';
        case INPUT_CODE_8: return '8';
        case INPUT_CODE_9: return '9';

        case INPUT_CODE_F1: return VK_F1;
        case INPUT_CODE_F2: return VK_F2;
        case INPUT_CODE_F3: return VK_F3;
        case INPUT_CODE_F4: return VK_F4;
        case INPUT_CODE_F5: return VK_F5;
        case INPUT_CODE_F6: return VK_F6;
        case INPUT_CODE_F7: return VK_F7;
        case INPUT_CODE_F8: return VK_F8;
        case INPUT_CODE_F9: return VK_F9;
        case INPUT_CODE_F10: return VK_F10;
        case INPUT_CODE_F11: return VK_F11;
        case INPUT_CODE_F12: return VK_F12;

        case INPUT_CODE_SPACE: return VK_SPACE;
        case INPUT_CODE_TAB: return VK_TAB;
        case INPUT_CODE_ENTER: return VK_RETURN;
        case INPUT_CODE_ESCAPE: return VK_ESCAPE;
        case INPUT_CODE_BACKSPACE: return VK_BACK;
        case INPUT_CODE_SHIFT: return VK_SHIFT;
        case INPUT_CODE_CTRL: return VK_CONTROL;
        case INPUT_CODE_ALT: return VK_MENU;

        case INPUT_CODE_COMMA: return VK_OEM_COMMA;
        case INPUT_CODE_PERIOD: return VK_OEM_PERIOD;
        case INPUT_CODE_SEMICOLON: return VK_OEM_1;
        case INPUT_CODE_APOSTROPHE: return VK_OEM_7;
        case INPUT_CODE_BRACKET_OPEN: return VK_OEM_4;
        case INPUT_CODE_BRACKET_CLOSE: return VK_OEM_6;
        case INPUT_CODE_BACKSLASH: return VK_OEM_5;
        case INPUT_CODE_SLASH: return VK_OEM_2;
        case INPUT_CODE_MINUS: return VK_OEM_MINUS;
        case INPUT_CODE_EQUALS: return VK_OEM_PLUS;

        case INPUT_CODE_MOUSE_LEFT: return VK_LBUTTON;
        case INPUT_CODE_MOUSE_RIGHT: return VK_RBUTTON;
        case INPUT_CODE_MOUSE_MIDDLE: return VK_MBUTTON;
        case INPUT_CODE_MOUSE_BUTTON4: return VK_XBUTTON1;
        case INPUT_CODE_MOUSE_BUTTON5: return VK_XBUTTON2;

        default: ST_ASSERT(false); return 0;  // Undefined or unhandled input code
    }
}

Input_Code to_st_input_code(u64 os_code) {
    switch(os_code) {
        case 'A': return INPUT_CODE_A;
        case 'B': return INPUT_CODE_B;
        case 'C': return INPUT_CODE_C;
        case 'D': return INPUT_CODE_D;
        case 'E': return INPUT_CODE_E;
        case 'F': return INPUT_CODE_F;
        case 'G': return INPUT_CODE_G;
        case 'H': return INPUT_CODE_H;
        case 'I': return INPUT_CODE_I;
        case 'J': return INPUT_CODE_J;
        case 'K': return INPUT_CODE_K;
        case 'L': return INPUT_CODE_L;
        case 'M': return INPUT_CODE_M;
        case 'N': return INPUT_CODE_N;
        case 'O': return INPUT_CODE_O;
        case 'P': return INPUT_CODE_P;
        case 'Q': return INPUT_CODE_Q;
        case 'R': return INPUT_CODE_R;
        case 'S': return INPUT_CODE_S;
        case 'T': return INPUT_CODE_T;
        case 'U': return INPUT_CODE_U;
        case 'V': return INPUT_CODE_V;
        case 'W': return INPUT_CODE_W;
        case 'X': return INPUT_CODE_X;
        case 'Y': return INPUT_CODE_Y;
        case 'Z': return INPUT_CODE_Z;

        case '0': return INPUT_CODE_0;
        case '1': return INPUT_CODE_1;
        case '2': return INPUT_CODE_2;
        case '3': return INPUT_CODE_3;
        case '4': return INPUT_CODE_4;
        case '5': return INPUT_CODE_5;
        case '6': return INPUT_CODE_6;
        case '7': return INPUT_CODE_7;
        case '8': return INPUT_CODE_8;
        case '9': return INPUT_CODE_9;

        case VK_F1: return INPUT_CODE_F1;
        case VK_F2: return INPUT_CODE_F2;
        case VK_F3: return INPUT_CODE_F3;
        case VK_F4: return INPUT_CODE_F4;
        case VK_F5: return INPUT_CODE_F5;
        case VK_F6: return INPUT_CODE_F6;
        case VK_F7: return INPUT_CODE_F7;
        case VK_F8: return INPUT_CODE_F8;
        case VK_F9: return INPUT_CODE_F9;
        case VK_F10: return INPUT_CODE_F10;
        case VK_F11: return INPUT_CODE_F11;
        case VK_F12: return INPUT_CODE_F12;

        case VK_SPACE: return INPUT_CODE_SPACE;
        case VK_TAB: return INPUT_CODE_TAB;
        case VK_RETURN: return INPUT_CODE_ENTER;
        case VK_ESCAPE: return INPUT_CODE_ESCAPE;
        case VK_BACK: return INPUT_CODE_BACKSPACE;
        case VK_SHIFT: return INPUT_CODE_SHIFT;
        case VK_CONTROL: return INPUT_CODE_CTRL;
        case VK_MENU: return INPUT_CODE_ALT;

        case VK_OEM_COMMA: return INPUT_CODE_COMMA;
        case VK_OEM_PERIOD: return INPUT_CODE_PERIOD;
        case VK_OEM_1: return INPUT_CODE_SEMICOLON;
        case VK_OEM_7: return INPUT_CODE_APOSTROPHE;
        case VK_OEM_4: return INPUT_CODE_BRACKET_OPEN;
        case VK_OEM_6: return INPUT_CODE_BRACKET_CLOSE;
        case VK_OEM_5: return INPUT_CODE_BACKSLASH;
        case VK_OEM_2: return INPUT_CODE_SLASH;
        case VK_OEM_MINUS: return INPUT_CODE_MINUS;
        case VK_OEM_PLUS: return INPUT_CODE_EQUALS;

        case VK_LBUTTON: return INPUT_CODE_MOUSE_LEFT;
        case VK_RBUTTON: return INPUT_CODE_MOUSE_RIGHT;
        case VK_MBUTTON: return INPUT_CODE_MOUSE_MIDDLE;
        case VK_XBUTTON1: return INPUT_CODE_MOUSE_BUTTON4;
        case VK_XBUTTON2: return INPUT_CODE_MOUSE_BUTTON5;

        default: ST_ASSERT(false); return INPUT_CODE_COUNT;  // Undefined or unhandled input code
    }
}

struct Internal {
    bool exit_flag = false;
};
static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    Window* pThis = nullptr;
    if (uMsg == WM_CREATE) {
        pThis = (Window*)((CREATESTRUCT*)lParam)->lpCreateParams;
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pThis);
    } else {
        pThis = (Window*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
    }

    if (pThis) {
        switch (uMsg) {
        case WM_DESTROY:
            ((Internal*)pThis->__internal)->exit_flag = true;
            PostQuitMessage(0);
            return 0;
        }
    }
    return DefWindowProcA(hWnd, uMsg, wParam, lParam);
}



Window::Window(const Window_Init_Spec& wnd_spec, Window* parent) {

    memset(input._input_states, 0, sizeof(input._input_states));

    static WNDCLASSEXA *wc = NULL; 
    if (!wc) {
        wc = (WNDCLASSEXA*)ST_MEM(sizeof(WNDCLASSEXA));
        memset(wc, 0, sizeof(WNDCLASSEXA));
        wc->cbSize = sizeof(WNDCLASSEXA);
        wc->style = CS_OWNDC;
        wc->lpfnWndProc = WindowProc;
        wc->hInstance = GetModuleHandleA(nullptr);
        wc->lpszClassName = "StalloutWindowClass";
        ST_ASSERT(RegisterClassExA(wc), "Window class registration failed");
    }
    

    
    _os_handle = CreateWindowExA(
        0,
        "StalloutWindowClass",
        wnd_spec.title,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        static_cast<int>(wnd_spec.width),
        static_cast<int>(wnd_spec.height),
        parent ? (HWND)parent->_os_handle : nullptr,
        nullptr,
        GetModuleHandleA(nullptr),
        nullptr
    );

    _is_main = parent == NULL;

    ST_ASSERT(_os_handle, "Window creation failed");

    SetWindowLongPtr((HWND)_os_handle, GWLP_USERDATA, (LONG_PTR)this);

    ShowWindow((HWND)_os_handle, wnd_spec.visible ? SW_SHOW : SW_HIDE);

    __internal = ST_NEW(Internal);

    log_info("Window '{}' was created", wnd_spec.title);
}

Window::~Window() {
    DestroyWindow((HWND)_os_handle);
    for (auto child : _children) {
        ST_DELETE(child);
    }
}

void Window::resize(size_t width, size_t height) {
    SetWindowPos((HWND)_os_handle, nullptr, 0, 0, static_cast<int>(width), static_cast<int>(height), SWP_NOMOVE | SWP_NOZORDER);
}

void Window::move_to(size_t x, size_t y) {
    SetWindowPos((HWND)_os_handle, nullptr, static_cast<int>(x), static_cast<int>(y), 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

void Window::set_fullscreen(bool fullscreen) {
    if (fullscreen) {
        SetWindowLongA((HWND)_os_handle, GWL_STYLE, GetWindowLongA((HWND)_os_handle, GWL_STYLE) & ~WS_OVERLAPPEDWINDOW);
    } else {
        SetWindowLongA((HWND)_os_handle, GWL_STYLE, GetWindowLongA((HWND)_os_handle, GWL_STYLE) | WS_OVERLAPPEDWINDOW);
    }
}

void Window::set_title(const char* title) {
    SetWindowTextA((HWND)_os_handle, title);
}


void Window::set_focus(bool focused) {
    if (focused) {
        SetFocus((HWND)_os_handle);
    }
    else {
        
    }
}

void Window::set_visibility(bool visible) {
    if (visible) {
        ShowWindow((HWND)_os_handle, SW_SHOW);
    } else {
        ShowWindow((HWND)_os_handle, SW_HIDE);
    }
}

Window* Window::add_child(const Window_Init_Spec& wnd_spec) {
    
    _children.push_back(ST_NEW(Window, wnd_spec, this));

    return _children.back();
}

Window* Window::get_child(size_t index) {
    if (index >= _children.size()) {
        return nullptr;
    }
    return _children[index];
}

size_t Window::get_child_count() {
    return _children.size();
}

bool Window::exit_flag() const {
    return ((Internal*)__internal)->exit_flag;
}

void Window::swap_buffers() {
    auto hdc = GetDC((HWND)_os_handle);
    SwapBuffers(hdc);
    ReleaseDC((HWND)_os_handle, hdc);

    MSG msg;
    while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);

        // Handle keyboard and mouse input
        if (msg.message >= WM_KEYFIRST && msg.message <= WM_KEYLAST) {
            Input_Code code = static_cast<Input_Code>(to_st_input_code(msg.wParam));

            input._input_states[code] = (msg.message == WM_KEYDOWN) ? INPUT_STATE_DOWN : INPUT_STATE_UP;
        }
        if (msg.message == WM_MOUSEMOVE) {
            input._mouse_x = static_cast<double>(LOWORD(msg.lParam)); 
            input._mouse_y = static_cast<double>(HIWORD(msg.lParam));
        }
    }
}

Input_State Window::Input::get_state(Input_Code code) const {
    return _input_states[code];
}


NS_END(os);
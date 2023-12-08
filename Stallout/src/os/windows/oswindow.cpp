#pragma once //


#include "pch.h"

#include "os/oswindow.h"
#include "os/env.h"

#include "os/graphics.h"

#include "Stallout/logger.h"

#undef UNICODE
#include "Windows.h"
#include "os/windows/winutils.h"

#include "Stallout/timing.h"

NS_BEGIN(stallout);
NS_BEGIN(os);
UINT to_win32_input_code(Input_Code code) {
    switch(code) {
        // Alphabets
        case INPUT_CODE_A:              return 'A';
        case INPUT_CODE_B:              return 'B';
        case INPUT_CODE_C:              return 'C';
        case INPUT_CODE_D:              return 'D';
        case INPUT_CODE_E:              return 'E';
        case INPUT_CODE_F:              return 'F';
        case INPUT_CODE_G:              return 'G';
        case INPUT_CODE_H:              return 'H';
        case INPUT_CODE_I:              return 'I';
        case INPUT_CODE_J:              return 'J';
        case INPUT_CODE_K:              return 'K';
        case INPUT_CODE_L:              return 'L';
        case INPUT_CODE_M:              return 'M';
        case INPUT_CODE_N:              return 'N';
        case INPUT_CODE_O:              return 'O';
        case INPUT_CODE_P:              return 'P';
        case INPUT_CODE_Q:              return 'Q';
        case INPUT_CODE_R:              return 'R';
        case INPUT_CODE_S:              return 'S';
        case INPUT_CODE_T:              return 'T';
        case INPUT_CODE_U:              return 'U';
        case INPUT_CODE_V:              return 'V';
        case INPUT_CODE_W:              return 'W';
        case INPUT_CODE_X:              return 'X';
        case INPUT_CODE_Y:              return 'Y';
        case INPUT_CODE_Z:              return 'Z';

        // Numbers
        case INPUT_CODE_0:              return '0';
        case INPUT_CODE_1:              return '1';
        case INPUT_CODE_2:              return '2';
        case INPUT_CODE_3:              return '3';
        case INPUT_CODE_4:              return '4';
        case INPUT_CODE_5:              return '5';
        case INPUT_CODE_6:              return '6';
        case INPUT_CODE_7:              return '7';
        case INPUT_CODE_8:              return '8';
        case INPUT_CODE_9:              return '9';

        // Numpad
        case INPUT_CODE_NUMPAD0:        return VK_NUMPAD0;
        case INPUT_CODE_NUMPAD1:        return VK_NUMPAD1;
        case INPUT_CODE_NUMPAD2:        return VK_NUMPAD2;
        case INPUT_CODE_NUMPAD3:        return VK_NUMPAD3;
        case INPUT_CODE_NUMPAD4:        return VK_NUMPAD4;
        case INPUT_CODE_NUMPAD5:        return VK_NUMPAD5;
        case INPUT_CODE_NUMPAD6:        return VK_NUMPAD6;
        case INPUT_CODE_NUMPAD7:        return VK_NUMPAD7;
        case INPUT_CODE_NUMPAD8:        return VK_NUMPAD8;
        case INPUT_CODE_NUMPAD9:        return VK_NUMPAD9;

        // Function keys
        case INPUT_CODE_F1:             return VK_F1;
        case INPUT_CODE_F2:             return VK_F2;
        case INPUT_CODE_F3:             return VK_F3;
        case INPUT_CODE_F4:             return VK_F4;
        case INPUT_CODE_F5:             return VK_F5;
        case INPUT_CODE_F6:             return VK_F6;
        case INPUT_CODE_F7:             return VK_F7;
        case INPUT_CODE_F8:             return VK_F8;
        case INPUT_CODE_F9:             return VK_F9;
        case INPUT_CODE_F10:            return VK_F10;
        case INPUT_CODE_F11:            return VK_F11;
        case INPUT_CODE_F12:            return VK_F12;

        // Special keys
        case INPUT_CODE_SPACE:          return VK_SPACE;
        case INPUT_CODE_TAB:            return VK_TAB;
        case INPUT_CODE_ENTER:          return VK_RETURN;
        case INPUT_CODE_ESCAPE:         return VK_ESCAPE;
        case INPUT_CODE_BACKSPACE:      return VK_BACK;
        case INPUT_CODE_CTRL:           return VK_CONTROL;
        case INPUT_CODE_ALT:            return VK_MENU;

        case INPUT_CODE_LSHIFT:         return VK_SHIFT;
        case INPUT_CODE_LCTRL:          return VK_CONTROL;
        case INPUT_CODE_LALT:           return VK_LMENU;
        case INPUT_CODE_LSUPER:         return VK_LWIN;
        case INPUT_CODE_RSHIFT:         return VK_RSHIFT;
        case INPUT_CODE_RCTRL:          return VK_RCONTROL;
        case INPUT_CODE_RALT:           return VK_RMENU;
        case INPUT_CODE_RSUPER:         return VK_RWIN;

        case INPUT_CODE_NUMPAD_ENTER:   return VK_SEPARATOR;  // You might want to verify this mapping

        // Symbols and other keys
        case INPUT_CODE_COMMA:          return VK_OEM_COMMA;
        case INPUT_CODE_PERIOD:         return VK_OEM_PERIOD;
        case INPUT_CODE_SEMICOLON:      return VK_OEM_1;
        case INPUT_CODE_APOSTROPHE:     return VK_OEM_7;
        case INPUT_CODE_BRACKET_OPEN:   return VK_OEM_4;
        case INPUT_CODE_BRACKET_CLOSE:  return VK_OEM_6;
        case INPUT_CODE_BACKSLASH:      return VK_OEM_5;
        case INPUT_CODE_SLASH:          return VK_OEM_2;
        case INPUT_CODE_MINUS:          return VK_OEM_MINUS;
        case INPUT_CODE_PLUS:           // This might be a complex combination
        case INPUT_CODE_EQUALS:         return VK_OEM_PLUS;
        case INPUT_CODE_DECIMAL:        return VK_DECIMAL;
        case INPUT_CODE_DIVIDE:         return VK_DIVIDE;
        case INPUT_CODE_MULTIPLY:       return VK_MULTIPLY;
        case INPUT_CODE_SUBTRACT:       return VK_SUBTRACT;
        case INPUT_CODE_ADD:            return VK_ADD;

        // Mouse buttons
        case INPUT_CODE_MOUSE_LEFT:     return VK_LBUTTON;
        case INPUT_CODE_MOUSE_RIGHT:    return VK_RBUTTON;
        case INPUT_CODE_MOUSE_MIDDLE:   return VK_MBUTTON;
        //case INPUT_CODE_MOUSE_BUTTON4:  // Win32 doesn't have a direct VK code, consider using XBUTTON1/XBUTTON2 and checking the high word of wParam for distinction
        //case INPUT_CODE_MOUSE_BUTTON5:  // Win32 doesn't have a direct VK code

        // Arrow keys
        case INPUT_CODE_LEFT_ARROW:     return VK_LEFT;
        case INPUT_CODE_RIGHT_ARROW:    return VK_RIGHT;
        case INPUT_CODE_UP_ARROW:       return VK_UP;
        case INPUT_CODE_DOWN_ARROW:     return VK_DOWN;

        // Other keys
        case INPUT_CODE_PAGEUP:         return VK_PRIOR;
        case INPUT_CODE_PAGEDOWN:       return VK_NEXT;
        case INPUT_CODE_HOME:           return VK_HOME;
        case INPUT_CODE_END:            return VK_END;
        case INPUT_CODE_INSERT:         return VK_INSERT;
        case INPUT_CODE_DELETE:         return VK_DELETE;
        case INPUT_CODE_BACK:           // This is ambiguous. Could it be backspace or a navigation "back" key?

        case INPUT_CODE_GRAVEACCENT:    return VK_OEM_3;
        case INPUT_CODE_LEFT_BRACKET:   return VK_OEM_4; // Same as BRACKET_OPEN
        case INPUT_CODE_RIGHT_BRACKET:  return VK_OEM_6; // Same as BRACKET_CLOSE

        case INPUT_CODE_CAPSLOCK:       return VK_CAPITAL;
        case INPUT_CODE_SCROLL:         return VK_SCROLL;
        case INPUT_CODE_NUMLOCK:        return VK_NUMLOCK;
        case INPUT_CODE_SNAPSHOT:       return VK_SNAPSHOT;
        case INPUT_CODE_PAUSE:          return VK_PAUSE;

        // Default or unknown key
        default:                        return 0;
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
        case VK_SHIFT: return INPUT_CODE_LSHIFT;
        case VK_LSHIFT: return INPUT_CODE_LSHIFT;
        case VK_CONTROL: return INPUT_CODE_LCTRL;
        case VK_LCONTROL: return INPUT_CODE_LCTRL;
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

        case VK_LEFT: return INPUT_CODE_LEFT_ARROW;
        case VK_RIGHT: return INPUT_CODE_RIGHT_ARROW;
        case VK_UP: return INPUT_CODE_UP_ARROW;
        case VK_DOWN: return INPUT_CODE_DOWN_ARROW;

        case VK_NUMPAD0: return INPUT_CODE_NUMPAD0;
        case VK_NUMPAD1: return INPUT_CODE_NUMPAD1;
        case VK_NUMPAD2: return INPUT_CODE_NUMPAD2;
        case VK_NUMPAD3: return INPUT_CODE_NUMPAD3;
        case VK_NUMPAD4: return INPUT_CODE_NUMPAD4;
        case VK_NUMPAD5: return INPUT_CODE_NUMPAD5;
        case VK_NUMPAD6: return INPUT_CODE_NUMPAD6;
        case VK_NUMPAD7: return INPUT_CODE_NUMPAD7;
        case VK_NUMPAD8: return INPUT_CODE_NUMPAD8;
        case VK_NUMPAD9: return INPUT_CODE_NUMPAD9;

        case VK_LWIN: return INPUT_CODE_LSUPER;
        case VK_RWIN: return INPUT_CODE_RSUPER;

        default: log_warn("Unhandled win32 input key {}", os_code); return INPUT_CODE_UNKNOWN;
    }
}

bool is_input_down(Input_Code input) {
    return (::GetKeyState(to_win32_input_code(input)) & 0x8000) != 0;
}

unsigned int to_win32_enum(Window_Style style) {
    unsigned int s = 0;

    if (style & WINDOW_STYLE_OVERLAPPED) s |= WS_OVERLAPPED;
    if (style & WINDOW_STYLE_POPUP) s |= WS_POPUP;
    if (style & WINDOW_STYLE_CHILD) s |= WS_CHILD;
    if (style & WINDOW_STYLE_MINIMIZE) s |= WS_MINIMIZE;
    if (style & WINDOW_STYLE_MAXIMIZE) s |= WS_MAXIMIZE;
    if (style & WINDOW_STYLE_BORDER) s |= WS_BORDER;
    if (style & WINDOW_STYLE_CAPTION) s |= WS_CAPTION;
    if (style & WINDOW_STYLE_SYSMENU) s |= WS_SYSMENU;
    if (style & WINDOW_STYLE_THICKFRAME) s |= WS_THICKFRAME;
    if (style & WINDOW_STYLE_MINIMIZEBOX) s |= WS_MINIMIZEBOX;
    if (style & WINDOW_STYLE_MAXIMIZEBOX) s |= WS_MAXIMIZEBOX;
    if (style & WINDOW_STYLE_VISIBLE) s |= WS_VISIBLE;
    if (style & WINDOW_STYLE_DISABLED) s |= WS_DISABLED;
    if (style & WINDOW_STYLE_OVERLAPPEDWINDOW) s |= WS_OVERLAPPEDWINDOW;

    return s;
}
unsigned int to_win32_enum(Window_Style_Ex style) {
    unsigned int s = 0;

    if (style & WINDOW_STYLE_EX_TOOLWINDOW) s |= WS_EX_TOOLWINDOW;
    if (style & WINDOW_STYLE_EX_APPWINDOW) s |= WS_EX_APPWINDOW;
    if (style & WINDOW_STYLE_EX_TOPMOST) s |= WS_EX_TOPMOST;

    return s;
}

/*UINT to_win32_enum(Window_Event_Type event) {
    switch(event) {
        case WINDOW_EVENT_TYPE_CLOSE:          return WM_CLOSE;
        case WINDOW_EVENT_TYPE_MOVE:           return WM_MOVE;
        case WINDOW_EVENT_TYPE_SIZE:           return WM_SIZE;
        case WINDOW_EVENT_TYPE_MOUSEACTIVATE:  return WM_MOUSEACTIVATE;
        case WINDOW_EVENT_TYPE_NCHITTEST:      return WM_NCHITTEST;
        case WINDOW_EVENT_TYPE_MOUSEWHEEL:     return WM_MOUSEWHEEL;
        case WINDOW_EVENT_TYPE_MOUSEMOVE:      return WM_MOUSEMOVE;
        case WINDOW_EVENT_TYPE_NCMOUSEMOVE:    return WM_NCMOUSEMOVE;
        case WINDOW_EVENT_TYPE_MOUSELEAVE:     return WM_MOUSELEAVE;
        case WINDOW_EVENT_TYPE_NCMOUSELEAVE:   return WM_NCMOUSELEAVE; // Note: Actual Win32 message might differ
        case WINDOW_EVENT_TYPE_LBUTTONDOWN:    return WM_LBUTTONDOWN;
        case WINDOW_EVENT_TYPE_RBUTTONDOWN:    return WM_RBUTTONDOWN;
        case WINDOW_EVENT_TYPE_MBUTTONDOWN:    return WM_MBUTTONDOWN;
        case WINDOW_EVENT_TYPE_XBUTTONDOWN:    return WM_XBUTTONDOWN;
        case WINDOW_EVENT_TYPE_LBUTTONDBLCLK:  return WM_LBUTTONDBLCLK;
        case WINDOW_EVENT_TYPE_RBUTTONDBLCLK:  return WM_RBUTTONDBLCLK;
        case WINDOW_EVENT_TYPE_MBUTTONDBLCLK:  return WM_MBUTTONDBLCLK;
        case WINDOW_EVENT_TYPE_XBUTTONDBLCLK:  return WM_XBUTTONDBLCLK;
        case WINDOW_EVENT_TYPE_LBUTTONUP:      return WM_LBUTTONUP;
        case WINDOW_EVENT_TYPE_RBUTTONUP:      return WM_RBUTTONUP;
        case WINDOW_EVENT_TYPE_MBUTTONUP:      return WM_MBUTTONUP;
        case WINDOW_EVENT_TYPE_XBUTTONUP:      return WM_XBUTTONUP;
        case WINDOW_EVENT_TYPE_KEYDOWN:        return WM_KEYDOWN;
        case WINDOW_EVENT_TYPE_KEYUP:          return WM_KEYUP;
        case WINDOW_EVENT_TYPE_SYSKEYDOWN:     return WM_SYSKEYDOWN;
        case WINDOW_EVENT_TYPE_SYSKEYUP:       return WM_SYSKEYUP;
        case WINDOW_EVENT_TYPE_CHAR:           return WM_CHAR;
        case WINDOW_EVENT_TYPE_SETFOCUS:       return WM_SETFOCUS;
        case WINDOW_EVENT_TYPE_KILLFOCUS:      return WM_KILLFOCUS;
        case WINDOW_EVENT_TYPE_SETCURSOR:      return WM_SETCURSOR;
        case WINDOW_EVENT_TYPE_DEVICECHANGE:   return WM_DEVICECHANGE;
        case WINDOW_EVENT_TYPE_DISPLAYCHANGE:  return WM_DISPLAYCHANGE;
        default: return 0;  // Can't crash because windows has countless events
    }
}*/

Window_Event_Type to_event_type(UINT win32Message) {
    switch(win32Message) {
        case WM_CLOSE:           return WINDOW_EVENT_TYPE_CLOSE;
        case WM_MOVE:            return WINDOW_EVENT_TYPE_MOVE;
        case WM_SIZE:            return WINDOW_EVENT_TYPE_SIZE;
        case WM_MOUSEACTIVATE:   return WINDOW_EVENT_TYPE_MOUSEACTIVATE;
        case WM_NCHITTEST:       return WINDOW_EVENT_TYPE_NCHITTEST;

        case WM_MOUSEWHEEL: case WM_MOUSEHWHEEL:      
            return WINDOW_EVENT_TYPE_MOUSE_SCROLL;

        case WM_MOUSEMOVE: case WM_NCMOUSEMOVE:     
            return WINDOW_EVENT_TYPE_MOUSEMOVE;

        //case WM_MOUSELEAVE:      return WINDOW_EVENT_TYPE_MOUSELEAVE;
        //case WM_NCMOUSELEAVE:    return WINDOW_EVENT_TYPE_NCMOUSELEAVE;

        case WM_LBUTTONDOWN:   case WM_RBUTTONDOWN:   case WM_MBUTTONDOWN:   case WM_XBUTTONDOWN:     
        case WM_LBUTTONDBLCLK: case WM_RBUTTONDBLCLK: case WM_MBUTTONDBLCLK: case WM_XBUTTONDBLCLK:   
        case WM_LBUTTONUP:     case WM_RBUTTONUP:     case WM_MBUTTONUP:     case WM_XBUTTONUP:       
            return WINDOW_EVENT_TYPE_MOUSE_BUTTON;

        case WM_KEYDOWN:    case WM_KEYUP:
        case WM_SYSKEYDOWN: case WM_SYSKEYUP:
            return WINDOW_EVENT_TYPE_KEY;
        case WM_CHAR:            return WINDOW_EVENT_TYPE_CHAR;
        case WM_SETFOCUS:        return WINDOW_EVENT_TYPE_SETFOCUS;
        case WM_KILLFOCUS:       return WINDOW_EVENT_TYPE_KILLFOCUS;
        case WM_SETCURSOR:       return WINDOW_EVENT_TYPE_SETCURSOR;
        case WM_DEVICECHANGE:    return WINDOW_EVENT_TYPE_DEVICECHANGE;
        case WM_DISPLAYCHANGE:   return WINDOW_EVENT_TYPE_DISPLAYCHANGE;
        default: 
            return WINDOW_EVENT_TYPE_UNIMPLEMENTED;
    }
}

Input_Code win32_message_to_input_code(UINT message, WPARAM wParam) {
    switch (message) {
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_LBUTTONDBLCLK:
            return INPUT_CODE_MOUSE_LEFT;

        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_RBUTTONDBLCLK:
            return INPUT_CODE_MOUSE_RIGHT;

        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
        case WM_MBUTTONDBLCLK:
            return INPUT_CODE_MOUSE_MIDDLE;

        case WM_XBUTTONDOWN:
        case WM_XBUTTONUP:
        case WM_XBUTTONDBLCLK:
        {
            switch (HIWORD(wParam)) {
                case XBUTTON1: return INPUT_CODE_MOUSE_SIDE1;
                case XBUTTON2: return INPUT_CODE_MOUSE_SIDE2;
            }
            return INPUT_CODE_MOUSE_BUTTON4;
        }

        default:
            INTENTIONAL_CRASH("Unhandled enum");
            return INPUT_CODE_COUNT;  // Unknown/Unhandled message
    }
}

Mouse_Source get_mouse_source() {
    LPARAM extra_info = ::GetMessageExtraInfo();
    if ((extra_info & 0xFFFFFF80) == 0xFF515700)
        return MOUSE_SOURCE_PEN;
    else if ((extra_info & 0xFFFFFF80) == 0xFF515780)
        return MOUSE_SOURCE_TOUCHSCREEN;
    else
        return MOUSE_SOURCE_MOUSE;
}

void* event_from_win32_message(os::Window* wnd, UINT message, WPARAM wParam, LPARAM lParam) {

    #define __NEW_EVENT(t) stnew (t) /*event_buffer->allocate_and_construct<t>()*/
    
    switch(message) {
        case WM_MOUSEMOVE:
        case WM_NCMOUSEMOVE:
        {         
            s32 x = LOWORD(lParam); 
            s32 y = HIWORD(lParam);
      
            auto e = __NEW_EVENT(Mouse_Move_Event);

            e->in_window = message == WM_MOUSEMOVE;

            if (e->in_window) {
                e->screen_mouse_x = e->window_mouse_x = x;
                e->screen_mouse_y = e->window_mouse_y = y;

                wnd->client_to_screen(&e->screen_mouse_x, &e->screen_mouse_y);
            } else {
                e->screen_mouse_x = e->window_mouse_x = x;
                e->screen_mouse_y = e->window_mouse_y = y;

                wnd->screen_to_client(&e->window_mouse_x, &e->window_mouse_y);
            }

            e->mouse_source = get_mouse_source();


            return e;
        }
        case WM_LBUTTONDOWN: case WM_LBUTTONDBLCLK:
        case WM_RBUTTONDOWN: case WM_RBUTTONDBLCLK:
        case WM_MBUTTONDOWN: case WM_MBUTTONDBLCLK:
        case WM_XBUTTONDOWN: case WM_XBUTTONDBLCLK:
        case WM_LBUTTONUP:   case WM_RBUTTONUP:
        case WM_MBUTTONUP:   case WM_XBUTTONUP:
        {
            
            Mouse_Button_Event* e = __NEW_EVENT(Mouse_Button_Event);
            
            e->button = win32_message_to_input_code(message, wParam);

            e->double_click = message == WM_LBUTTONDBLCLK || message == WM_RBUTTONDBLCLK || message == WM_MBUTTONDBLCLK || message == WM_XBUTTONDBLCLK;

            e->mouse_source = get_mouse_source();

            if (e->double_click || message == WM_LBUTTONDOWN || message == WM_RBUTTONDOWN || message == WM_MBUTTONDOWN || message == WM_XBUTTONDOWN) {
                e->state = INPUT_STATE_DOWN;
            } else {
                e->state = INPUT_STATE_UP;
            }

            

            return e;
        }
        case WM_MOUSEWHEEL: case WM_MOUSEHWHEEL:
        {
            auto* e = __NEW_EVENT(Mouse_Scroll_Event);

            e->delta = message == WM_MOUSEWHEEL ? (float)GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA : (float)GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA;
            e->axis = message == WM_MOUSEWHEEL ? SCROLL_AXIS_VERTICAL : SCROLL_AXIS_HORIZONTAL;
            e->mouse_source = get_mouse_source();

            return e;
        }
        case WM_KEYDOWN: case WM_KEYUP: case WM_SYSKEYDOWN: case WM_SYSKEYUP:
        {
            auto* e = __NEW_EVENT(Key_Event);

            e->is_sys_key = message == WM_SYSKEYDOWN || message == WM_SYSKEYUP;

            e->key = to_st_input_code((u64)wParam);

            if (e->key == INPUT_CODE_ENTER && (HIWORD(lParam) & KF_EXTENDED)) {
                e->key = INPUT_CODE_NUMPAD_ENTER;
            }

            e->state = message == WM_KEYDOWN || message == WM_SYSKEYDOWN ? INPUT_STATE_DOWN : INPUT_STATE_UP;



            return e;
        }
        case WM_CHAR: 
        {
            auto* e = __NEW_EVENT(Char_Event);
            // TODO: #support #unicode #utf16 #utf8
            e->value = (char)wParam;
            return e;
        }
        case WM_SETCURSOR: 
        {
            auto* e = __NEW_EVENT(Setcursor_Event);
            e->in_client = LOWORD(lParam) == HTCLIENT;
            return e;
        }

        default: return 0;
    }
}

struct Internal {
    bool exit_flag = false;
    HDC hdc = 0;
};
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {

    win32::clear_error();
    Window* window = nullptr;
    if (message == WM_CREATE) {
        window = (Window*)((CREATESTRUCT*)lParam)->lpCreateParams;
        WIN32_CALL(SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)window));
    } else {
        window = (Window*)WIN32_CALL(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    }


    if (window) {
        if (((Internal*)window->__internal)->exit_flag) return TRUE;
        Window_Event_Type etype = to_event_type(message);
        if (etype != WINDOW_EVENT_TYPE_UNIMPLEMENTED) {
            
            void* pevent = NULL; 
            pevent = event_from_win32_message(window, message, wParam, lParam);
            if (!window->dispatch_event(etype, pevent)) return TRUE;

            switch (etype) {
                case WINDOW_EVENT_TYPE_MOUSE_BUTTON:
                {
                    auto& e = *(Mouse_Button_Event*)pevent;
                    window->input._input_states[e.button] = e.state;
                    break;
                }
                case WINDOW_EVENT_TYPE_KEY:
                {
                    auto& e = *(Key_Event*)pevent;
                    window->input._input_states[e.key] = e.state;
                    break;
                }
            }
        }
        if (message == WM_MOUSEMOVE) {
            s32 w, h;
            window->get_size(&w, &h);
            window->input._mouse_x = std::max((double)((s32)LOWORD(lParam)), 0.0); 
            window->input._mouse_y = std::min((double)((s32)HIWORD(lParam)), (double)h);
        }
        if (((Internal*)window->__internal)->exit_flag) {
            return TRUE;
        }
        if (message == WM_CLOSE) {
            ((Internal*)window->__internal)->exit_flag = true;
            return FALSE;
        }
        
    }
    
    return DefWindowProc(hWnd, message, wParam, lParam);
}

Window::Window(const Window_Init_Spec& wnd_spec, Window* parent) {

    memset(input._input_states, 0, sizeof(input._input_states));

    WNDCLASSEX wc;
    ZeroMemory(&wc, sizeof(wc));
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = WIN32_CALL(GetModuleHandle(nullptr));
    wc.lpszClassName = wnd_spec.title;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hIconSm = nullptr;
    wc.lpszMenuName = nullptr;
    wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND + 1);
    WIN32_CALL(RegisterClassEx(&wc));
    
    auto x = wnd_spec.x;
    auto y = wnd_spec.y;
    auto width = wnd_spec.width;
    auto height = wnd_spec.height;
    if (!wnd_spec.size_includes_styles) {
        RECT rect = { (LONG)wnd_spec.x, (LONG)wnd_spec.y, (LONG)(wnd_spec.x + wnd_spec.width), (LONG)(wnd_spec.y + wnd_spec.height) };
        AdjustWindowRectEx(&rect, to_win32_enum(wnd_spec.style), FALSE, to_win32_enum(wnd_spec.style_ex));

        x = rect.left;
        y = rect.top;
        width = rect.right - rect.left;
        height = rect.bottom - rect.top;
    }
    
    _current_style = wnd_spec.style;
    _current_style_ex = wnd_spec.style_ex;

    __internal = ST_NEW(Internal);

    _os_handle = WIN32_CALL(CreateWindowEx(
        to_win32_enum(wnd_spec.style_ex),
        wnd_spec.title,
        wnd_spec.title,
        to_win32_enum(wnd_spec.style),
        (int)x,
        (int)y,
        (int)width,
        (int)height,
        //parent ? (HWND)parent->_os_handle : nullptr,
        NULL,
        nullptr,
        GetModuleHandle(nullptr),
        nullptr
    ));

    _device_context = stnew (graphics::Device_Context)(this);

    _is_main = parent == NULL;
    _parent = parent;

    ST_ASSERT(_os_handle, "Window creation failed");

    

    WIN32_CALL(SetWindowLongPtr((HWND)_os_handle, GWLP_USERDATA, (LONG_PTR)this));

    WIN32_CALL(ShowWindow((HWND)_os_handle, wnd_spec.visible ? SW_SHOW : SW_HIDE));


    _device_context->bind();
    ((Internal*)__internal)->hdc = WIN32_CALL(GetDC((HWND)_os_handle));
    _device_context->unbind();

    log_info("Window '{}' was created", wnd_spec.title);
}

Window::~Window() {
    event_callbacks.clear();
    for (s64 i = _children.size() - 1; i >= 0; i--) {
        ST_DELETE(_children[i]);
        _children.erase(i);
    }
    if (_parent) {
        for (int i = 0; i < _parent->_children.size(); i++) {
            if (_parent->_children[i] == this) {
                _parent->_children.erase(i);
                break;
            }
        }
        
    }

    WIN32_CALL(ReleaseDC((HWND)_os_handle, ((Internal*)__internal)->hdc));
    WIN32_CALL(DestroyWindow((HWND)_os_handle));
    
}

void Window::add_event_callback(window_event_callback_t callback, void* userdata) {
    event_callbacks.push_back({ callback, userdata });
}

void Window::set_size(s32 width, s32 height) {
    RECT rect = { 0, 0, (LONG)width, (LONG)height };
    
    AdjustWindowRectEx(&rect, to_win32_enum(_current_style), FALSE, to_win32_enum(_current_style_ex));
    WIN32_CALL(SetWindowPos((HWND)_os_handle, nullptr, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE));
}
bool Window::get_size(s32* width, s32* height) const{
    tm_func();
    RECT rect;
    BOOL result = WIN32_CALL(GetClientRect((HWND)_os_handle, &rect));
    *width = rect.right - rect.left;
    *height = rect.bottom - rect.top;
    return result == TRUE;
}
void Window::set_position(s32 x, s32 y) {
    RECT rect = { (LONG)x, (LONG)y, (LONG)x, (LONG)y };
    ::AdjustWindowRectEx(&rect, to_win32_enum(_current_style), FALSE, to_win32_enum(_current_style_ex));
    WIN32_CALL(::SetWindowPos((HWND)_os_handle, nullptr, rect.left, rect.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE));
}
bool Window::get_position(s32* x, s32* y) const{
    POINT pos = { 0, 0 };
    auto result = WIN32_CALL(ClientToScreen((HWND)_os_handle, &pos));
    *x = pos.x;
    *y = pos.y;
    return result == TRUE;
}


void Window::set_fullscreen(bool fullscreen) {
    if (fullscreen) {
        WIN32_CALL(SetWindowLong((HWND)_os_handle, GWL_STYLE, GetWindowLong((HWND)_os_handle, GWL_STYLE) & ~WS_OVERLAPPEDWINDOW));
    } else {
        WIN32_CALL(SetWindowLong((HWND)_os_handle, GWL_STYLE, GetWindowLong((HWND)_os_handle, GWL_STYLE) | WS_OVERLAPPEDWINDOW));
    }
}

void Window::set_title(const char* title) {
    WIN32_CALL(SetWindowText((HWND)_os_handle, title));
}


void Window::set_focus(bool focused) {
    if (focused) {
        WIN32_CALL(BringWindowToTop((HWND)_os_handle));
        WIN32_CALL(SetForegroundWindow((HWND)_os_handle));
        WIN32_CALL(SetFocus((HWND)_os_handle));
    }
    else {
        // TODO: #unfinished
        // Not sure if unfocusing is actually a thing
    }
}

void Window::set_visibility(bool visible) {
    if (visible) {
        WIN32_CALL(ShowWindow((HWND)_os_handle, SW_SHOW));
    } else {
        WIN32_CALL(ShowWindow((HWND)_os_handle, SW_HIDE));
    }
}

void Window::set_alpha(float alpha) {
    assert(alpha >= 0.0f && alpha <= 1.0f);
    if (alpha < 1.0f)
    {
        DWORD style = WIN32_CALL(::GetWindowLongW((HWND)_os_handle, GWL_EXSTYLE) | WS_EX_LAYERED);
        WIN32_CALL(::SetWindowLongW((HWND)_os_handle, GWL_EXSTYLE, style));
        WIN32_CALL(::SetLayeredWindowAttributes((HWND)_os_handle, 0, (BYTE)(255 * alpha), LWA_ALPHA));
    }
    else
    {
        DWORD style = WIN32_CALL(::GetWindowLongW((HWND)_os_handle, GWL_EXSTYLE) & ~WS_EX_LAYERED);
        WIN32_CALL(::SetWindowLongW((HWND)_os_handle, GWL_EXSTYLE, style));
    }
}

void Window::set_parent(Window* new_parent) {
    if (new_parent == this->_parent) return;
    if (this->_parent) {
        for (int i = 0; i < _parent->_children.size(); i++) {
            if (_parent->_children[i] == this) {
                _parent->_children.erase(i);
                break;
            }
        }
    }
    if (!new_parent) {
        //WIN32_CALL(SetWindowLongPtr((HWND)this->_os_handle, GWLP_HWNDPARENT, 0));    
        return;
    }
    _parent = new_parent;
    new_parent->_children.push_back(this);

    //WIN32_CALL(SetWindowLongPtr((HWND)this->_os_handle, GWLP_HWNDPARENT, (LONG_PTR)new_parent->_os_handle));
}

Window* Window::add_child(const Window_Init_Spec& wnd_spec) {
    
    _children.push_back(stnew (Window)(wnd_spec, this));

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
bool Window::is_visible() const {
    return WIN32_CALL(IsWindowVisible((HWND)_os_handle));
}
bool Window::is_focused() const {
    return WIN32_CALL(GetForegroundWindow()) == (HWND)_os_handle;
}
bool Window::is_minimized() const {
    return WIN32_CALL(IsIconic((HWND)_os_handle) != 0);
}
bool Window::is_hovered() const {
    POINT pt;
    WIN32_CALL(GetCursorPos(&pt));
    HWND hwndHovered = WIN32_CALL(WindowFromPoint(pt));
    return hwndHovered == (HWND)_os_handle;
}
void* Window::get_monitor() const {
    return WIN32_CALL(MonitorFromWindow((HWND)_os_handle, MONITOR_DEFAULTTONEAREST));
}

graphics::Device_Context* Window::get_device_context() const {
    return _device_context;
}

bool Window::screen_to_client(s32* x, s32* y) const {
    POINT p {*x, *y};
    auto result = WIN32_CALL(ScreenToClient((HWND)_os_handle, &p));
    *x = p.x;
    *y = p.y;

    return result == TRUE;
}
bool Window::client_to_screen(s32* x, s32* y) const {
    POINT p {*x, *y};
    auto result = WIN32_CALL(ClientToScreen((HWND)_os_handle, &p));
    *x = p.x;
    *y = p.y;

    return result == TRUE;
}

bool Window::has_captured_mouse() const {
    return WIN32_CALL(::GetCapture()) == (HWND)_os_handle;
}
void Window::capture_mouse() {
    if (!has_captured_mouse()) {
        WIN32_CALL(::SetCapture((HWND)_os_handle));
        ST_ASSERT(has_captured_mouse());
    }
}
void Window::release_mouse() {
    if (has_captured_mouse()) {
        WIN32_CALL(::ReleaseCapture());
    }
}

bool Window::dispatch_event(Window_Event_Type etype, void* param) {
    for (auto& callback : event_callbacks) {
        if (!callback.fn(this, etype, param, callback.ud)) return false;
    }

    return true;
}

void Window::poll_events() {
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void Window::swap_buffers() {
    tm_func();
    //std::lock_guard context_lock(_device_context->_mutex.m);
    auto hdc = WIN32_CALL(GetDC((HWND)_os_handle));
    SwapBuffers(hdc);
    WIN32_CALL(ReleaseDC((HWND)_os_handle, hdc));
}

Input_State Window::Input::get_state(Input_Code code) const {
    return _input_states[code];
}


NS_END(os);
NS_END(stallout)
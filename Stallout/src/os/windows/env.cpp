#include "pch.h"

#include "os/env.h"

#include "Stallout/memory.h"
#include "Stallout/logger.h"

#undef UNICODE
#include <Windows.h>
#include "os/windows/winutils.h"

#include <assert.h>
NS_BEGIN(stallout)
NS_BEGIN(os);
NS_BEGIN(env);

LPCSTR to_win32_enum(Mouse_Cursor e) {
    switch (e) {
        case MOUSE_CURSOR_ARROW: return IDC_ARROW;
        case MOUSE_CURSOR_TEXTINPUT: return IDC_IBEAM;
        case MOUSE_CURSOR_RESIZEALL: return IDC_SIZEALL;
        case MOUSE_CURSOR_RESIZEEW: return IDC_SIZEWE;
        case MOUSE_CURSOR_RESIZENS: return IDC_SIZENS;
        case MOUSE_CURSOR_RESIZENESW: return IDC_SIZENESW;
        case MOUSE_CURSOR_RESIZENWSE: return IDC_SIZENWSE;
        case MOUSE_CURSOR_HAND: return IDC_HAND;
        case MOUSE_CURSOR_NOTALLOWED: return IDC_NO;
        case MOUSE_CURSOR_NONE: return 0;
        default: INTENTIONAL_CRASH("Unhandled enum"); return 0;
    }
}

Cache_Size get_cache_size() {
    DWORD bufferSize = 0;
    BOOL result = GetLogicalProcessorInformation(NULL, &bufferSize);
    DWORD lastError = GetLastError();
    (void)lastError;
    assert(result == TRUE || lastError == ERROR_INSUFFICIENT_BUFFER && "Failed to query buffer size for GetLogicalProcessorInformation");

    std::vector<SYSTEM_LOGICAL_PROCESSOR_INFORMATION> buffer(bufferSize / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION));
    result = WIN32_CALL(GetLogicalProcessorInformation(&buffer[0], &bufferSize));
    assert(result == TRUE && "Failed to get processor information");

    Cache_Size cs{};

    for (const auto& info : buffer) {
        if (info.Relationship == RelationCache) {
            switch (info.Cache.Level) {
                case 1:
                    cs.L1 = info.Cache.Size;
                    break;
                case 2:
                    cs.L2 = info.Cache.Size;
                    break;
                case 3:
                    cs.L3 = info.Cache.Size;
                    break;
                default:
                   
                    break;
            }
        }
    }

    return cs;
}


void get_monitors(Monitor*& monitors, size_t* num_monitors) {
    assert(num_monitors);
    struct Monitors_Param {
        Monitor*& monitors;
        size_t* num_monitors;
        size_t n;
    };
    Monitors_Param p { monitors, num_monitors, 0 };
    *p.num_monitors = 0;

    WIN32_CALL(::EnumDisplayMonitors(nullptr, nullptr, [](HMONITOR, HDC, LPRECT, LPARAM param) {(*(((Monitors_Param*)param)->num_monitors))++; return TRUE; }, (LPARAM)&p));

    monitors = (Monitor*)ST_MEM(sizeof(Monitor) * *num_monitors);

    WIN32_CALL(::EnumDisplayMonitors(nullptr, nullptr, [](HMONITOR monitor, HDC, LPRECT, LPARAM p) {
        auto param = (Monitors_Param*)p;
        MONITORINFO info = {};
        info.cbSize = sizeof(MONITORINFO);
        if (!::GetMonitorInfo(monitor, &info))
            return TRUE;

        Monitor& m = param->monitors[param->n++];
        m.main_left   = info.rcMonitor.left;
        m.main_top    = info.rcMonitor.top;
        m.main_right  = info.rcMonitor.right;
        m.main_bottom = info.rcMonitor.bottom;

        m.work_left = info.rcWork.left;
        m.work_top = info.rcWork.top;
        m.work_right = info.rcWork.right;
        m.work_bottom = info.rcWork.bottom;

        m.is_primary = m.main_left == 0 && m.main_top == 0;

        m.handle = monitor;

        return TRUE;
    }, (LPARAM)&p));
}

typedef enum { PROCESS_DPI_UNAWARE = 0, PROCESS_SYSTEM_DPI_AWARE = 1, PROCESS_PER_MONITOR_DPI_AWARE = 2 } PROCESS_DPI_AWARENESS;
typedef enum { MDT_EFFECTIVE_DPI = 0, MDT_ANGULAR_DPI = 1, MDT_RAW_DPI = 2, MDT_DEFAULT = MDT_EFFECTIVE_DPI } MONITOR_DPI_TYPE;

typedef HRESULT(WINAPI* PFN_GetDpiForMonitor)(HMONITOR, MONITOR_DPI_TYPE, UINT*, UINT*); 
float get_monitor_dpi(void* monitor) {
    UINT xdpi = 96, ydpi = 96;
    static HINSTANCE shcore_dll = WIN32_CALL(::LoadLibrary(TEXT("shcore.dll")));
    static PFN_GetDpiForMonitor GetDpiForMonitorFn = nullptr;
    
    if (GetDpiForMonitorFn == nullptr && shcore_dll != nullptr) {
        GetDpiForMonitorFn = (PFN_GetDpiForMonitor)WIN32_CALL(::GetProcAddress(shcore_dll, "GetDpiForMonitor"));
    }
    
    if (GetDpiForMonitorFn != nullptr) {
        HRESULT hr = GetDpiForMonitorFn((HMONITOR)monitor, MDT_EFFECTIVE_DPI, &xdpi, &ydpi);
        if (SUCCEEDED(hr)) {
            assert(xdpi == ydpi);
            return xdpi / 96.0f;
        }
        win32::clear_error();
    }
    
#ifndef NOGDI
    const HDC dc = WIN32_CALL(::GetDC(nullptr));
    if (dc != nullptr) {
        xdpi = WIN32_CALL(::GetDeviceCaps(dc, LOGPIXELSX));
        ydpi = WIN32_CALL(::GetDeviceCaps(dc, LOGPIXELSY));
        WIN32_CALL(::ReleaseDC(nullptr, dc));
        assert(xdpi == ydpi); 
        return xdpi / 96.0f;
    }
#endif

    // return default scale factor if all else fails
    return 1.0f;
}

void set_mouse_cursor(Mouse_Cursor cursor) {
    SetCursor(LoadCursor(NULL, to_win32_enum(cursor)));
}

bool is_app_focused() {
    HWND hWnd = WIN32_CALL(GetForegroundWindow());
    if (!hWnd) {
        return false; 
    }

    DWORD focusedWindowProcessId;
    WIN32_CALL(GetWindowThreadProcessId(hWnd, &focusedWindowProcessId));

    DWORD currentProcessId = WIN32_CALL(GetCurrentProcessId());

    return focusedWindowProcessId == currentProcessId;
}

void set_cursor_pos(s32 x, s32 y) {
    POINT pos = { x, y };
    WIN32_CALL(SetCursorPos(pos.x, pos.y));
}

NS_END(env);
NS_END(os);
NS_END(stallout)
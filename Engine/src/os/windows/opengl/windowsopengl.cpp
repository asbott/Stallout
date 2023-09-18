#pragma once

#include "pch.h"

#include "os/graphics.h"

#include <Windows.h>

NS_BEGIN(os);
NS_BEGIN(graphics);
OS_Graphics_Context::OS_Graphics_Context(Window* wnd) {

    HDC hDC = GetDC((HWND)wnd->_os_handle);
    if (!hDC) {
        // Handle error
        return;
    }
    
    _os_window_handle = wnd->_os_handle;

    PIXELFORMATDESCRIPTOR pfd = {0};
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;

    int pixelFormat = ChoosePixelFormat(hDC, &pfd);
    ST_ASSERT(pixelFormat);

    ST_ASSERT(SetPixelFormat(hDC, pixelFormat, &pfd));

    _os_context_handle = wglCreateContext(hDC);
    ST_ASSERT(_os_context_handle);

    ReleaseDC((HWND)_os_window_handle, hDC);

    log_info("wgl context '{}' was created", (u64)_os_context_handle);
}

OS_Graphics_Context::~OS_Graphics_Context() {
    wglDeleteContext((HGLRC)_os_context_handle);
}

void OS_Graphics_Context::make_current() {
    HDC hDC = GetDC((HWND)_os_window_handle);
    ST_ASSERT(hDC);
    auto result = wglMakeCurrent(hDC, (HGLRC)_os_context_handle);
    ST_ASSERT(result);
    ReleaseDC((HWND)_os_window_handle, hDC);
}


void* get_current_context_handle() {
    return wglGetCurrentContext();
}

NS_END(graphics);
NS_END(os);
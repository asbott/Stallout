#pragma once

#include "pch.h"

#include "os/graphics.h"

#include "Engine/containers.h"

#include <Windows.h>



NS_BEGIN(os);
NS_BEGIN(graphics);

engine::Hash_Map<std::thread::id, OS_Graphics_Context*> current;
std::mutex current_mutex;

OS_Graphics_Context::OS_Graphics_Context(Window* wnd) {
    std::lock_guard lock(_mutex);

    auto hwnd = (HWND)wnd->_os_handle;
    HDC hDC = GetDC(hwnd);

    ST_ASSERT(hDC);
    
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

    log_info("wgl context '{}' was created", (u64)_os_context_handle);

    ReleaseDC(hwnd, hDC);
}

OS_Graphics_Context::~OS_Graphics_Context() {
    std::lock_guard lock(_mutex);
    wglDeleteContext((HGLRC)_os_context_handle);
}

__Must_Unbind OS_Graphics_Context::bind() {
    
    HDC hDC = GetDC((HWND)_os_window_handle);
    ST_DEBUG_ASSERT(hDC);
    auto unbind_promise = __Must_Unbind(this, &_mutex); // wait mutex lock
    auto result = wglMakeCurrent(hDC, (HGLRC)_os_context_handle);
    if (!result) {
        DWORD err = GetLastError();
        if (err != ERROR_SUCCESS) {
            char* msg;
            FormatMessageA(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                NULL, err, 1033 /*English*/, 
                (LPSTR)&msg, 0, NULL
            );

            log_critical(msg);

            LocalFree(msg);
        }
    }
    ST_ASSERT(result, "Failed making wgl context {} current", (u64)_os_context_handle);

    {
        std::lock_guard lock(current_mutex);
        current[std::this_thread::get_id()] = this;
    }
    

    ReleaseDC((HWND)_os_window_handle, hDC);
    return std::move(unbind_promise);
}

void OS_Graphics_Context::unbind() {
    std::lock_guard lock(current_mutex);
    if (current[std::this_thread::get_id()] == this) {
        wglMakeCurrent(0, 0);
        current[std::this_thread::get_id()] = NULL;
    }

    return;
}

void OS_Graphics_Context::use(std::function<void(OS_Graphics_Context*)> fn) {
    auto _ = this->bind();
    fn(this);
    this->unbind();
}
void OS_Graphics_Context::use(std::function<void()> fn) {
    auto _ = this->bind();
    fn();
    this->unbind();
}

void share_resources(OS_Graphics_Context* a, OS_Graphics_Context* b) {
    ST_DEBUG_ASSERT(a && b);
    ST_ASSERT(wglShareLists((HGLRC)a->_os_context_handle, (HGLRC)b->_os_context_handle), "Resource sharing failed");
    log_info("Enabled resource sharing between wgl contexts {} and {}", (u64)a->_os_context_handle, (u64)b->_os_context_handle);
}

OS_Graphics_Context* get_current_context() {
    return current[std::this_thread::get_id()];
}

NS_END(graphics);
NS_END(os);
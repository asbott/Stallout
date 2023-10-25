#include "pch.h"

#include "os/graphics.h"
#include "os/oswindow.h"

#include "Engine/containers.h"
#include "Engine/timing.h"
#include "Engine/logger.h"

#include <Windows.h>
#include "os/windows/winutils.h"

typedef BOOL (WINAPI * vsyncfunc)(int);

NS_BEGIN(os);
NS_BEGIN(graphics);

thread_local Device_Context* current = NULL;


Device_Context::Device_Context(os::Window* target) {
    _os_target_handle = target->_os_handle;

    std::lock_guard lock(_mutex.m);

    auto hdc = WIN32_CALL(GetDC((HWND)_os_target_handle));

    PIXELFORMATDESCRIPTOR pfd = {0};
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;

    int pixel_format = WIN32_CALL(ChoosePixelFormat(hdc, &pfd));

    WIN32_CALL(SetPixelFormat(hdc, pixel_format, &pfd));

    _os_context_handle = WIN32_CALL(wglCreateContext(hdc));

    WIN32_CALL(ReleaseDC((HWND)_os_target_handle, hdc));

    log_info("wgl context '{}' was created", (u64)_os_context_handle);
}
Device_Context::~Device_Context() {
    std::lock_guard lock(_mutex.m);
    WIN32_CALL(wglDeleteContext((HGLRC)_os_context_handle));
}

void Device_Context::bind() {
    if (current == this) return;
    std::lock_guard lock(_mutex.m);
    
    auto hdc = WIN32_CALL(GetDC((HWND)_os_target_handle));

    num_binds++;
    WIN32_CALL(wglMakeCurrent(hdc, (HGLRC)_os_context_handle));

    if (!_os_vsync_func) {
        _os_vsync_func = (vsyncfunc) WIN32_CALL(wglGetProcAddress("wglSwapIntervalEXT"));
        ST_ASSERT(_os_vsync_func);
        set_vsync(false);
    }

    current = this;

    WIN32_CALL(ReleaseDC((HWND)_os_target_handle, hdc));
}
void Device_Context::unbind() {
    if (!current) return;
    std::lock_guard lock(_mutex.m);
    if (current != this) return;

    current = NULL;

    num_unbinds++;
    WIN32_CALL(wglMakeCurrent(0, 0));
}
void Device_Context::set_vsync(bool vsync) {
    if (_os_vsync_func) {
        WIN32_CALL(((vsyncfunc)_os_vsync_func)(vsync));
    }
}

void Device_Context::share(Device_Context* other) {
    ST_DEBUG_ASSERT(other);
    WIN32_CALL(wglShareLists((HGLRC)_os_context_handle, (HGLRC)other->_os_context_handle));
    log_info("Enabled resource sharing between wgl contexts {} and {}", (u64)_os_context_handle, (u64)other->_os_context_handle);
}

Device_Context* get_current_context() {
    return current;
}

NS_END(graphics);
NS_END(os);
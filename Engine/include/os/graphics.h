#pragma once

#include "os/oswindow.h"

NS_BEGIN(os);
NS_BEGIN(graphics);

struct OS_Graphics_Context {
    OS_Graphics_Context(Window* wnd);
    ~OS_Graphics_Context();
    void make_current();
    void* _os_window_handle;
    void* _os_context_handle;
};

void* get_current_context_handle();

NS_END(graphics);
NS_END(os);
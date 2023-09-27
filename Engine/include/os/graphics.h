#pragma once

#include "os/oswindow.h"

NS_BEGIN(os);
NS_BEGIN(graphics);
struct OS_Graphics_Context;
ST_API OS_Graphics_Context* get_current_context();

struct __Must_Unbind {
    OS_Graphics_Context* h;
    std::mutex* _m;
    __Must_Unbind(OS_Graphics_Context* h, std::mutex* _m) : h(h), _m(_m) {
        ST_ASSERT(get_current_context() != h, "Graphics context Already bound");
        
        _m->lock();
        
    }
    __Must_Unbind(__Must_Unbind && o) noexcept {
        this->h = o.h;
        this->_m = o._m;

        o.h = NULL;
        o._m = NULL;
    }
    ~__Must_Unbind() {
        ST_ASSERT(!h || get_current_context() != h, "OS_Graphics_Context was left bound after scope");
        if (_m) _m->unlock();
    }
};

struct OS_Graphics_Context {
    OS_Graphics_Context(Window* wnd);
    ~OS_Graphics_Context();
    [[nodiscard]] __Must_Unbind bind();
    void unbind();
    void use(std::function<void(OS_Graphics_Context*)> fn);
    void use(std::function<void()> fn);

    void* _os_window_handle;
    void* _os_context_handle;
    std::mutex _mutex;
};

ST_API void share_resources(OS_Graphics_Context* a, OS_Graphics_Context* b);



NS_END(graphics);
NS_END(os);
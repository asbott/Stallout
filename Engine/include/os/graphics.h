#pragma once

NS_BEGIN(os);

struct Window;

NS_BEGIN(graphics);

struct _Mutex_Wrapper {
    std::mutex m;
};
struct ST_API Device_Context {
    Device_Context(os::Window* target);
    ~Device_Context();

    void bind();
    void unbind();
    void set_vsync(bool vsync);
    void share(Device_Context* other);

    void* _os_target_handle;
    void* _os_context_handle;
    void* _os_vsync_func = NULL;
    _Mutex_Wrapper _mutex;

    size_t num_binds = 0, num_unbinds = 0;
};

ST_API Device_Context* get_current_context();

NS_END(graphics);
NS_END(os);
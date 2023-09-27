

#include "pch.h"

#include "renderer/rendercontext.h"
#include "Engine/logger.h"
#include "Engine/stringutils.h"

NS_BEGIN(engine);
NS_BEGIN(renderer);

Render_Window::Render_Window(Render_Context* context, utils::Double_Buffered_Thread* render_thread, os::Window* backend) 
    : _context(context), _render_thread(render_thread), _backend(backend), _parent(NULL) {
    _init_backend(backend);
}
Render_Window::Render_Window(Render_Window* parent, os::Window* backend) 
    : _context(parent->_context), _render_thread(parent->_render_thread), _backend(backend), _parent(parent) {
    _init_backend(backend);
}
Render_Window::~Render_Window() {
    ST_ASSERT(_context);
    _dead = true;
    _backend->set_event_callback(0);

    // We might have sened render commands which need
    // the window so we cant free it until those have
    // been processed
    _send([this](os::Window* wnd) {
        if (_parent) {            
            ST_DELETE(wnd);
        }
    });

    this->set_parent(NULL);
}

void Render_Window::add_event_callback(render_window_callback_t callback, void* userdata) {
    _event_callbacks.push_back(Window_Event_Callback {callback, userdata});
}
void Render_Window::add_event_callback(os::Window_Event_Type event, render_window_callback_t callback, void* userdata) {
    _specific_event_callbacks[event].push_back(Window_Event_Callback {callback, userdata});
}

void Render_Window::dispatch_event(u32 type, void* param) {
    for (auto callback : _event_callbacks) {
        auto response = callback.fn(this, type, param, callback.user_data);       
        if (!response) break;
    }
}

void Render_Window::set_parent(Render_Window* new_parent) {
    if (new_parent == this->_parent) return;
    if (this->_parent) {
        for (int i = 0; i < _parent->_children.size(); i++) {
            if (_parent->_children[i] == this) {
                _parent->_children.erase(i);
                break;
            }
        }
    }
    _parent = new_parent;

    _os_context->use([&]() {
        if (new_parent) {
            new_parent->_children.push_back(this);
            _backend->set_parent((os::Window*)new_parent->_backend);
        } else {
            _backend->set_parent(NULL);
        }
    });
    
}
Render_Window* Render_Window::add_child(os::Window_Init_Spec spec) {
    Render_Window* child = NULL; 

    // Win32 window needs to be created and modified
    // on the same thread at all times
    _context->do_now([&]() {
        child = stnew (Render_Window)(this, _backend->add_child(spec));
    });
    _children.push_back(child);
    child->_parent = this;
    
    return child;
}




void Render_Window::set_size(const mz::s32vec2& sz) {
    _send([sz, this](os::Window* wnd){
        ST_ASSERT(_context->get_window()->_backend != wnd);
        wnd->set_size(sz.x, sz.y);
    });
}


void Render_Window::capture_mouse() {
    _send([](os::Window* wnd) {
        wnd->capture_mouse();
    });
}
void Render_Window::release_mouse() {
    _send([](os::Window* wnd) {
        wnd->release_mouse();
    });
}


void Render_Window::set_visibility(bool visible) {
    _send([visible](os::Window* wnd) {
        wnd->set_visibility(visible);
    });
}

void Render_Window::set_focus(bool focused) {
    _send([focused](os::Window* wnd) {
        wnd->set_focus(focused);
    });
}
void Render_Window::set_title(const char* title) {
    New_String str(title);
    _send([str](os::Window* wnd) {
        wnd->set_title(str.str);
    });
}

void Render_Window::set_alpha(float alpha) {
    _send([alpha](os::Window* wnd) {
        wnd->set_alpha(alpha);
    });
}
bool Render_Window::exit_flag() const {
    bool ret;
    _os_context->use([&]() {
        ret = _backend->exit_flag();
    });
    return ret;
}

void Render_Window::poll_events() {
    _backend->poll_events();
}
void Render_Window::swap_buffers() {
    _send([](os::Window* wnd) {
        wnd->swap_buffers();
    });
}

void Render_Window::set_position(const mz::s32vec2& pos) {
    _send([pos](os::Window* wnd){
        wnd->set_position(pos.x, pos.y);
    });
}
mz::s32vec2 Render_Window::get_position() {
    s32 x = 0, y = 0;
    _os_context->use([&]() {
        _backend->get_position(&x, &y);
    });
    return {x, y};
}
mz::s32vec2 Render_Window::get_size() {
    s32 w = 0, h = 0;
    _os_context->use([&]() {
        _backend->get_size(&w, &h);
    });
    return {w, h};
}

mz::s32vec2 Render_Window::screen_to_client(mz::s32vec2 screen) const {
    _os_context->use([&]() {
        _backend->screen_to_client(&screen.x, &screen.y);
    });
    return screen;
}
mz::s32vec2 Render_Window::client_to_screen(mz::s32vec2 client) const {
    _os_context->use([&]() {
        _backend->client_to_screen(&client.x, &client.y);
    });

    return client;
}
bool Render_Window::is_visible() const {
    bool ret = false;
    _os_context->use([&]() {
        ret = _backend->is_visible();
    });
    return ret;
}
bool Render_Window::is_focused() const {
    bool ret = false;
    _os_context->use([&]() {
        ret = _backend->is_focused();
    });
    return ret;
}
bool Render_Window::is_minimized() const {
    bool ret = false;
    _os_context->use([&]() {
        ret = _backend->is_minimized();
    });
    return ret;
}
bool Render_Window::is_hovered() const {
    bool ret = false;
    _os_context->use([&]() {
        ret = _backend->is_hovered();
    });
    return ret;
}
void* Render_Window::get_monitor() const {
    void* ret = 0;
    _os_context->use([&]() {
        ret = _backend->get_monitor();
    });
    return ret;
}
bool Render_Window::is_input_down(os::Input_Code code) const {
    bool ret = false;
    _os_context->use([&]() {
        ret = _backend->input.get_state(code) == os::INPUT_STATE_DOWN;
    });
    return ret;
}

bool Render_Window::has_captured_mouse() const {
    bool ret = false;
    _os_context->use([&]() {
        ret = _backend->has_captured_mouse();
    });
    return ret;
}

void Render_Window::_init_backend(os::Window* backend) {
    backend->set_event_callback([this](os::Window* , os::Window_Event_Type etype , void* param, void*) -> void{
        ST_ASSERT(!_dead, "Callback on dead Render_Window");
        dispatch_event(etype, param);
    });
    _backend = backend;
    _os_context = stnew (os::graphics::OS_Graphics_Context)(_backend);
    if (this->_parent) {
        os::graphics::share_resources(_parent->_os_context, _os_context);
    }
}

void Render_Window::_send(std::function<void(os::Window*)> fn) {
    Render_Command command;
    command.type = RENDER_COMMAND_TYPE_WINDOW_CALL;
    command.size = sizeof(Render_Command) + sizeof(Window_Call);
    Window_Call wc;
    // TODO: #performance #memory
    // Use stack allocator which resets each frame
    wc.fn = stnew (std::function<void(os::Window*)>)(fn); 
    wc.caller = this;
    
    _render_thread->send(&command, &wc, sizeof(Window_Call));
}

void Render_Window::__query_backend(os::Window* backend) {
    (void)backend;
    // Also query backends in children
    for (auto child : _children) {
        child->__query_backend((os::Window*)child->_backend);
    }
}

Render_Context::Render_Context(size_t buffer_size)
    : _id_allocator(3000000, sizeof(Resource_ID)),
      _mapping_promise_allocator(sizeof(_Mapping_Promise) * 1000, sizeof(_Mapping_Promise)),
      _render_thread(buffer_size, [&](std::mutex* mtx) {__enter_loop(mtx);}) {
    _render_thread.set_command_type<Render_Command>();
}
Render_Context::~Render_Context() {
    _render_thread.stop();
}

void Render_Context::wait_ready() {
    std::mutex ready_mtx;
    std::unique_lock lock(ready_mtx);

    _ready_cond.wait(lock, [&]() { return _ready; });
}


Resource_Handle Render_Context::create(Resource_Type resource_type, const void* data, size_t data_size) {
    size_t command_size = sizeof(Render_Command) + data_size;

    ST_ASSERT(data);

    auto handle = (Resource_Handle)_id_allocator.allocate(sizeof(Resource_Handle));

    Render_Command command_header;
    command_header.type = RENDER_COMMAND_TYPE_CREATE;
    command_header.resource_type = resource_type;
    command_header.handle = handle;
    command_header.size = command_size;       

    _render_thread.send(&command_header, data, data_size);

    return handle;
}

void Render_Context::submit(Render_Message message, const void* data, size_t data_size) {
    size_t command_size = sizeof(Render_Command) + data_size;
    
    ST_ASSERT(data);

    Render_Command command_header;
    command_header.type = RENDER_COMMAND_TYPE_SUBMIT;
    command_header.message = message;
    command_header.size = command_size;
    _render_thread.send(&command_header, data, data_size);

}
void Render_Context::set(Resource_Handle hnd, Resource_Type resource_type, const void* data, size_t data_size) {
    size_t command_size = sizeof(Render_Command) + data_size;

    ST_ASSERT(this->get_resource_state(hnd) != RESOURCE_STATE_ERROR && this->get_resource_state(hnd) != RESOURCE_STATE_DEAD, "Invalid handle");
    
    ST_DEBUG_ASSERT(data_size);

    Render_Command command_header;
    command_header.type = RENDER_COMMAND_TYPE_SET;
    command_header.resource_type = resource_type;
    command_header.handle = hnd;
    command_header.size = command_size;       

    _render_thread.send(&command_header, data, data_size);
}

void Render_Context::destroy(Resource_Handle hnd) {
    Render_Command command_header;
    command_header.type = RENDER_COMMAND_TYPE_SUBMIT;
    command_header.message = RENDER_MESSAGE_DESTROY;
    command_header.size = sizeof(Render_Command);
    command_header.handle = hnd;
    _render_thread.send(&command_header, NULL, 0);
}

void Render_Context::map_buffer(Resource_Handle buffer_hnd, Buffer_Access_Mode access_Mode, const std::function<void(void*)>& result_callback) {
    
    // Cant check resource meta here because it might
    // not have been initialized yet since this is 
    // the caller thread.
    
    // Delete when processed
    _Mapping_Promise* promise = _mapping_promise_allocator.allocate_and_construct<_Mapping_Promise>();
    ST_ASSERT(promise, "Too many map commands in one cycle");

    promise->callback = result_callback;

    _mapping_promises[buffer_hnd] = promise;

    _Map_Command cm;
    cm.buffer_hnd = buffer_hnd;
    cm.promise = promise;
    cm.access = access_Mode;

    this->submit(__INTERNAL_RENDER_MESSAGE_MAP_BUFFER, &cm, sizeof(cm));
}
void Render_Context::unmap_buffer(Resource_Handle buffer_hnd) {
    ST_ASSERT(this->get_resource_state(buffer_hnd) != RESOURCE_STATE_ERROR && this->get_resource_state(buffer_hnd) != RESOURCE_STATE_DEAD);
    ST_ASSERT(this->get_resource_meta(buffer_hnd).type == RESOURCE_TYPE_BUFFER);

    _Unmap_Command cm;
    cm.buffer_hnd = buffer_hnd;
    this->submit(__INTERNAL_RENDER_MESSAGE_UNMAP_BUFFER, &cm, sizeof(cm));
}

Render_Context::Environment Render_Context::get_environment() const {
    std::lock_guard lock(_env_mutex);
    return _env;
}

void Render_Context::swap_command_buffers() {
    _render_thread.swap();

    for (const auto& [hnd, promise] : _mapping_promises) {
        void* result = promise->wait();
        promise->callback(result);

        _mapping_promise_allocator.deallocate_and_deconstruct(promise);
    }

    _mapping_promises.clear();
}

void Render_Context::set_target(Render_Window* target) {
    spec::submit::Set_Target spec;
    spec.target = target;
    this->submit(spec);
}
Render_Window* Render_Context::get_current_target() const {
    return _current_target;
}

void Render_Context::__enter_loop(std::mutex* buffer_mutex) {
    Timer frame_timer;

    os::Window_Init_Spec wnd_spec;
    __backend_window = stnew (os::Window)(wnd_spec);
    _frontend_window = stnew (Render_Window)(this, &_render_thread, __backend_window);
    _current_target = _frontend_window;

    _current_target->_os_context->use([&]() {
        this->__internal_init();
    });

    _ready = true;
    _ready_cond.notify_all();

    while (!_render_thread.should_exit()) {
       
        std::unique_lock lock(*buffer_mutex);
        _frame_time = frame_timer.record();
        frame_timer.reset();

        // TODO: (2023-09-05) #unfinished #hacky
        _render_thread._read_condition.wait(lock, [&]() { 
            return _render_thread._buffer_state == utils::COMMAND_BUFFER_STATE_NEW || (!_running) || !_priority_queue.empty(); 
        });
        
        while (!_priority_queue.empty()) {
            Priority_Task task;
            {
                std::lock_guard plock(_priority_mutex);
                task = _priority_queue.front();
                _priority_queue.pop();
            }
            _current_target->_os_context->use([&]() {
                task.task();
            });
            
            task.promise->done = true;
            task.promise->cond.notify_all();
        }

        if (_render_thread._buffer_state == utils::COMMAND_BUFFER_STATE_NEW) {
            _render_thread.traverse_commands<Render_Command>([&](Render_Command* header, void* data) {            
                while (!_priority_queue.empty()) {
                    Priority_Task task;
                    {
                        std::lock_guard lock(_priority_mutex);
                        task = _priority_queue.front();
                        _priority_queue.pop();
                    }
                    _current_target->_os_context->use([&]() {
                        task.task();
                    });
                    task.promise->done = true;
                    task.promise->cond.notify_all();
                    
                }
                // Window calls not handled in api implementations, handled here
                if (header->type == RENDER_COMMAND_TYPE_WINDOW_CALL) {
                    auto wc = (Window_Call*)data;
                    
                    wc->caller->_os_context->use([&]() {
                        (*wc->fn)(wc->caller->_backend);
                    });
                    ST_DELETE(wc->fn);
                } else if (header->type == RENDER_COMMAND_TYPE_SUBMIT && header->message == RENDER_MESSAGE_SET_TARGET) {
                    auto& spec = *(spec::submit::Set_Target*)data;
                    _current_target = spec.target;
                } else {
                    _current_target->_os_context->use([&]() {
                        this->__internal_handle_command(header, data);
                    });
                }
                return header->size;
            });
        }
        _frontend_window->_os_context->use([&]() {
            _frontend_window->__query_backend(__backend_window);
        });

        _render_thread.notify_buffer_finished();
    }   
    _current_target->_os_context->use([&]() {
        this->__internal_shutdown();
    });
    log_info("Render thread shutdown");

    /*_frontend_window->_os_context->use([&]() {
        ST_DELETE(__backend_window);
    });*/
    

    log_info("Render thread shutdown");
}

NS_END(engine);
NS_END(renderer);
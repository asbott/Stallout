#include "pch.h"

#include "renderer/rendercontext.h"
#include "Engine/logger.h"

NS_BEGIN(engine);
NS_BEGIN(renderer);

namespace mem = engine::Global_Allocator;


byte_t* allocate_buffer(size_t buffer_size) {
    return (byte_t*)mem::allocate(buffer_size, GLOBAL_ALLOC_FLAG_LARGE & GLOBAL_ALLOC_FLAG_STATIC);
}

Render_Context::Render_Context(size_t buffer_size)
    : _buffer_size(buffer_size), 
      _buffer_allocator(allocate_buffer(buffer_size), buffer_size),
      _main_buffer(allocate_buffer(buffer_size)),
      _id_allocator(3000000, sizeof(Resource_ID)),
      _mapping_promise_allocator(sizeof(_Mapping_Promise) * 1000, sizeof(_Mapping_Promise)) {

        _render_thread = std::thread([&]() {__enter_loop();});
}
Render_Context::~Render_Context() {
    this->swap_buffers();
    _running = false;
    _render_condition.notify_all();
    _render_thread.join();
}

void Render_Context::wait_ready() {
    std::mutex ready_mtx;
    std::unique_lock lock(ready_mtx);

    _ready_cond.wait(lock, [&]() { return _ready; });
}

void Render_Context::_send_command(Render_Command command, const void* data, size_t data_size) {
    ST_ASSERT(_buffer_allocator._next + command.size <= _buffer_allocator._tail, "Command buffer overflow. Please allocate more.\nFree: {}kb/{}kb\nRequested: {}kb", (_buffer_size - (_buffer_allocator._tail - _buffer_allocator._next)) * 1000.0, _buffer_size * 1000.0, command.size * 1000.0);
    std::lock_guard lock(_buffer_mutex);
    byte_t* command_buffer = (byte_t*)_buffer_allocator.allocate(command.size);
    memcpy(command_buffer, &command, sizeof(Render_Command));

    // Should be an option to pass uninitialized or no data
    if(data && data_size) memcpy(command_buffer + sizeof(Render_Command), data, data_size);
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

    _send_command(command_header, data, data_size);

    return handle;
}

void Render_Context::submit(Render_Message message, const void* data, size_t data_size) {
    size_t command_size = sizeof(Render_Command) + data_size;
    
    ST_ASSERT(data);

    Render_Command command_header;
    command_header.type = RENDER_COMMAND_TYPE_SUBMIT;
    command_header.message = message;
    command_header.size = command_size;
    _send_command(command_header, data, data_size);
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

    _send_command(command_header, data, data_size);
}

void Render_Context::destroy(Resource_Handle hnd) {
    Render_Command command_header;
    command_header.type = RENDER_COMMAND_TYPE_SUBMIT;
    command_header.message = RENDER_MESSAGE_DESTROY;
    command_header.size = sizeof(Render_Command);
    command_header.handle = hnd;
    _send_command(command_header, NULL, 0);
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

void Render_Context::swap_buffers() {
    {
        std::unique_lock lock(_buffer_mutex);

        _swap_condition.wait(lock, [&]() { return _buffer_state == COMMAND_BUFFER_STATE_OLD; });

        _buffer_usage = (size_t)(_buffer_allocator._next - _buffer_allocator._head);;

        _main_buffer = (byte_t*)_buffer_allocator.swap_buffer(_main_buffer, _buffer_size);

        _buffer_state = COMMAND_BUFFER_STATE_NEW;
        _render_condition.notify_one();
    }

    for (const auto& [hnd, promise] : _mapping_promises) {
        void* result = promise->wait();
        promise->callback(result);

        _mapping_promise_allocator.deallocate_and_deconstruct(promise);
    }

    _mapping_promises.clear();
}

void Render_Context::__enter_loop() {
    Timer frame_timer;

    os::Window_Init_Spec wnd_spec;
    _os_window = ST_NEW(os::Window, wnd_spec);
    _os_context = ST_NEW(os::graphics::OS_Graphics_Context, _os_window);
    
    this->__internal_init();

    while (_running || _buffer_state == COMMAND_BUFFER_STATE_NEW) {
       
        std::unique_lock lock(_buffer_mutex);
        _frame_time = frame_timer.record();
        frame_timer.reset();
        _render_condition.wait(lock, [&]() { 
            return _buffer_state == COMMAND_BUFFER_STATE_NEW || (!_running); 
        });

        if (_buffer_state == COMMAND_BUFFER_STATE_OLD) break;

        this->__internal_render();

        _os_window->swap_buffers();

        _buffer_state = COMMAND_BUFFER_STATE_OLD;
        _swap_condition.notify_one();
        
    }   

    if (_buffer_state == COMMAND_BUFFER_STATE_NEW) {
        this->__internal_render();
    }

    this->__internal_shutdown();

    ST_DELETE(_os_context);
    ST_DELETE(_os_window);

    log_info("Render thread shutdown");
}

NS_END(engine);
NS_END(renderer);
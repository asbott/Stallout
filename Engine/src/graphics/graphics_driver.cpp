

#include "pch.h"

#include <condition_variable>

#include "graphics/graphics_driver.h"
#include "Engine/logger.h"
#include "Engine/stringutils.h"

#include "os/graphics.h"

NS_BEGIN(engine);
NS_BEGIN(graphics);

Graphics_Driver::Graphics_Driver(Device_Context* target, size_t buffer_size)
    : _id_allocator(3000000, sizeof(Resource_ID)),
      _mapping_promise_allocator(sizeof(_Mapping_Promise) * 1000, sizeof(_Mapping_Promise)),
      _target(target) {
    _render_thread = stnew (utils::Double_Buffered_Thread)(buffer_size, [&](std::mutex* mtx) {__enter_loop(mtx);});
    _render_thread->set_command_type<Render_Command>();

    this->wait_ready();
}
Graphics_Driver::~Graphics_Driver() {
    _render_thread->stop();
}

void Graphics_Driver::wait_ready() {
    std::mutex ready_mtx;
    std::unique_lock lock(ready_mtx);

    _ready_cond.wait(lock, [&]() { return _ready; });
}


Resource_Handle Graphics_Driver::create(Resource_Type resource_type, const void* data, size_t data_size) {
    tm_func();
    ST_ASSERT(_target, "Using Graphics_Driver without set target; call set_target()");

    size_t command_size = sizeof(Render_Command) + data_size;

    ST_ASSERT(data);

    auto handle = (Resource_Handle)_id_allocator.allocate(sizeof(Resource_Handle));

    Render_Command command_header;
    command_header.type = RENDER_COMMAND_TYPE_CREATE;
    command_header.resource_type = resource_type;
    command_header.handle = handle;
    command_header.size = command_size;       

    __internal_on_command_send(&command_header, data);

    _render_thread->send(&command_header, data, data_size);

    return handle;
}

void Graphics_Driver::submit(Render_Message message, const void* data, size_t data_size) {
    tm_func();
    ST_ASSERT(_target, "Using Graphics_Driver without set target; call set_target()");
    
    size_t command_size = sizeof(Render_Command) + data_size;

    ST_ASSERT(data);

    Render_Command command_header;
    command_header.type = RENDER_COMMAND_TYPE_SUBMIT;
    command_header.message = message;
    command_header.size = command_size;

    __internal_on_command_send(&command_header, data);

    _render_thread->send(&command_header, data, data_size);

}
void Graphics_Driver::set(Resource_Handle hnd, Resource_Type resource_type, const void* data, size_t data_size) {
    tm_func();
    ST_ASSERT(_target, "Using Graphics_Driver without set target; call set_target()");

    size_t command_size = sizeof(Render_Command) + data_size;

    ST_ASSERT(this->get_resource_state(hnd) != RESOURCE_STATE_ERROR && this->get_resource_state(hnd) != RESOURCE_STATE_DEAD, "Invalid handle");
    
    ST_DEBUG_ASSERT(data_size);

    Render_Command command_header;
    command_header.type = RENDER_COMMAND_TYPE_SET;
    command_header.resource_type = resource_type;
    command_header.handle = hnd;
    command_header.size = command_size;       

    __internal_on_command_send(&command_header, data);

    _render_thread->send(&command_header, data, data_size);
}


void Graphics_Driver::append(Resource_Handle hnd, Resource_Type resource_type, const void* spec, size_t spec_size, const void* data, size_t data_size) {
    tm_func();
    ST_DEBUG_ASSERT(_target, "Using Render_Context without set target; call set_target()");

    size_t command_size = sizeof(Render_Command) + spec_size + data_size;

    ST_ASSERT(this->get_resource_state(hnd) != RESOURCE_STATE_ERROR && this->get_resource_state(hnd) != RESOURCE_STATE_DEAD, "Invalid handle");
    
    ST_DEBUG_ASSERT(data_size && spec_size);

    Render_Command command_header;
    command_header.type = RENDER_COMMAND_TYPE_APPEND;
    command_header.resource_type = resource_type;
    command_header.handle = hnd;
    command_header.size = command_size;

    // A bit verbose but avoids extra copies of data
    byte_t* command_buffer = (byte_t*)_render_thread->allocate_command(command_size);
    memcpy(command_buffer, &command_header, sizeof(Render_Command));
    command_buffer += sizeof(Render_Command);
    memcpy(command_buffer, spec, spec_size);
    command_buffer += spec_size;
    memcpy(command_buffer, data, data_size);

    //__internal_on_command_send(&command_header, command_buffer);
}

void Graphics_Driver::destroy(Resource_Handle hnd) {
    tm_func();
    ST_ASSERT(_target, "Using Graphics_Driver without set target; call set_target()");
    Render_Command command_header;
    command_header.type = RENDER_COMMAND_TYPE_SUBMIT;
    command_header.message = RENDER_MESSAGE_DESTROY;
    command_header.size = sizeof(Render_Command);
    command_header.handle = hnd;
    _render_thread->send(&command_header, NULL, 0);
}

void Graphics_Driver::client_acquire_target_context() {
    bool acquired = _context_mutex.try_lock();
    if (!acquired && _context_state_mutex.try_lock()) {
        _context_state = TARGET_CONTEXT_STATE_REQUEST;
        _context_state_mutex.unlock();
    }
    if (!acquired) _context_mutex.lock();
    ST_DEBUG_ASSERT(_context_state == TARGET_CONTEXT_STATE_AVAILABLE, "Concurrency fail");
    _target->bind();
    _context_state = TARGET_CONTEXT_STATE_CLIENT_ACQUIRED;
}

void Graphics_Driver::client_release_target_context() {
    
    ST_ASSERT(_context_state == TARGET_CONTEXT_STATE_CLIENT_ACQUIRED, "Context is not acquired; cannot release");

    {
        tm_scope("Unbind");
        _target->unbind();
    }
    _context_state = TARGET_CONTEXT_STATE_AVAILABLE;
    _context_mutex.unlock();
}

void Graphics_Driver::sync() {
    // TODO: #concurrency
    // This won't work as expected if Graphics_Driver is used
    // by multiple threads since the render thread may unlock
    // this mutex mid-command buffer if another thread has 
    // requested to require it.
    _context_mutex.lock();
    _context_mutex.unlock();
}

bool Graphics_Driver::query(Query_Type type, _Query_Result* result) {

    return this->__internal_query(type, result);
}

void Graphics_Driver::map_buffer(Resource_Handle buffer_hnd, Buffer_Access_Mode access_Mode, const std::function<void(void*)>& result_callback) {
    ST_ASSERT(_target, "Using Graphics_Driver without set target; call set_target()");

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
void Graphics_Driver::unmap_buffer(Resource_Handle buffer_hnd) {
    ST_ASSERT(_target, "Using Graphics_Driver without set target; call set_target()");

    ST_ASSERT(this->get_resource_state(buffer_hnd) != RESOURCE_STATE_ERROR && this->get_resource_state(buffer_hnd) != RESOURCE_STATE_DEAD);
    ST_ASSERT(this->get_resource_meta(buffer_hnd).type == RESOURCE_TYPE_BUFFER);

    _Unmap_Command cm;
    cm.buffer_hnd = buffer_hnd;
    this->submit(__INTERNAL_RENDER_MESSAGE_UNMAP_BUFFER, &cm, sizeof(cm));
}

const Graphics_Driver::Environment& Graphics_Driver::get_environment() const {
    std::lock_guard lock(_env_mutex);
    return _env;
}

void Graphics_Driver::swap_command_buffers() {
    ST_ASSERT(_target, "Using Graphics_Driver without set target; call set_target()");
    _render_thread->swap();

    for (const auto& [hnd, promise] : _mapping_promises) {
        void* result = promise->wait();
        promise->callback(result);

        _mapping_promise_allocator.deallocate_and_deconstruct(promise);
    }

    _mapping_promises.clear();
}

void Graphics_Driver::set_target(Device_Context* target) {
    ST_ASSERT(target, "Graphics_Driver must have a valid target");
    spec::submit::Set_Target spec;
    spec.target = target;
    this->submit(spec);
}
Device_Context* Graphics_Driver::get_current_target() const {
    return _target;
}

void Graphics_Driver::__enter_loop(std::mutex* buffer_mutex) {
    Timer frame_timer;

    _context_mutex.lock();
    _target->bind();
    this->__internal_init();

    _ready = true;
    _ready_cond.notify_all();
    
    while (!_render_thread->should_exit()) {
        
        std::unique_lock buffer_lock(*buffer_mutex);
        
        {
            tm_scope("Renderer cycle setup");
            _frame_time = frame_timer.record();
            frame_timer.reset();
            _target->unbind();
            {    
                tm_scope("State = Available");
                std::lock_guard state_lock(_context_state_mutex);
                _context_state = TARGET_CONTEXT_STATE_AVAILABLE;
            }
            _context_mutex.unlock();
        }
        
        {
            tm_scope("Wait buffer swap");
            _render_thread->_read_condition.wait_for(buffer_lock, std::chrono::seconds(5), [&]() {
                return (_render_thread->_buffer_state == utils::COMMAND_BUFFER_STATE_NEW 
                            && _context_state == TARGET_CONTEXT_STATE_AVAILABLE)
                        || !_running;
            });
        }
        tm_scope("Renderer cycle");
        {
            tm_scope("Wait context mutex");
            _context_mutex.lock();
        }
        _target->bind();
        _context_state = TARGET_CONTEXT_STATE_RENDERER_ACQUIRED;


        if (_render_thread->_buffer_state == utils::COMMAND_BUFFER_STATE_NEW) {
            _render_thread->traverse_commands<Render_Command>([&](Render_Command* header, void* data) {
                static const char* name = "";
                if (header->type == RENDER_COMMAND_TYPE_SUBMIT) {
                    name = render_message_to_string(header->message);
                } else {
                    name = resource_type_to_string(header->resource_type);
                }
                tm_scope(name);
                {
                    bool want_acquire_context = false;
                    {
                        tm_scope("Check context request");
                        std::lock_guard state_lock(_context_state_mutex);
                        want_acquire_context = _context_state == TARGET_CONTEXT_STATE_REQUEST;
                    }
                    if (want_acquire_context) {
                        tm_scope("Release context");
                        {
                            tm_scope("Set context state available");
                            std::lock_guard state_set_lock(_context_state_mutex);
                            _context_state = TARGET_CONTEXT_STATE_AVAILABLE;
                        }
                        _target->unbind();
                        _context_mutex.unlock();

                        _context_mutex.lock();
                        _target->bind();
                        {
                            tm_scope("Set context busy");
                            std::lock_guard state_set_lock(_context_state_mutex);
                            _context_state = TARGET_CONTEXT_STATE_RENDERER_ACQUIRED;
                        }
                    }
                }

                if (header->type == RENDER_COMMAND_TYPE_SUBMIT && header->message == RENDER_MESSAGE_SET_TARGET) {
                    auto& spec = *(spec::submit::Set_Target*)data;
                    _target = spec.target;
                    _target->bind();
                    this->__internal_on_context_change();
                } else {
                    this->__internal_handle_command(header, data);
                }
                return header->size;
            });
        }

        _render_thread->notify_buffer_finished();
    }   
    this->__internal_shutdown();
    log_info("Render thread shutdown");
}

NS_END(engine);
NS_END(graphics);
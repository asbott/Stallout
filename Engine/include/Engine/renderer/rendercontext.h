#pragma once

#include "Engine/memory.h"
#include "Engine/timing.h"

#include "os/graphics.h"

#include "Engine/renderer/rendercommands.h"

NS_BEGIN(engine);
NS_BEGIN(renderer);

enum Render_State {
    RENDER_STATE_READY, // Ready for swap buffers
    RENDER_STATE_BUSY // Busy; dont swap buffers
};
enum Command_Buffer_State {
    COMMAND_BUFFER_STATE_NEW, // Main buffer newly swapped, time to process
    COMMAND_BUFFER_STATE_OLD // Main buffer old, already processed
};






struct Mapping_Promise;

struct ST_API Render_Context {
    
    struct Environment {
        const char* vendor, *hardware, *driver, *version, *shading_version;
        u32 version_major, version_minor;
    };

    struct _Mapping_Promise;

    byte_t* _main_buffer;
    size_t _buffer_size;
    std::atomic_uint64_t _buffer_usage = 0;
    Linear_Allocator _buffer_allocator; // Contains secondary buffer to be swapped with main
    std::mutex _buffer_mutex;
    std::thread _render_thread;
    bool _running = true;
    Block_Allocator _id_allocator;
    Duration _frame_time;
    Hash_Map<Resource_Handle, _Mapping_Promise*> _mapping_promises;
    Block_Allocator _mapping_promise_allocator;
    std::condition_variable _ready_cond;
    bool _ready = false;
    Environment _env;
    mutable std::mutex _env_mutex;
    
    os::Window* _os_window;
    os::graphics::OS_Graphics_Context* _os_context;

    void* __internal = NULL;

    Command_Buffer_State _buffer_state = COMMAND_BUFFER_STATE_OLD;
    std::condition_variable _render_condition;
    std::condition_variable _swap_condition;

    struct _Mapping_Promise {
        std::mutex mut;
        std::condition_variable cond;
        std::function<void(void*)> callback;

        void* result = NULL;
        bool done = false;

        void* wait() {
            
            std::unique_lock lock(mut);
            cond.wait(lock, [&]() { return done; });

            return result;
        }
    };
    struct _Map_Command {
        Resource_Handle buffer_hnd;
        Buffer_Access_Mode access;

        _Mapping_Promise* promise;
    };
    struct _Unmap_Command {
        Resource_Handle buffer_hnd;
    };
    

    Render_Context(size_t buffer_size);
    ~Render_Context();

    void wait_ready();

    void _send_command(Render_Command command, const void* data, size_t data_size);
    Resource_Handle create(Resource_Type resource_type, const void* data, size_t data_size);
    void submit(Render_Message message, const void* data, size_t data_size);
    void set(Resource_Handle hnd, Resource_Type resource_type, const void* data, size_t data_size);
    void destroy(Resource_Handle hnd);

    template <typename type_t>
    Resource_Handle create(const type_t& data) {
        static_assert(std::is_trivially_copyable<type_t>());
        return this->create(type_t::type, &data, sizeof(type_t));
    }
    template <typename type_t>
    void submit(const type_t& data) {
        static_assert(std::is_trivially_copyable<type_t>());
        this->submit(type_t::message, &data, sizeof(type_t));
    }
    
    void map_buffer(Resource_Handle buffer_hnd, Buffer_Access_Mode access_Mode, const std::function<void(void*)>& result_callback);
    void unmap_buffer(Resource_Handle buffer_hnd);

    Environment get_environment() const;

    void swap_buffers();

    void __enter_loop();


    // Implemented per graphics API

    // API
    const Resource_Meta_Info& get_resource_meta(Resource_Handle hnd) const;
    Resource_State get_resource_state(Resource_Handle hnd) const;

    // Internal
    void __internal_init();
    void __internal_render(); 
    void __internal_shutdown();
};

NS_END(renderer);
NS_END(engine);
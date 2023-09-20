#pragma once

#include "Engine/memory.h"
#include "Engine/logger.h"

NS_BEGIN(engine)
NS_BEGIN(utils)

// Buffer which deallocates on free
// unless released.
// Used to pass ownership of buffers
// and let the receives decide if it
// should be freed when scope ends
// or released and freed at another
// point.
struct ST_API Shared_Buffer {
    mutable const void* buffer;
    const size_t size;
    bool _ownership;

    // Takes ownership
    Shared_Buffer(void* buffer, size_t size);
    Shared_Buffer(size_t size);
    Shared_Buffer(const Shared_Buffer& src) = delete;

    ~Shared_Buffer();

    void* release();
};

ST_API void* load_image(void* image_data, int buffer_size, int* width, int* height, int* channels, int forced_channels = 0);
ST_API void* load_image_from_file(const char* path, int* width, int* height, int* channels, int forced_channels = 0);
ST_API void free_image(void* data);

ST_API void* load_audio_from_memory(void* audio_data, size_t buffer_size, u32* channels, u32* sample_rate, u32* bits_per_sample, u64* frame_count);
ST_API void* load_audio_from_file(const char* path, u32* channels, u32* sample_rate, u32* bits_per_sample, u64* frame_count);
ST_API void free_audio(void* data);



enum Read_State {
    READ_STATE_READY, // Ready for swap buffers
    READ_STATE_BUSY // Busy; dont swap buffers
};
enum Command_Buffer_State {
    COMMAND_BUFFER_STATE_NEW, // Main buffer newly swapped, time to process
    COMMAND_BUFFER_STATE_OLD // Main buffer old, already processed
};
enum Thread_Run_State {
    THREAD_RUN_STATE_RUNNING,
    THREAD_RUN_STATE_PAUSED,
    THREAD_RUN_STATE_STOPPED
};

// Read thread is the created thread
// Write thread is the caller thread

struct Double_Buffered_Thread {
    typedef std::function<void(std::mutex* buffer_mutex)> thread_fn_t;
    std::thread _thread;
    thread_fn_t _thread_function;
    byte_t* _readbuffer; // Buffer to be read on separate thread
    Linear_Allocator  _writebuffer_allocator; // Buffer to be written to by caller thread
    std::mutex _buffer_mutex;
    const size_t _buffer_size;
    std::atomic_uint64_t _buffer_usage = 0;
    size_t _command_size = 0;
    bool _running;
    Thread_Run_State thread_run_state;

    Command_Buffer_State _buffer_state = COMMAND_BUFFER_STATE_OLD;
    std::condition_variable _read_condition;
    std::condition_variable _swap_condition;

    Double_Buffered_Thread(size_t buffer_sizes, const thread_fn_t& thread_fn);
    ~Double_Buffered_Thread();

    template <typename type_t>
    void set_command_type() {
        _command_size = sizeof(type_t);
    }
    
    void stop();

    void send(void* command, const void* data, size_t data_size);

    void swap();


    // For use on read thread
    void wait_swap(std::unique_lock<std::mutex>*);
    void notify_buffer_finished();
    bool should_exit() { return _running == false; };

    // Stream  command and move pointer to next command
    template <typename type_t>
    void traverse_commands(const std::function<size_t(type_t*, void*)>& callback) {
        ST_ASSERT(sizeof(type_t) == _command_size);
        size_t offset = 0;
        for (byte_t* next = _readbuffer; next < _readbuffer + _buffer_usage; next += offset) {
            void* data = next + _command_size;
            offset = callback((type_t*)next, data);

            ST_ASSERT(offset, "traverse_commands() must return offset to next command (sizeof command + sizeof data)");
        }
    };
};


NS_END(utils)
NS_END(engine)
#include "pch.h"

#include "utils.h"
#include "Engine/memory.h"

#include "os/io.h"

#ifdef _MSVC_LANG
#pragma warning (disable: 4244) // STB int conversion warnings
#endif

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define MINIAUDIO_IMPLEMENTATION
#define DR_FLAC_IMPLEMENTATION
#define DR_MP3_IMPLEMENTATION
#define DR_WAV_IMPLEMENTATION
#include "miniaudio.h"

NS_BEGIN(engine);
NS_BEGIN(utils);

Shared_Buffer::Shared_Buffer(void* buffer, size_t size)
    : buffer(buffer), size(size), _ownership(true) {
}

Shared_Buffer::Shared_Buffer(size_t size)
    : size(size), buffer(ST_MEM(size)), _ownership(true) {
    
}

Shared_Buffer::~Shared_Buffer() {
    if (_ownership) ST_FREE((void*)buffer, size);
}

void* Shared_Buffer::release() {
    _ownership = false;
    return (void*)buffer;
}


void* load_image(void* image_data, int buffer_size, int* width, int* height, int* channels, int forced_channels) {
    void* data = stbi_load_from_memory((stbi_uc*)image_data, buffer_size, width, height, channels, forced_channels);

    if (!data) return NULL;

    return data;
}

void* load_image_from_file(const char* path, int* width, int* height, int* channels, int forced_channels) {
    void* data = stbi_load(path, width, height, channels, forced_channels);

    if (!data) return NULL;

    return data;
}

void free_image(void* data) {
    stbi_image_free(data);
}

void* load_audio_from_memory(void* audio_data, size_t buffer_size, u32* channels, u32* sample_rate, u32* bits_per_sample, u64* frame_count) {
    void* output = NULL;
    
    ma_decoder_config cfg;
    memset(&cfg, 0, sizeof(ma_decoder_config));
    cfg.format = ma_format_s16;
    ma_result result = ma_decode_memory(audio_data, buffer_size, &cfg, frame_count, &output);
    *channels = cfg.channels;
    *sample_rate = cfg.sampleRate;
    *bits_per_sample = ma_get_bytes_per_sample(cfg.format) * 8;

    if (result != MA_SUCCESS) {
        return NULL;
    }

    return output;
}

void* load_audio_from_file(const char* path, u32* channels, u32* sample_rate, u32* bits_per_sample, u64* frame_count) {
    void* output = NULL;

    ma_decoder_config cfg;
    memset(&cfg, 0, sizeof(ma_decoder_config));
    cfg.format = ma_format_s16;
    auto result = ma_decode_file(path, &cfg, frame_count, &output);
    *channels = cfg.channels;
    *sample_rate = cfg.sampleRate;
    *bits_per_sample = ma_get_bytes_per_sample(cfg.format) * 8;

    if (result != MA_SUCCESS) {
        return NULL;
    }

    return output;
}

void free_audio(void* data) {
    ma_free(data, NULL);
}



byte_t* allocate_buffer(size_t buffer_size) {
    return (byte_t*)ST_MEMF(buffer_size, GLOBAL_ALLOC_FLAG_LARGE & GLOBAL_ALLOC_FLAG_STATIC);
}
void deallocate_buffer(void* buf, size_t sz) {
    ST_FREE(buf, sz);
}

Double_Buffered_Thread::Double_Buffered_Thread(size_t buffer_sizes, const Double_Buffered_Thread::thread_fn_t& thread_fn)
    : _buffer_size(buffer_sizes), 
    _thread_function(thread_fn), 
    _readbuffer(allocate_buffer(buffer_sizes)), 
    _writebuffer_allocator(allocate_buffer(buffer_sizes), buffer_sizes) {
    _thread = std::thread(thread_fn, &_buffer_mutex);
}
Double_Buffered_Thread::~Double_Buffered_Thread() {
    if (_running) this->stop();

    deallocate_buffer(_readbuffer, _buffer_size);
    deallocate_buffer(_writebuffer_allocator._head, _buffer_size);
}

void Double_Buffered_Thread::stop() {
    if (_writebuffer_allocator._head == _writebuffer_allocator._next) {
        this->swap(); // Swap if writebuffer not empty
    }

    _running = false;
    _read_condition.notify_all();

    _thread.join();
}

void Double_Buffered_Thread::send(void* command, const void* data, size_t data_size) {
    ST_ASSERT(_command_size, "No command type set in double buffer thread (call set_command_tpye)");
    size_t total_size = data_size + _command_size;
    // TODO: (2023-09-25) #unfinished #stupid
    // Just swap the buffers if write buffer is full....
    // Need to refactor a little bit so window->swap_buffers
    // isn't called for every command buffer (change up naming...)
    ST_ASSERT(_writebuffer_allocator._next + total_size <= _writebuffer_allocator._tail, "Command buffer overflow. Please allocate more.\nFree: {}kb/{}kb\nRequested: {}kb", (_buffer_size - (_writebuffer_allocator._tail - _writebuffer_allocator._next)) * 1000.0, _buffer_size * 1000.0, total_size * 1000.0);
    std::lock_guard lock(_buffer_mutex);
    byte_t* command_buffer = (byte_t*)_writebuffer_allocator.allocate(total_size);
    memcpy(command_buffer, command, _command_size);

    // Should be an option to pass uninitialized or no data
    if(data && data_size) memcpy(command_buffer + _command_size, data, data_size);
}

void Double_Buffered_Thread::swap() {
    std::unique_lock lock(_buffer_mutex);

    _swap_condition.wait(lock, [&]() { return _buffer_state == COMMAND_BUFFER_STATE_OLD; });

    _buffer_usage = (size_t)(_writebuffer_allocator._next - _writebuffer_allocator._head);;

    _readbuffer = (byte_t*)_writebuffer_allocator.swap_buffer(_readbuffer, _buffer_size);

    _buffer_state = COMMAND_BUFFER_STATE_NEW;
    _read_condition.notify_one();
}

void Double_Buffered_Thread::wait_swap(std::unique_lock<std::mutex>* lock) {
    _read_condition.wait(*lock, [&]() { 
        return _buffer_state == COMMAND_BUFFER_STATE_NEW || (!_running); 
    });
}

void Double_Buffered_Thread::notify_buffer_finished() {
    _buffer_state = COMMAND_BUFFER_STATE_OLD;
    _swap_condition.notify_one();
}

NS_END(utils)
NS_END(engine)
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

NS_END(utils)
NS_END(engine)
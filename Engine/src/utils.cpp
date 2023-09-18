#include "pch.h"

#include "utils.h"
#include "Engine/memory.h"

#ifdef _MSVC_LANG
#pragma warning (disable: 4244) // STB int conversion warnings
#endif

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

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

NS_END(utils)
NS_END(engine)
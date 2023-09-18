#pragma once

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

NS_END(utils)
NS_END(engine)
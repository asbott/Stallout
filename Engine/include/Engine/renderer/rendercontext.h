#pragma once

#include "Engine/memory.h"
#include "Engine/timing.h"

#include "os/graphics.h"

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


enum Render_Command_Type : u8 {
    RENDER_COMMAND_TYPE_CREATE,
    RENDER_COMMAND_TYPE_SUBMIT,
    RENDER_COMMAND_TYPE_SET
};
enum Render_Message : u32 {
    RENDER_MESSAGE_CLEAR,
    RENDER_MESSAGE_SET_CLEAR_COLOR,
    RENDER_MESSAGE_BIND_SHADER_UNIFORM_BUFFER,
    RENDER_MESSAGE_BIND_TEXTURE2D,
    RENDER_MESSAGE_DRAW_INDEXED,
    RENDER_MESSAGE_DESTROY,

    __INTERNAL_RENDER_MESSAGE_MAP_BUFFER,
    __INTERNAL_RENDER_MESSAGE_UNMAP_BUFFER
};



// Whenever this type is used it means its the actual
// value of the address of Resource_Handle. To be used
// in implementations.
typedef u64 Resource_ID;

// This will actually just be the pointer address
// to the real ID so the id is allocated as soon
// as clients submits a creation command and when
// buffers are swapped and the renderer handles the
// creation the actual resource id will be set at
// that address
typedef Resource_ID* Resource_Handle;



enum Resource_Type : u16 {
    RESOURCE_TYPE_TEXTURE2D,
    RESOURCE_TYPE_RENDERTARGET,
    RESOURCE_TYPE_SHADER,
    RESOURCE_TYPE_BUFFER,
    RESOURCE_TYPE_BUFFER_LAYOUT,

    RESOURCE_TYPE_COUNT,

    RESOURCE_TYPE_UNSET
};
enum Resource_State : u16 {
    RESOURCE_STATE_BUSY,
    RESOURCE_STATE_DEAD,
    RESOURCE_STATE_ERROR,
    RESOURCE_STATE_READY,

    RESOURCE_STATE_UNSET
};

struct Resource_Meta_Info {
    Resource_State state = RESOURCE_STATE_UNSET;
    Resource_Type type = RESOURCE_TYPE_UNSET;

    void* __internal;
};

enum Buffer_Type {
    BUFFER_TYPE_ARRAY_BUFFER,
    BUFFER_TYPE_ATOMIC_COUNTER_BUFFER,
    BUFFER_TYPE_COPY_READ_BUFFER,
    BUFFER_TYPE_COPY_WRITE_BUFFER,
    BUFFER_TYPE_DISPATCH_INDIRECT_BUFFER,
    BUFFER_TYPE_DRAW_INDIRECT_BUFFER,
    BUFFER_TYPE_ELEMENT_ARRAY_BUFFER,
    BUFFER_TYPE_PIXEL_PACK_BUFFER,
    BUFFER_TYPE_PIXEL_UNPACK_BUFFER,
    BUFFER_TYPE_QUERY_BUFFER,
    BUFFER_TYPE_SHADER_STORAGE_BUFFER,
    BUFFER_TYPE_TEXTURE_BUFFER,
    BUFFER_TYPE_TRANSFORM_FEEDBACK_BUFFER,
    BUFFER_TYPE_UNIFORM_BUFFER
};
enum Buffer_Usage {
    BUFFER_USAGE_STREAM_DRAW, 
    BUFFER_USAGE_STREAM_READ, 
    BUFFER_USAGE_STREAM_COPY, 
    BUFFER_USAGE_STATIC_DRAW, 
    BUFFER_USAGE_STATIC_READ, 
    BUFFER_USAGE_STATIC_COPY, 
    BUFFER_USAGE_DYNAMIC_DRAW, 
    BUFFER_USAGE_DYNAMIC_READ, 
    BUFFER_USAGE_DYNAMIC_COPY
};

typedef u8 Clear_Flags;
enum Clear_Flags_Impl {
    CLEAR_FLAG_COLOR = BIT(1),
    CLEAR_FLAG_STENCIL = BIT(2),
    CLEAR_FLAG_DEPTH = BIT(3),
};

enum Data_Type {
    DATA_TYPE_BYTE, 
    DATA_TYPE_UBYTE, 
    DATA_TYPE_SHORT, 
    DATA_TYPE_USHORT, 
    DATA_TYPE_INT, 
    DATA_TYPE_UINT, 
    DATA_TYPE_HALF, 
    DATA_TYPE_FLOAT, 
    DATA_TYPE_DOUBLE
};

enum Texture2D_Format {
    TEXTURE2D_FORMAT_RED,
    TEXTURE2D_FORMAT_RG,
    TEXTURE2D_FORMAT_RGB,
    TEXTURE2D_FORMAT_BGR,
    TEXTURE2D_FORMAT_RGBA,
    TEXTURE2D_FORMAT_BGRA,
    TEXTURE2D_FORMAT_RED_INTEGER,
    TEXTURE2D_FORMAT_RG_INTEGER,
    TEXTURE2D_FORMAT_RGB_INTEGER,
    TEXTURE2D_FORMAT_BGR_INTEGER,
    TEXTURE2D_FORMAT_RGBA_INTEGER,
    TEXTURE2D_FORMAT_BGRA_INTEGER,
};

struct Buffer_Layout_Entry {
    Buffer_Layout_Entry(size_t ncomponents, Data_Type data_type, bool normalized = false)
        :  ncomponents(ncomponents), data_type(data_type), normalized(normalized) {}
    size_t ncomponents;
    Data_Type data_type;
    bool normalized;

    //index, layout size (stride) & offset can be calculated later
};

enum Buffer_Access_Mode {
    BUFFER_ACCESS_MODE_READONLY,
    BUFFER_ACCESS_MODE_WRITEONLY,
    BUFFER_ACCESS_MODE_READWRITE
};

enum Draw_Mode {
    DRAW_MODE_POINTS, 
    DRAW_MODE_LINE_STRIP, 
    DRAW_MODE_LINE_LOOP, 
    DRAW_MODE_LINES, 
    DRAW_MODE_LINE_STRIP_ADJACENCY, 
    DRAW_MODE_LINES_ADJACENCY, 
    DRAW_MODE_TRIANGLE_STRIP, 
    DRAW_MODE_TRIANGLE_FAN, 
    DRAW_MODE_TRIANGLES, 
    DRAW_MODE_TRIANGLE_STRIP_ADJACENCY, 
    DRAW_MODE_TRIANGLES_ADJACENCY,
    DRAW_MODE_PATCHES
};

enum Texture_Wrap_Mode {
    TEXTURE_WRAP_MODE_CLAMP_TO_EDGE, 
    TEXTURE_WRAP_MODE_CLAMP_TO_BORDER, 
    TEXTURE_WRAP_MODE_MIRRORED_REPEAT, 
    TEXTURE_WRAP_MODE_REPEAT,
    TEXTURE_WRAP_MODE_MIRROR_CLAMP_TO_EDGE
};

enum Texture_Filter_Mode {
    TEXTURE_FILTER_MODE_NEAREST,
    TEXTURE_FILTER_MODE_LINEAR
};

enum Mipmap_Mode {
    MIPMAP_MODE_NEAREST,
    MIPMAP_MODE_LINEAR,
    MIPMAP_MODE_NONE
};

// Specification classes for render commands
NS_BEGIN(spec);
NS_BEGIN(submit);

struct Clear {
    Clear_Flags clear_flags;

    static const Render_Message message = RENDER_MESSAGE_CLEAR;
};

struct Set_Clear_Color {
    mz::fcolor16 clear_color;

    static const Render_Message message = RENDER_MESSAGE_SET_CLEAR_COLOR;
};

struct Bind_Shader_Uniform_Buffer {
    Resource_Handle buffer_hnd;
    Resource_Handle shader_hnd;

    // The name of the block in the shader
    const char* block_name;

    // Only one uniform buffer can be bound at a time per index
    size_t bind_index = 0;

    static const Render_Message message = RENDER_MESSAGE_BIND_SHADER_UNIFORM_BUFFER;
};

struct Bind_Texture2D {
    Resource_Handle hnd_texture;
    u64 slot;
    static const Render_Message message = RENDER_MESSAGE_BIND_TEXTURE2D;
};

struct Draw_Indexed {
    Resource_Handle vbo, ibo, layout, shader;

    Draw_Mode draw_mode;

    // Must be ubyte, ushort or uint
    Data_Type index_data_type;

    static const Render_Message message = RENDER_MESSAGE_DRAW_INDEXED;
};

NS_END(submit);

NS_BEGIN(create);

struct Shader {
    const char* vertex_source, *pixel_source;

    static const Resource_Type type = RESOURCE_TYPE_SHADER;
};

struct Buffer {
    Buffer_Type buffer_type;
    Buffer_Usage buffer_usage;

    static const Resource_Type type = RESOURCE_TYPE_BUFFER;
};


struct Buffer_Layout {
    Buffer_Layout(const std::initializer_list<Buffer_Layout_Entry>& entries) {
        pentries = (Buffer_Layout_Entry*)ST_MEM(entries.size() * sizeof(Buffer_Layout_Entry));
        memcpy(pentries, entries.begin(), entries.size() * sizeof(Buffer_Layout_Entry));
        num_entries = entries.size();
    }

    // Need to store pointer which needs to be
    // deleted later in the renderer impl 
    Buffer_Layout_Entry* pentries;
    size_t num_entries;

    static const Resource_Type type = RESOURCE_TYPE_BUFFER_LAYOUT;
};

struct Texture2D {
    u32 width, height, channels;

    // Format of the data to be passed in
    Texture2D_Format input_format;
    // Format to store the data internally
    Texture2D_Format internal_format;

    Data_Type component_type;

    Mipmap_Mode mipmap_mode = MIPMAP_MODE_NONE;
    Texture_Filter_Mode min_filter_mode = TEXTURE_FILTER_MODE_LINEAR;
    Texture_Filter_Mode mag_filter_mode = TEXTURE_FILTER_MODE_NEAREST;
    Texture_Wrap_Mode wrap_mode = TEXTURE_WRAP_MODE_CLAMP_TO_BORDER;

    static const Resource_Type type = RESOURCE_TYPE_TEXTURE2D;
}; 

NS_END(create);
NS_END(spec);

struct Render_Command {
    Render_Command_Type type;
    union {
        struct {
            Resource_Type resource_type;
            
        };
        Render_Message message;
    };
    Resource_Handle handle;
    size_t size;
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
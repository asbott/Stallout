#pragma once

#include "mz_vector.hpp"

NS_BEGIN(engine);
NS_BEGIN(renderer);

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

    Mipmap_Mode mipmap_mode = MIPMAP_MODE_LINEAR;
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

NS_END(renderer);
NS_END(engine);
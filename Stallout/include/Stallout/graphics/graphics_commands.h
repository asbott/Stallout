#pragma once //

#include "mz_vector.hpp"

NS_BEGIN(stallout); NS_BEGIN(os); NS_BEGIN(graphics);
struct Device_Context;
NS_END(stallout); NS_END(graphics); NS_END(os);

NS_BEGIN(stallout);
NS_BEGIN(graphics);

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

enum Error_Code {
    ERROR_CODE_NONE,
    ERROR_CODE_SHADER_COMPILATION_FAILED,
};

enum Render_Command_Type : u8 {
    RENDER_COMMAND_TYPE_CREATE,
    RENDER_COMMAND_TYPE_SUBMIT,
    RENDER_COMMAND_TYPE_SET,
    RENDER_COMMAND_TYPE_APPEND,
};
enum Render_Message : u32 {
    RENDER_MESSAGE_CLEAR,
    RENDER_MESSAGE_SET_CLEAR_COLOR,
    RENDER_MESSAGE_BIND_SHADER_UNIFORM_BUFFER,
    RENDER_MESSAGE_BIND_TEXTURE2D,
    RENDER_MESSAGE_DRAW_INDEXED,
    RENDER_MESSAGE_DESTROY,
    RENDER_MESSAGE_SET_BLENDING,
    RENDER_MESSAGE_TOGGLE,
    RENDER_MESSAGE_SET_POLYGON_MODE,
    RENDER_MESSAGE_SET_VIEWPORT,
    RENDER_MESSAGE_SET_SCISSOR_BOX,
    RENDER_MESSAGE_SET_TARGET,
    RENDER_MESSAGE_SWAP_RENDER_BUFFERS,
    RENDER_MESSAGE_GENERATE_MIPMAP,

    __INTERNAL_RENDER_MESSAGE_MAP_BUFFER,
    __INTERNAL_RENDER_MESSAGE_UNMAP_BUFFER
};
inline constexpr const char* render_message_to_string(Render_Message msg) {
    switch (msg) {
        case RENDER_MESSAGE_CLEAR: 
            return "RENDER_MESSAGE_CLEAR";
        case RENDER_MESSAGE_SET_CLEAR_COLOR: 
            return "RENDER_MESSAGE_SET_CLEAR_COLOR";
        case RENDER_MESSAGE_BIND_SHADER_UNIFORM_BUFFER: 
            return "RENDER_MESSAGE_BIND_SHADER_UNIFORM_BUFFER";
        case RENDER_MESSAGE_BIND_TEXTURE2D: 
            return "RENDER_MESSAGE_BIND_TEXTURE2D";
        case RENDER_MESSAGE_DRAW_INDEXED: 
            return "RENDER_MESSAGE_DRAW_INDEXED";
        case RENDER_MESSAGE_DESTROY: 
            return "RENDER_MESSAGE_DESTROY";
        case RENDER_MESSAGE_SET_BLENDING: 
            return "RENDER_MESSAGE_SET_BLENDING";
        case RENDER_MESSAGE_TOGGLE: 
            return "RENDER_MESSAGE_TOGGLE";
        case RENDER_MESSAGE_SET_POLYGON_MODE: 
            return "RENDER_MESSAGE_SET_POLYGON_MODE";
        case RENDER_MESSAGE_SET_VIEWPORT: 
            return "RENDER_MESSAGE_SET_VIEWPORT";
        case RENDER_MESSAGE_SET_SCISSOR_BOX: 
            return "RENDER_MESSAGE_SET_SCISSOR_BOX";
        case RENDER_MESSAGE_SET_TARGET: 
            return "RENDER_MESSAGE_SET_TARGET";
        case RENDER_MESSAGE_SWAP_RENDER_BUFFERS: 
            return "RENDER_MESSAGE_SWAP_RENDER_BUFFERS";
        case RENDER_MESSAGE_GENERATE_MIPMAP: 
            return "RENDER_MESSAGE_GENERATE_MIPMAP";
        case __INTERNAL_RENDER_MESSAGE_MAP_BUFFER: 
            return "__INTERNAL_RENDER_MESSAGE_MAP_BUFFER";
        case __INTERNAL_RENDER_MESSAGE_UNMAP_BUFFER: 
            return "__INTERNAL_RENDER_MESSAGE_UNMAP_BUFFER";
        default: 
            INTENTIONAL_CRASH("Unhandled enum");
            return nullptr; 
    }
}
enum Resource_Type : u16 {
    RESOURCE_TYPE_TEXTURE2D,
    RESOURCE_TYPE_RENDERTARGET,
    RESOURCE_TYPE_SHADER,
    RESOURCE_TYPE_BUFFER,
    RESOURCE_TYPE_BUFFER_LAYOUT,
    RESOURCE_TYPE_RENDERTEXTURE2D,

    RESOURCE_TYPE_COUNT,

    RESOURCE_TYPE_UNSET
};
inline constexpr const char* resource_type_to_string(Resource_Type type) {
    switch (type) {
        case RESOURCE_TYPE_TEXTURE2D: 
            return "RESOURCE_TYPE_TEXTURE2D";
        case RESOURCE_TYPE_RENDERTARGET: 
            return "RESOURCE_TYPE_RENDERTARGET";
        case RESOURCE_TYPE_SHADER: 
            return "RESOURCE_TYPE_SHADER";
        case RESOURCE_TYPE_BUFFER: 
            return "RESOURCE_TYPE_BUFFER";
        case RESOURCE_TYPE_BUFFER_LAYOUT: 
            return "RESOURCE_TYPE_BUFFER_LAYOUT";
        case RESOURCE_TYPE_RENDERTEXTURE2D: 
            return "RESOURCE_TYPE_RENDERTEXTURE2D";
        case RESOURCE_TYPE_COUNT: 
            return "RESOURCE_TYPE_COUNT";
        case RESOURCE_TYPE_UNSET: 
            return "RESOURCE_TYPE_UNSET";
        default: 
            INTENTIONAL_CRASH("Unhandled enum");
            return nullptr;
    }
}

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

    TEXTURE2D_FORMAT_RED_F16,
    TEXTURE2D_FORMAT_RG_F16,
    TEXTURE2D_FORMAT_RGB_F16,
    TEXTURE2D_FORMAT_RGBA_F16,

    TEXTURE2D_FORMAT_RED_NORM_S8,
    TEXTURE2D_FORMAT_RG_NORM_S8,
    TEXTURE2D_FORMAT_RGB_NORM_S8,
    TEXTURE2D_FORMAT_RGBA_NORM_S8,

    TEXTURE2D_FORMAT_RED_NORM_S16,
    TEXTURE2D_FORMAT_RG_NORM_S16,
    TEXTURE2D_FORMAT_RGB_NORM_S16,
    TEXTURE2D_FORMAT_RGBA_NORM_S16,

    TEXTURE2D_FORMAT_RED_U8,
    TEXTURE2D_FORMAT_RG_U8,
    TEXTURE2D_FORMAT_RGB_U8,
    TEXTURE2D_FORMAT_RGBA_U8,

    TEXTURE2D_FORMAT_RED_S8,
    TEXTURE2D_FORMAT_RG_S8,
    TEXTURE2D_FORMAT_RGB_S8,
    TEXTURE2D_FORMAT_RGBA_S8,

    TEXTURE2D_FORMAT_RED_U16,
    TEXTURE2D_FORMAT_RG_U16,
    TEXTURE2D_FORMAT_RGB_U16,
    TEXTURE2D_FORMAT_RGBA_U16,

    TEXTURE2D_FORMAT_RED_S16,
    TEXTURE2D_FORMAT_RG_S16,
    TEXTURE2D_FORMAT_RGB_S16,
    TEXTURE2D_FORMAT_RGBA_S16,

    TEXTURE2D_FORMAT_RED_U32,
    TEXTURE2D_FORMAT_RG_U32,
    TEXTURE2D_FORMAT_RGB_U32,
    TEXTURE2D_FORMAT_RGBA_U32,

    TEXTURE2D_FORMAT_RED_S32,
    TEXTURE2D_FORMAT_RG_S32,
    TEXTURE2D_FORMAT_RGB_S32,
    TEXTURE2D_FORMAT_RGBA_S32,

    

    TEXTURE2D_FORMAT_RED_NORM_BYTE = TEXTURE2D_FORMAT_RED_NORM_S8,
    TEXTURE2D_FORMAT_RG_NORM_BYTE = TEXTURE2D_FORMAT_RG_NORM_S8,
    TEXTURE2D_FORMAT_RGB_NORM_BYTE = TEXTURE2D_FORMAT_RGB_NORM_S8,
    TEXTURE2D_FORMAT_RGBA_NORM_BYTE = TEXTURE2D_FORMAT_RGBA_NORM_S8,

    TEXTURE2D_FORMAT_RED_NORM_SHORT = TEXTURE2D_FORMAT_RED_NORM_S16,
    TEXTURE2D_FORMAT_RG_NORM_SHORT = TEXTURE2D_FORMAT_RG_NORM_S16,
    TEXTURE2D_FORMAT_RGB_NORM_SHORT = TEXTURE2D_FORMAT_RGB_NORM_S16,
    TEXTURE2D_FORMAT_RGBA_NORM_SHORT = TEXTURE2D_FORMAT_RGBA_NORM_S16,


    TEXTURE2D_FORMAT_RED_F32 = TEXTURE2D_FORMAT_RED,
    TEXTURE2D_FORMAT_RG_F32 = TEXTURE2D_FORMAT_RG,
    TEXTURE2D_FORMAT_RGB_F32 = TEXTURE2D_FORMAT_RGB,
    TEXTURE2D_FORMAT_RGBA_F32 = TEXTURE2D_FORMAT_RGBA,

    TEXTURE2D_FORMAT_RED_NORM_S32 = TEXTURE2D_FORMAT_RED_F32,
    TEXTURE2D_FORMAT_RG_NORM_S32 = TEXTURE2D_FORMAT_RG_F32,
    TEXTURE2D_FORMAT_RGB_NORM_S32 = TEXTURE2D_FORMAT_RGB_F32,
    TEXTURE2D_FORMAT_RGBA_NORM_S32 = TEXTURE2D_FORMAT_RGBA_F32,

    TEXTURE2D_FORMAT_RED_NORM_INT = TEXTURE2D_FORMAT_RED_NORM_S32,
    TEXTURE2D_FORMAT_RG_NORM_INT = TEXTURE2D_FORMAT_RG_NORM_S32,
    TEXTURE2D_FORMAT_RGB_NORM_INT = TEXTURE2D_FORMAT_RGB_NORM_S32,
    TEXTURE2D_FORMAT_RGBA_NORM_INT = TEXTURE2D_FORMAT_RGBA_NORM_S32,

    TEXTURE2D_FORMAT_RED_HALF = TEXTURE2D_FORMAT_RED_F16,
    TEXTURE2D_FORMAT_RG_HALF = TEXTURE2D_FORMAT_RG_F16,
    TEXTURE2D_FORMAT_RGB_HALF = TEXTURE2D_FORMAT_RGB_F16,
    TEXTURE2D_FORMAT_RGBA_HALF = TEXTURE2D_FORMAT_RGBA_F16,

    TEXTURE2D_FORMAT_RED_FLOAT = TEXTURE2D_FORMAT_RED,
    TEXTURE2D_FORMAT_RG_FLOAT = TEXTURE2D_FORMAT_RG,
    TEXTURE2D_FORMAT_RGB_FLOAT = TEXTURE2D_FORMAT_RGB,
    TEXTURE2D_FORMAT_RGBA_FLOAT = TEXTURE2D_FORMAT_RGBA,

    TEXTURE2D_FORMAT_RED_UBYTE = TEXTURE2D_FORMAT_RED_U8,
    TEXTURE2D_FORMAT_RG_UBYTE = TEXTURE2D_FORMAT_RG_U8,
    TEXTURE2D_FORMAT_RGB_UBYTE = TEXTURE2D_FORMAT_RGB_U8,
    TEXTURE2D_FORMAT_RGBA_UBYTE = TEXTURE2D_FORMAT_RGBA_U8,

    TEXTURE2D_FORMAT_RED_BYTE = TEXTURE2D_FORMAT_RED_S8,
    TEXTURE2D_FORMAT_RG_BYTE = TEXTURE2D_FORMAT_RG_S8,
    TEXTURE2D_FORMAT_RGB_BYTE = TEXTURE2D_FORMAT_RGB_S8,
    TEXTURE2D_FORMAT_RGBA_BYTE = TEXTURE2D_FORMAT_RGBA_S8,

    TEXTURE2D_FORMAT_RED_USHORT = TEXTURE2D_FORMAT_RED_U16,
    TEXTURE2D_FORMAT_RG_USHORT = TEXTURE2D_FORMAT_RG_U16,
    TEXTURE2D_FORMAT_RGB_USHORT = TEXTURE2D_FORMAT_RGB_U16,
    TEXTURE2D_FORMAT_RGBA_USHORT = TEXTURE2D_FORMAT_RGBA_U16,

    TEXTURE2D_FORMAT_RED_SHORT = TEXTURE2D_FORMAT_RED_S16,
    TEXTURE2D_FORMAT_RG_SHORT = TEXTURE2D_FORMAT_RG_S16,
    TEXTURE2D_FORMAT_RGB_SHORT = TEXTURE2D_FORMAT_RGB_S16,
    TEXTURE2D_FORMAT_RGBA_SHORT = TEXTURE2D_FORMAT_RGBA_S16,

    TEXTURE2D_FORMAT_RED_UINT = TEXTURE2D_FORMAT_RED_U32,
    TEXTURE2D_FORMAT_RG_UINT = TEXTURE2D_FORMAT_RG_U32,
    TEXTURE2D_FORMAT_RGB_UINT = TEXTURE2D_FORMAT_RGB_U32,
    TEXTURE2D_FORMAT_RGBA_UINT = TEXTURE2D_FORMAT_RGBA_U32,

    TEXTURE2D_FORMAT_RED_INT = TEXTURE2D_FORMAT_RED_S32,
    TEXTURE2D_FORMAT_RG_INT = TEXTURE2D_FORMAT_RG_S32,
    TEXTURE2D_FORMAT_RGB_INT = TEXTURE2D_FORMAT_RGB_S32,
    TEXTURE2D_FORMAT_RGBA_INT = TEXTURE2D_FORMAT_RGBA_S32,
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

enum Blend_Equation {
    BLEND_EQUATION_ADD,
    BLEND_EQUATION_SUBTRACT,
    BLEND_EQUATION_REVERSE_SUBTRACT,
    BLEND_EQUATION_MAX,
    BLEND_EQUATION_MIN,
    BLEND_EQUATION_NONE
};

enum Blend_Func_Factor {
    BLEND_FACTOR_ZERO,
    BLEND_FACTOR_ONE,
    BLEND_FACTOR_SRC_COLOR,
    BLEND_FACTOR_ONE_MINUS_SRC_COLOR,
    BLEND_FACTOR_DST_COLOR,
    BLEND_FACTOR_ONE_MINUS_DST_COLOR,
    BLEND_FACTOR_SRC_ALPHA,
    BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
    BLEND_FACTOR_DST_ALPHA,
    BLEND_FACTOR_ONE_MINUS_DST_ALPHA,
    BLEND_FACTOR_CONSTANT_COLOR,
    BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR,
    BLEND_FACTOR_CONSTANT_ALPHA,
    BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA,
    BLEND_FACTOR_SRC_ALPHA_SATURATE,
    BLEND_FACTOR_SRC1_COLOR,
    BLEND_FACTOR_ONE_MINUS_SRC1_COLOR,
    BLEND_FACTOR_SRC1_ALPHA,
    BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA
};

enum Renderer_Setting_Flags : s32 {
    RENDERER_SETTING_UNSET = 0,
    RENDERER_SETTING_CULLING = BIT(1),
    RENDERER_SETTING_DEPTH_TESTING = BIT(2),
    RENDERER_SETTING_STENCIL_TESTING = BIT(3),
    RENDERER_SETTING_PRIMITIVE_RESTART = BIT(4),
    RENDERER_SETTING_SCISSOR_TESTING = BIT(5),

    RENDERER_SETTING_MAX,
};
FLAGIFY(Renderer_Setting_Flags);

enum Query_Type : s32 {
    QUERY_TYPE_RENDERER_SETTINGS_FLAGS,
    QUERY_TYPE_BLENDING_EQUATION,
    QUERY_TYPE_BLENDING_SRC_COLOR,
    QUERY_TYPE_BLENDING_DST_COLOR,
    QUERY_TYPE_BLENDING_SRC_ALPHA,
    QUERY_TYPE_BLENDING_DST_ALPHA,
    QUERY_TYPE_CULLED_FACE,
    QUERY_TYPE_POLY_MODE_FRONT,
    QUERY_TYPE_POLY_MODE_BACK,
    QUERY_TYPE_VIEWPORT,
    QUERY_TYPE_TEXTURE_SLOT, // This + x gives the resource handle of the texture bound to slot x (NULL if none)
    QUERY_TYPE_NEXT = QUERY_TYPE_TEXTURE_SLOT + 1000,
};


enum Polygon_Face {
    POLY_FACE_FRONT,
    POLY_FACE_BACK,
    POLY_FACE_FRONT_AND_BACK
};

enum Polygon_Mode {
    POLY_MODE_FILL,
    POLY_MODE_LINE,
    POLY_MODE_POINT,
};

// Specification classes for render commands
NS_BEGIN(spec);
NS_BEGIN(submit);

struct Clear {
    Clear_Flags clear_flags;
    Resource_Handle render_texture = 0; // Clear buffer in render texture; 0 for clearing main buffer

    static const Render_Message message = RENDER_MESSAGE_CLEAR;
};

struct Set_Clear_Color {
    mz::fcolor16 clear_color;
    Resource_Handle render_texture = 0;

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
    Resource_Handle render_texture = 0;

    Draw_Mode draw_mode;

    // Must be ubyte, ushort or uint
    Data_Type index_data_type;

    size_t index_count;

    // Where at which index of indices in the index
    // buffer to start from
    size_t indices_offset = 0;

    static const Render_Message message = RENDER_MESSAGE_DRAW_INDEXED;
};

struct Set_Blending {
    Blend_Equation equation;

    Blend_Func_Factor src_color_factor;
    Blend_Func_Factor dst_color_factor;
    Blend_Func_Factor src_alpha_factor;
    Blend_Func_Factor dst_alpha_factor;

    static const Render_Message message = RENDER_MESSAGE_SET_BLENDING;
};

struct Toggle {
    bool enabled;
    Renderer_Setting_Flags settings = RENDERER_SETTING_UNSET;

    Resource_Handle render_texture = 0;

    static const Render_Message message = RENDER_MESSAGE_TOGGLE;
};

struct Set_Polygon_Mode {
    Polygon_Face face;
    Polygon_Mode mode;

    static const Render_Message message = RENDER_MESSAGE_SET_POLYGON_MODE;
};

struct Set_Viewport {
    mz::s32vec2 pos;
    mz::s32vec2 size;

    static const Render_Message message = RENDER_MESSAGE_SET_VIEWPORT;
};

struct Set_Scissor_Box {

    // x, y, width, height
    mz::irect rect;

    static const Render_Message message = RENDER_MESSAGE_SET_SCISSOR_BOX;
};


struct Set_Target {
    os::graphics::Device_Context* target = NULL;
    static const Render_Message message = RENDER_MESSAGE_SET_TARGET;
};

struct Swap_Render_Buffers {
    os::graphics::Device_Context* dc;  
    static const Render_Message message = RENDER_MESSAGE_SWAP_RENDER_BUFFERS;
};

struct Generate_Mipmap {
    Resource_Handle htexture;
    static const Render_Message message = RENDER_MESSAGE_GENERATE_MIPMAP;
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

// TODO: #design #api #graphics
// Make no sense to specify input format here.
// It should be passed with the data when setting
// it.
// Actually all of Texture2D api should be revamped.
// We could get rid of Data_Type and deduce that in
// the backend based on the now extensive Format enum.
// And no reason to specify channels.
// AAaaaand separate internal and input format types.
// Maybe just a simpler approach where we specify how
// many channels and what datatype they are.!
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
    Texture_Wrap_Mode wrap_mode = TEXTURE_WRAP_MODE_CLAMP_TO_EDGE;

    static const Resource_Type type = RESOURCE_TYPE_TEXTURE2D;
}; 

struct Render_Texture2D {
    Texture2D tex_spec;

    static const Resource_Type type = RESOURCE_TYPE_RENDERTEXTURE2D;
};

NS_END(create);

NS_BEGIN(append);

struct Buffer {
    size_t offset;
    size_t data_size;

    static const Resource_Type type = RESOURCE_TYPE_BUFFER;
};

struct Texture2D {
    u32 xoffset, yoffset;
    u32 width, height;
    u32 channels;
    size_t data_size;

    Data_Type storage_type;
    Texture2D_Format format;

    static const Resource_Type type = RESOURCE_TYPE_TEXTURE2D;
};

NS_END(append);


NS_END(spec);


struct Render_Command {
    Render_Command_Type type;
    union {
        Resource_Type resource_type;
        Render_Message message;
    };
    Resource_Handle handle;
    size_t size;
};

NS_END(graphics);
NS_END(stallout);
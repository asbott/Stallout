#include "pch.h"

#include "renderer/rendercontext.h"
#include "Engine/logger.h"
#include "Engine/stringutils.h"


#include <glad/glad.h>



const char* gl_debug_source_string(GLenum source) {
    switch (source) {
        case GL_DEBUG_SOURCE_API: return "OpenGL API";
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM: return "Window-System API";
        case GL_DEBUG_SOURCE_SHADER_COMPILER: return "Shader Compiler";
        case GL_DEBUG_SOURCE_THIRD_PARTY: return "Third Party";
        case GL_DEBUG_SOURCE_APPLICATION: return "Application";
        case GL_DEBUG_SOURCE_OTHER: return "Other";
        default: return "INVALID";
    }
}
const char* gl_debug_type_string(GLenum type) {
    switch (type) {
        case GL_DEBUG_TYPE_ERROR: return "Error";
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "Deprecated";
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: return "Undefined Behaviour";
        case GL_DEBUG_TYPE_PORTABILITY: return "Portability";
        case GL_DEBUG_TYPE_PERFORMANCE: return "Performance";
        case GL_DEBUG_TYPE_MARKER: return "Marker";
        case GL_DEBUG_TYPE_PUSH_GROUP: return "Push Group";
        case GL_DEBUG_TYPE_POP_GROUP: return "Pop Group";
        case GL_DEBUG_TYPE_OTHER: return "Other";
        default: return "INVALID";
    }
}

void GLAPIENTRY gl_debug_callback( 
                 GLenum source,
                 GLenum type,
                 GLuint id,
                 GLenum severity,
                 GLsizei /*length*/,
                 const GLchar* message,
                 const void* /*user_data*/ )
{
    #define log_it(fn, sev) log_##fn(\
    "OpenGL Message\n"\
    "Severity: {}\n"\
    "Source: {}\n"\
    "Type: {}\n"\
    "ID: {}\n"\
    "Message:\n{}",\
    sev, gl_debug_source_string(source), gl_debug_type_string(type), id, message)

    switch (severity) {
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            log_it(trace, "Notification");
            break;
        case GL_DEBUG_SEVERITY_LOW:
            log_it(warn, "Low");
            break;
        case GL_DEBUG_SEVERITY_MEDIUM:
            log_it(error, "Medium");
            break;
        case GL_DEBUG_SEVERITY_HIGH:
            if (source != GL_DEBUG_SOURCE_SHADER_COMPILER) {
                log_it(critical, "High");
            } else {
                log_it(error, "High");
            }
            break;
    }

    if (source != GL_DEBUG_SOURCE_SHADER_COMPILER) { // Ignore shader errors because they are handled in runtime
        ST_DEBUG_ASSERT(severity != GL_DEBUG_SEVERITY_HIGH, "Severe OpenGL Message");
    }
}
bool has_single_uniform(GLuint shader) {
    GLint numUniforms = 0, numUniformBlocks = 0;

    // Query the number of active uniforms and uniform blocks
    glGetProgramiv(shader, GL_ACTIVE_UNIFORMS, &numUniforms);
    glGetProgramiv(shader, GL_ACTIVE_UNIFORM_BLOCKS, &numUniformBlocks);

    // Early exit if there are no uniforms at all
    if (numUniforms == 0) {
        return false;
    }

    std::vector<GLint> blockUniformIndices;

    // Loop through each uniform block to find out which uniforms are part of blocks
    for (int block = 0; block < numUniformBlocks; ++block) {
        GLint numUniformsInBlock = 0;

        // Get the number of uniforms in this block
        glGetActiveUniformBlockiv(shader, block, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &numUniformsInBlock);

        std::vector<GLint> blockIndices(numUniformsInBlock);

        // Get the indices of the uniforms in this block
        glGetActiveUniformBlockiv(shader, block, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, &blockIndices[0]);

        // Add the indices to the list of uniforms which are part of blocks
        blockUniformIndices.insert(blockUniformIndices.end(), blockIndices.begin(), blockIndices.end());
    }

    // Now check each individual uniform to see if it's NOT part of any block
    for (int i = 0; i < numUniforms; ++i) {
        if (std::find(blockUniformIndices.begin(), blockUniformIndices.end(), i) == blockUniformIndices.end()) {
            // This uniform is NOT part of any block

            char uniformName[256];
            GLsizei length;
            // If we dont pass these it will crash on nvidia drivers......
            GLint type_size;
            GLenum type;
            glGetActiveUniform(shader, i, sizeof(uniformName), &length, &type_size, &type, uniformName);

            // Skip the one that's named "textures"
            if (strcmp(uniformName, "textures[0]") != 0) {
                return true;
            }
        }
    }

    // No "single uniforms" found
    return false;
}







NS_BEGIN(engine);
NS_BEGIN(renderer);

// GL ids are unique per resource type
// So we need to convert between gl ids
// and our universal resource id
typedef GLuint GL_ID;

static_assert(sizeof(GL_ID) < sizeof(Resource_ID) && "GL_ID type size must fit in Resource_Id type size");

Resource_ID to_resource_id(GL_ID glid, Resource_Type rtype) {
    constexpr auto max_id = std::numeric_limits<GL_ID>::max();
    constexpr u8 num_types = (size_t)RESOURCE_TYPE_COUNT;
    constexpr GL_ID base = max_id / num_types;

    ST_DEBUG_ASSERT(glid < base, "Invalid GL ID; too large");

    return base * (GL_ID)rtype + glid;
}

GL_ID to_gl_id(Resource_ID rid, Resource_Type rtype) {
    constexpr auto max_id = std::numeric_limits<GL_ID>::max();
    constexpr u8 num_types = (size_t)RESOURCE_TYPE_COUNT;
    constexpr GL_ID base = max_id / num_types;

    ST_DEBUG_ASSERT(rid >= (base * (size_t)rtype) && rid <= ((base * (size_t)rtype) + base - 1), "Resource_Id out of range for this resource type");
    
    return (GL_ID)rid - (GL_ID)rtype * base;
}

GL_ID to_gl_enum(Buffer_Type type) {
    switch (type) {
        case BUFFER_TYPE_ARRAY_BUFFER:
            return GL_ARRAY_BUFFER;
        case BUFFER_TYPE_ATOMIC_COUNTER_BUFFER:
            return GL_ATOMIC_COUNTER_BUFFER;
        case BUFFER_TYPE_COPY_READ_BUFFER:
            return GL_COPY_READ_BUFFER;
        case BUFFER_TYPE_COPY_WRITE_BUFFER:
            return GL_COPY_WRITE_BUFFER;
        case BUFFER_TYPE_DISPATCH_INDIRECT_BUFFER:
            return GL_DISPATCH_INDIRECT_BUFFER;
        case BUFFER_TYPE_DRAW_INDIRECT_BUFFER:
            return GL_DRAW_INDIRECT_BUFFER;
        case BUFFER_TYPE_ELEMENT_ARRAY_BUFFER:
            return GL_ELEMENT_ARRAY_BUFFER;
        case BUFFER_TYPE_PIXEL_PACK_BUFFER:
            return GL_PIXEL_PACK_BUFFER;
        case BUFFER_TYPE_PIXEL_UNPACK_BUFFER:
            return GL_PIXEL_UNPACK_BUFFER;
        case BUFFER_TYPE_QUERY_BUFFER:
            return GL_QUERY_BUFFER;
        case BUFFER_TYPE_SHADER_STORAGE_BUFFER:
            return GL_SHADER_STORAGE_BUFFER;
        case BUFFER_TYPE_TEXTURE_BUFFER:
            return GL_TEXTURE_BUFFER;
        case BUFFER_TYPE_TRANSFORM_FEEDBACK_BUFFER:
            return GL_TRANSFORM_FEEDBACK_BUFFER;
        case BUFFER_TYPE_UNIFORM_BUFFER:
            return GL_UNIFORM_BUFFER;
        default:
            INTENTIONAL_CRASH("Invalid Buffer_Usage enum value");
            return BUFFER_TYPE_ARRAY_BUFFER;
    }
}

GL_ID to_gl_enum(Buffer_Usage usage) {
    switch (usage) {
        case BUFFER_USAGE_STREAM_DRAW:
            return GL_STREAM_DRAW;
        case BUFFER_USAGE_STREAM_READ:
            return GL_STREAM_READ;
        case BUFFER_USAGE_STREAM_COPY:
            return GL_STREAM_COPY;
        case BUFFER_USAGE_STATIC_DRAW:
            return GL_STATIC_DRAW;
        case BUFFER_USAGE_STATIC_READ:
            return GL_STATIC_READ;
        case BUFFER_USAGE_STATIC_COPY:
            return GL_STATIC_COPY;
        case BUFFER_USAGE_DYNAMIC_DRAW:
            return GL_DYNAMIC_DRAW;
        case BUFFER_USAGE_DYNAMIC_READ:
            return GL_DYNAMIC_READ;
        case BUFFER_USAGE_DYNAMIC_COPY:
            return GL_DYNAMIC_COPY;
        default:
            INTENTIONAL_CRASH("Invalid Buffer_Usage enum value");
            return BUFFER_USAGE_DYNAMIC_COPY;
    }
}

GL_ID to_gl_enum(Data_Type e) {
    switch (e) {
        case DATA_TYPE_BYTE: return GL_BYTE;
        case DATA_TYPE_UBYTE: return GL_UNSIGNED_BYTE;
        case DATA_TYPE_SHORT: return GL_SHORT;
        case DATA_TYPE_USHORT: return GL_UNSIGNED_SHORT;
        case DATA_TYPE_INT: return GL_INT;
        case DATA_TYPE_UINT: return GL_UNSIGNED_INT;
        case DATA_TYPE_HALF: return GL_HALF_FLOAT;
        case DATA_TYPE_FLOAT: return GL_FLOAT;
        case DATA_TYPE_DOUBLE: return GL_DOUBLE;
        default:
            INTENTIONAL_CRASH("Invalid Data_Type enum value");
            return DATA_TYPE_UINT;
    }
}

size_t data_type_to_size(Data_Type e) {
    switch (e) {
        case DATA_TYPE_BYTE: return sizeof(byte_t);
        case DATA_TYPE_UBYTE: return sizeof(unsigned char);
        case DATA_TYPE_SHORT: return sizeof(short);
        case DATA_TYPE_USHORT: return sizeof(unsigned short);
        case DATA_TYPE_INT: return sizeof(int);
        case DATA_TYPE_UINT: return sizeof(unsigned int);
        case DATA_TYPE_HALF: return sizeof(float) / 2;
        case DATA_TYPE_FLOAT: return sizeof(float);
        case DATA_TYPE_DOUBLE: return sizeof(double);
        default:
            INTENTIONAL_CRASH("Invalid Data_Type enum value");
            return 0;
    }
}

GL_ID to_gl_enum(Texture2D_Format e) {
    switch (e) {
        case TEXTURE2D_FORMAT_RED: return GL_RED;
        case TEXTURE2D_FORMAT_RG: return GL_RG;
        case TEXTURE2D_FORMAT_RGB: return GL_RGB;
        case TEXTURE2D_FORMAT_BGR: return GL_BGR;
        case TEXTURE2D_FORMAT_RGBA: return GL_RGBA;
        case TEXTURE2D_FORMAT_BGRA: return GL_BGRA;
        case TEXTURE2D_FORMAT_RED_INTEGER: return GL_RED_INTEGER;
        case TEXTURE2D_FORMAT_RG_INTEGER: return GL_RG_INTEGER;
        case TEXTURE2D_FORMAT_RGB_INTEGER: return GL_RGB_INTEGER;
        case TEXTURE2D_FORMAT_BGR_INTEGER: return GL_BGR_INTEGER;
        case TEXTURE2D_FORMAT_RGBA_INTEGER: return GL_RGBA_INTEGER;
        case TEXTURE2D_FORMAT_BGRA_INTEGER: return GL_BGRA_INTEGER;
        default:
            INTENTIONAL_CRASH("Invalid Texture2D_Format enum value");
            return TEXTURE2D_FORMAT_BGR;
    }
}

GL_ID to_gl_enum(Buffer_Access_Mode e) {
    switch (e) {
        case BUFFER_ACCESS_MODE_READONLY: return GL_READ_ONLY;
        case BUFFER_ACCESS_MODE_WRITEONLY: return GL_WRITE_ONLY;
        case BUFFER_ACCESS_MODE_READWRITE: return GL_READ_WRITE;
        default:
            INTENTIONAL_CRASH("Invalid Buffer_Access_Mode enum value");
            return BUFFER_ACCESS_MODE_READWRITE;
    }
}

GL_ID to_gl_enum(Draw_Mode mode) {
    switch (mode) {
        case DRAW_MODE_POINTS: return GL_POINTS;
        case DRAW_MODE_LINE_STRIP: return GL_LINE_STRIP;
        case DRAW_MODE_LINE_LOOP: return GL_LINE_LOOP;
        case DRAW_MODE_LINES: return GL_LINES;
        case DRAW_MODE_LINE_STRIP_ADJACENCY: return GL_LINE_STRIP_ADJACENCY;
        case DRAW_MODE_LINES_ADJACENCY: return GL_LINES_ADJACENCY;
        case DRAW_MODE_TRIANGLE_STRIP: return GL_TRIANGLE_STRIP;
        case DRAW_MODE_TRIANGLE_FAN: return GL_TRIANGLE_FAN;
        case DRAW_MODE_TRIANGLES: return GL_TRIANGLES;
        case DRAW_MODE_TRIANGLE_STRIP_ADJACENCY: return GL_TRIANGLE_STRIP_ADJACENCY;
        case DRAW_MODE_TRIANGLES_ADJACENCY: return GL_TRIANGLES_ADJACENCY;
        case DRAW_MODE_PATCHES: return GL_PATCHES;
        default:
            INTENTIONAL_CRASH("Invalid Draw_Mode enum value");
            return GL_POINTS; 
    }
}
GL_ID to_gl_enum(Texture_Wrap_Mode mode) {
    switch (mode) {
        case TEXTURE_WRAP_MODE_CLAMP_TO_EDGE: return GL_CLAMP_TO_EDGE;
        case TEXTURE_WRAP_MODE_CLAMP_TO_BORDER: return GL_CLAMP_TO_BORDER;
        case TEXTURE_WRAP_MODE_MIRRORED_REPEAT: return GL_MIRRORED_REPEAT;
        case TEXTURE_WRAP_MODE_REPEAT: return GL_REPEAT;
        case TEXTURE_WRAP_MODE_MIRROR_CLAMP_TO_EDGE: return GL_MIRROR_CLAMP_TO_EDGE;
        default:
            INTENTIONAL_CRASH("Invalid Draw_Mode enum value");
            return 0; 
    }
}
GL_ID to_gl_enum(Texture_Filter_Mode mode) {
    switch (mode) {
        case TEXTURE_FILTER_MODE_NEAREST: return GL_NEAREST;
        case TEXTURE_FILTER_MODE_LINEAR: return GL_LINEAR;
        default:
            INTENTIONAL_CRASH("Invalid Draw_Mode enum value");
            return 0; 
    }
}

GL_ID decide_mipmap_filter(Texture_Filter_Mode min_filter, Mipmap_Mode mipmap_filter) {
    if (mipmap_filter == MIPMAP_MODE_NONE) {
        return to_gl_enum(min_filter);
    }

    if (min_filter == TEXTURE_FILTER_MODE_NEAREST) {
        if (mipmap_filter == MIPMAP_MODE_NEAREST) {
            return GL_NEAREST_MIPMAP_NEAREST;
        } else if (mipmap_filter == MIPMAP_MODE_LINEAR) {
            return GL_NEAREST_MIPMAP_LINEAR;
        }
    } else if (min_filter == TEXTURE_FILTER_MODE_LINEAR) {
        if (mipmap_filter == MIPMAP_MODE_NEAREST) {
            return GL_LINEAR_MIPMAP_NEAREST;
        } else if (mipmap_filter == MIPMAP_MODE_LINEAR) {
            return GL_LINEAR_MIPMAP_LINEAR;
        }
    }

    INTENTIONAL_CRASH("Invalid min/mipmap filter");
    return 0; 
}

bool glad_initialized = false;

struct Buffer_State {
    Buffer_Type buffer_type;
    Buffer_Usage buffer_usage;
    size_t size = 0;
    bool mapped = false;
};
struct Texture2D_State {
    u32 width, height, channels;

    Texture2D_Format input_format;
    Texture2D_Format internal_format;

    Data_Type component_type;

    Mipmap_Mode mipmap_mode;
    Texture_Filter_Mode min_filter_mode;
    Texture_Filter_Mode mag_filter_mode;
    Texture_Wrap_Mode wrap_mode;
};

struct Shader_State {
    Hash_Map<const char*, GLuint> uniform_blocks;
};

struct Layout_State {
    Array<Buffer_Layout_Entry> entries;
    size_t total_size;
    GL_ID last_draw_glvbo;
};

struct GL_State {
    std::mutex resource_meta_map_mutex;
    Hash_Map<Resource_ID, Resource_Meta_Info> resource_meta_map; // Stored per ID

    Hash_Map<Resource_ID, Buffer_State> buffers;
    Hash_Map<Resource_ID, Texture2D_State> texture2ds;
    Hash_Map<Resource_ID, Shader_State> shaders;
    Hash_Map<Resource_ID, Layout_State> layouts;

    mz::color current_clear_color;

    GLuint last_active_texture_slot = 0;
    GL_ID last_bound_gltexture = 0;
    GL_ID last_bound_shader = 0;
};

void check_resource_handle(GL_State& state, Resource_Handle hnd) {
    std::lock_guard lock(state.resource_meta_map_mutex);

    ST_ASSERT(hnd, "Null handle");
    ST_ASSERT(state.resource_meta_map.contains(*hnd), "Invalid resource handle");

    const auto& meta = state.resource_meta_map[*hnd];

    ST_ASSERT(meta.state != RESOURCE_STATE_DEAD && meta.state != RESOURCE_STATE_ERROR, "Unusable resource");
}
void check_resource_handle_is(GL_State& state, Resource_Handle hnd, Resource_Type expected_type) {
    check_resource_handle(state, hnd);
    ST_ASSERT(state.resource_meta_map[*hnd].type == expected_type, "Unexpected resource type");
}

GLuint create_shader(const engine::renderer::spec::create::Shader& spec, engine::New_String* error, Shader_State* ss) {
    GLint max_slots;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &max_slots);

    // Update vertex shader source
    const char vert_src_prefix[] = "#version 450 core\n";
    New_String ext_vert_src(strlen(vert_src_prefix) + strlen(spec.vertex_source) + 1);
    sprintf(ext_vert_src.str, "%s%s", vert_src_prefix, spec.vertex_source);

    // Update fragment shader source
    const char frag_src_prefix_fmt[] = "#version 450 core\nconst int MAX_TEXTURES = %i;\nuniform sampler2D textures[MAX_TEXTURES];\n";
    New_String frag_src_prefix(sizeof(frag_src_prefix_fmt) + 4);
    sprintf(frag_src_prefix.str, frag_src_prefix_fmt, max_slots);

    New_String ext_frag_src(strlen(frag_src_prefix.str) + strlen(spec.pixel_source) + 1);
    sprintf(ext_frag_src.str, "%s%s", frag_src_prefix.str, spec.pixel_source);

    log_trace("Compiling shader\n\nVERTEX:\n{}\n\nFRAGMENT:{}", ext_vert_src.str, ext_frag_src.str);

    // Compile vertex shader
    GLuint vert_shader;
    vert_shader = glCreateShader(GL_VERTEX_SHADER);
    
    glShaderSource(vert_shader, 1, &ext_vert_src.str, NULL);
    glCompileShader(vert_shader);

    // Check for vertex shader compilation errors
    GLint vert_result;
    glGetShaderiv(vert_shader, GL_COMPILE_STATUS, &vert_result);
    if (vert_result != GL_TRUE)
    {
        GLsizei log_length = 0;
        GLchar message[1024];
        glGetShaderInfoLog(vert_shader, 1024, &log_length, message);
        const char fmt[]= "Vertex Shader compilation failed: %s";
        *error = engine::New_String(sizeof(message) + sizeof(fmt));

        sprintf(error->str, fmt, message);

        return 0;
    }

    // Compile fragment shader
    GLuint frag_shader;
    frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag_shader, 1, &ext_frag_src.str, NULL);
    glCompileShader(frag_shader);

    // Check for fragment shader compilation errors
    GLint frag_result;
    glGetShaderiv(frag_shader, GL_COMPILE_STATUS, &frag_result);
    if (frag_result != GL_TRUE)
    {
        GLsizei log_length = 0;
        GLchar message[1024];
        glGetShaderInfoLog(frag_shader, 1024, &log_length, message);

        const char fmt[]= "Fragment Shader compilation failed: %s";
        *error = engine::New_String(sizeof(message) + sizeof(fmt));

        sprintf(error->str, fmt, message);

        return 0;
    }

    // Link shaders into a shader program
    GLuint shader_program;
    shader_program = glCreateProgram();
    glAttachShader(shader_program, vert_shader);
    glAttachShader(shader_program, frag_shader);
    glLinkProgram(shader_program);

    // Check for linking errors
    GLint link_result;
    glGetProgramiv(shader_program, GL_LINK_STATUS, &link_result);
    if (link_result != GL_TRUE)
    {
        GLsizei log_length = 0;
        GLchar message[1024];
        glGetProgramInfoLog(shader_program, 1024, &log_length, message);
        const char fmt[]= "Shader linking failed: %s";
        *error = engine::New_String(sizeof(message) + sizeof(fmt));

        sprintf(error->str, fmt, message);

        return 0;
    }

    if (has_single_uniform(shader_program)) {
        *error = engine::New_String(sizeof("Single uniforms are not supported; please use uniform buffers.") + 1);
        strcpy(error->str, "Single uniforms are not supported; please use uniform buffers.");

        return 0;
    }

    GLint num_active_uniforms = 0;
    glGetProgramiv(shader_program, GL_ACTIVE_UNIFORM_BLOCKS, &num_active_uniforms);

    for (GLint i = 0; i < num_active_uniforms; ++i) {
        GLint name_len = -1;
        glGetProgramiv(shader_program, GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH, &name_len);

        New_String name(name_len + 1);
        GLsizei actual_len = 0;
        glGetActiveUniformBlockName(shader_program, i, name_len, &actual_len, name.str);

        name.str[actual_len] = '\0';

        if (strcmp(name.str, "textures") == 0) continue;

        GLint location = glGetUniformBlockIndex(shader_program, name.str);

        // TODO: Memory leak
        ss->uniform_blocks[name.release()] = location;
    }

    GLint textures_location = glGetUniformLocation(shader_program, "textures");
    if (textures_location != -1) {
        Array<GLint>texture_indices;
        texture_indices.reserve(max_slots);
        for (int i = 0; i < max_slots; ++i) {
            texture_indices.push_back(i);
        }

        // Set the uniform array to use texture units from 0 to MAX_TEXTURES - 1
        glUseProgram(shader_program);
        glUniform1iv(textures_location, max_slots, texture_indices.data());
    }

    glDeleteShader(vert_shader);
    glDeleteShader(frag_shader);
    glUseProgram(0);

    return shader_program;
}


void handle_creation(GL_State& state, Render_Command* header, void* data) {
    Resource_Meta_Info rinfo;
    rinfo.type = header->resource_type;
    Resource_ID rid = 0;
    switch (header->resource_type)
    {
        case RESOURCE_TYPE_SHADER:
        {
            log_trace("Creating Shader Resource");
            const auto& shader_spec = *(spec::create::Shader*)data;

            Shader_State ss;
            engine::New_String shader_err("");
            if (GL_ID shader = create_shader(shader_spec, &shader_err, &ss)) {
                rid = to_resource_id(shader, header->resource_type);
                state.shaders[rid] = ss;
                rinfo.state = RESOURCE_STATE_READY; 
                log_info("Successfully created a shader with GL_ID {} and RID {}", shader, rid);
            } else {
                rinfo.state = RESOURCE_STATE_ERROR;
                log_error("Shader error:\n{}", shader_err.str);
            }

            break;
        }
        case RESOURCE_TYPE_BUFFER:
        {
            log_trace("Creating Buffer Resource");
            const auto& spec = *(spec::create::Buffer*)data;

            GLuint buffer;
            glGenBuffers(1, &buffer);

            rid = to_resource_id(buffer, header->resource_type);

            Buffer_State bd;
            bd.buffer_type = spec.buffer_type;
            bd.buffer_usage = spec.buffer_usage;

            state.buffers[rid] = bd;

            rinfo.state = RESOURCE_STATE_READY;
            
            break;
        }
        case RESOURCE_TYPE_BUFFER_LAYOUT:
        {
            log_trace("Creating Buffer Layout Resource");
            const auto& spec = *(spec::create::Buffer_Layout*)data;

            size_t total_size = 0;
            for (size_t i = 0; i < spec.num_entries; i++) {
                total_size += data_type_to_size(spec.pentries[i].data_type) * spec.pentries[i].ncomponents;
            }

            GLuint vao;
            glGenVertexArrays(1, &vao);

            Layout_State layout_state;
            
            for (GLsizei i = 0; i < spec.num_entries; i++) {
                layout_state.entries.push_back(spec.pentries[i]);
            }

            layout_state.total_size = total_size;


            ST_FREE(spec.pentries, spec.num_entries * sizeof(Buffer_Layout_Entry));

            rid = to_resource_id(vao, header->resource_type);
            state.layouts[rid] = layout_state;
            rinfo.state = RESOURCE_STATE_READY;

            break;
        }
        case RESOURCE_TYPE_TEXTURE2D:
        {
            log_trace("Creating Texture2D Resource");
            const auto& spec = *(spec::create::Texture2D*)data;
            GLuint texture;
            glGenTextures(1, &texture);
            rid = to_resource_id(texture, header->resource_type);

            Texture2D_State td;
            td.width = spec.width;
            td.height = spec.height;
            td.channels = spec.channels;
            td.component_type = spec.component_type;
            td.input_format = spec.input_format;
            td.internal_format = spec.internal_format;
            td.mipmap_mode = spec.mipmap_mode;
            td.min_filter_mode = spec.min_filter_mode;
            td.mag_filter_mode = spec.mag_filter_mode;
            td.wrap_mode = spec.wrap_mode;
            state.texture2ds[rid] = td;

            rinfo.state = RESOURCE_STATE_READY;

            break;
        }
        default:
        {
            INTENTIONAL_CRASH("Unimplemented");
            break;   
        }
    }

    ST_DEBUG_ASSERT(rinfo.state != RESOURCE_STATE_ERROR ? rid != 0 : true, "RID not correctly set in creation");
    ST_DEBUG_ASSERT(rinfo.state != RESOURCE_STATE_UNSET, "Resource state not correctly set in creation");
    ST_DEBUG_ASSERT(rinfo.type  != RESOURCE_TYPE_UNSET,  "Resource type not correctly set in creation");

    *header->handle = rid;
    std::lock_guard rlock(state.resource_meta_map_mutex);
    state.resource_meta_map[rid] = rinfo;
}

void handle_set(GL_State& state, Render_Command* header, void* data) {
    size_t data_size = header->size - sizeof(Render_Command);
    Resource_ID rid = *header->handle;
    Resource_Meta_Info rinfo;
    {
        std::lock_guard l(state.resource_meta_map_mutex);

        ST_ASSERT(state.resource_meta_map.contains(rid), "Invalid RID");

        rinfo = state.resource_meta_map[rid];

    }
    ST_ASSERT(rinfo.state != RESOURCE_STATE_ERROR && rinfo.state != RESOURCE_STATE_DEAD);
    ST_ASSERT(rinfo.type == header->resource_type, "Resource type mismatch");
    
    switch (header->resource_type)
    {
    case RESOURCE_TYPE_BUFFER:
    {
        ST_ASSERT(state.buffers.contains(rid), "Missing buffer for RID");
        GL_ID buffer = to_gl_id(rid, RESOURCE_TYPE_BUFFER);
        auto& buffer_data = state.buffers[rid];
        buffer_data.size = data_size;
        
        GL_ID buffer_type = to_gl_enum(buffer_data.buffer_type);

        if (data && data_size) {
            glBindBuffer(buffer_type, buffer);
            glBufferData(buffer_type, data_size, data, to_gl_enum(buffer_data.buffer_usage));
            glBindBuffer(buffer_type, 0);
        }

        break;
    }
    case RESOURCE_TYPE_TEXTURE2D:
    {
        ST_ASSERT(state.texture2ds.contains(rid), "Missing Texture2D for RID");
        GL_ID texture = to_gl_id(rid, RESOURCE_TYPE_TEXTURE2D);
        auto& tex_data = state.texture2ds[rid];
        
        glBindTexture(GL_TEXTURE_2D, texture);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, to_gl_enum(tex_data.wrap_mode));	
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, to_gl_enum(tex_data.wrap_mode));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, decide_mipmap_filter(tex_data.min_filter_mode, tex_data.mipmap_mode));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, to_gl_enum(tex_data.mag_filter_mode));
        glTexImage2D(GL_TEXTURE_2D, 0, to_gl_enum(tex_data.internal_format), tex_data.width, tex_data.height, 0, to_gl_enum(tex_data.input_format), to_gl_enum(tex_data.component_type), data);

        if (tex_data.mipmap_mode != MIPMAP_MODE_NONE) {
            glGenerateMipmap(GL_TEXTURE_2D);
        }

        if (state.last_active_texture_slot) {
            glActiveTexture(state.last_active_texture_slot);
            glBindTexture(GL_TEXTURE_2D, state.last_bound_gltexture);
        }

        break;
    }
    case RESOURCE_TYPE_BUFFER_LAYOUT:
    {
        
        INTENTIONAL_CRASH("Cannot set data in RESOURCE_TYPE_BUFFER_LAYOUT");
        break;
    }
    case RESOURCE_TYPE_SHADER:
    {
        INTENTIONAL_CRASH("Cannot set data in shader");

        break;
    }
    default:
        INTENTIONAL_CRASH("Unimplemented");
        break;
    }
    
}

void handle_message(GL_State& state, engine::renderer::Render_Command* header, void* data) {
    switch (header->message)
    {
    case RENDER_MESSAGE_CLEAR:
    {
        const auto& spec = *(spec::submit::Clear*)data;
        int gl_clear_flags = 0;

        if (spec.clear_flags & CLEAR_FLAG_COLOR) gl_clear_flags |= GL_COLOR_BUFFER_BIT;
        if (spec.clear_flags & CLEAR_FLAG_STENCIL) gl_clear_flags |= GL_STENCIL_BUFFER_BIT;
        if (spec.clear_flags & CLEAR_FLAG_DEPTH) gl_clear_flags |= GL_DEPTH_BUFFER_BIT;
        
        
        glClear(gl_clear_flags);


        break;
    }
    case RENDER_MESSAGE_SET_CLEAR_COLOR:
    {
        const auto& spec = *(spec::submit::Set_Clear_Color*)data;
        
        if (spec.clear_color != state.current_clear_color) {
            glClearColor(spec.clear_color.r, spec.clear_color.g, spec.clear_color.b, spec.clear_color.a);
            state.current_clear_color = spec.clear_color;
        }
        break;
    }
    case RENDER_MESSAGE_BIND_SHADER_UNIFORM_BUFFER:
    {
        const auto& spec = *(spec::submit::Bind_Shader_Uniform_Buffer*)data;
        check_resource_handle_is(state, spec.buffer_hnd, RESOURCE_TYPE_BUFFER);
        check_resource_handle_is(state, spec.shader_hnd, RESOURCE_TYPE_SHADER);

        Resource_ID buffer_id = *spec.buffer_hnd;
        Resource_ID shader_id = *spec.shader_hnd;

        const auto& buffer_state = state.buffers[buffer_id];
        auto& shader_state = state.shaders[shader_id];

        ST_ASSERT(buffer_state.buffer_type == BUFFER_TYPE_UNIFORM_BUFFER);

        GL_ID glbuffer = to_gl_id(buffer_id, RESOURCE_TYPE_BUFFER);
        GL_ID glshader = to_gl_id(shader_id, RESOURCE_TYPE_SHADER);

        if (shader_state.uniform_blocks.contains(spec.block_name)) {
            // Bind shader block to given index
            glUniformBlockBinding(glshader, shader_state.uniform_blocks[spec.block_name], (GLuint)spec.bind_index);

            // Bind the ubo to given index
            glBindBufferBase(GL_UNIFORM_BUFFER, (GLuint)spec.bind_index, glbuffer);
        } else {
            log_warn("Missing shader uniform block '{}' was set", spec.block_name);
        }

        break;
    }
    case RENDER_MESSAGE_BIND_TEXTURE2D:
    {
        const auto& spec = *(spec::submit::Bind_Texture2D*)data;
        check_resource_handle_is(state, spec.hnd_texture, RESOURCE_TYPE_TEXTURE2D);

        Resource_ID texture_id = *spec.hnd_texture;

        GL_ID gltexture = to_gl_id(texture_id, RESOURCE_TYPE_TEXTURE2D);

        glActiveTexture(GL_TEXTURE0 + (GLenum)spec.slot);
        glBindTexture(GL_TEXTURE_2D, gltexture);

        state.last_active_texture_slot = GL_TEXTURE0 + (GLuint)spec.slot;
        state.last_bound_gltexture = gltexture;

        break;
    }
    case RENDER_MESSAGE_DRAW_INDEXED:
    {
        const auto& spec = *(spec::submit::Draw_Indexed*)data;
        check_resource_handle_is(state, spec.vbo, RESOURCE_TYPE_BUFFER);
        check_resource_handle_is(state, spec.ibo, RESOURCE_TYPE_BUFFER);
        check_resource_handle_is(state, spec.layout, RESOURCE_TYPE_BUFFER_LAYOUT);
        check_resource_handle_is(state, spec.shader, RESOURCE_TYPE_SHADER);

        Resource_ID vbo = *spec.vbo;
        Resource_ID ibo = *spec.ibo;
        Resource_ID layout = *spec.layout;
        Resource_ID shader = *spec.shader;

        const auto& vbo_state = state.buffers[vbo];
        const auto& ibo_state = state.buffers[ibo];
        auto& layout_state = state.layouts[layout];

        ST_ASSERT(vbo_state.buffer_type == BUFFER_TYPE_ARRAY_BUFFER, "Expected array buffer");
        ST_ASSERT(ibo_state.buffer_type == BUFFER_TYPE_ELEMENT_ARRAY_BUFFER, "Expected element buffer");


        ST_ASSERT(spec.index_data_type == DATA_TYPE_UBYTE || spec.index_data_type == DATA_TYPE_USHORT || spec.index_data_type == DATA_TYPE_UINT, "Invalid data type for indices");

        GL_ID glvbo = to_gl_id(vbo, RESOURCE_TYPE_BUFFER);
        GL_ID glibo = to_gl_id(ibo, RESOURCE_TYPE_BUFFER);
        GL_ID gllayout = to_gl_id(layout, RESOURCE_TYPE_BUFFER_LAYOUT);
        GL_ID glshader = to_gl_id(shader, RESOURCE_TYPE_SHADER);

        if (glshader != state.last_bound_shader) {
            glUseProgram(glshader);
        }

        glBindVertexArray(gllayout);
        glBindBuffer(to_gl_enum(vbo_state.buffer_type), glvbo);
        glBindBuffer(to_gl_enum(ibo_state.buffer_type), glibo);

        // Set vao layout if needed
        if (layout_state.last_draw_glvbo != glvbo) {
            layout_state.last_draw_glvbo = glvbo;

            size_t offset = 0;
            const size_t sz = layout_state.entries.size();
            for (size_t i = 0; i < sz; i++) {
                const auto& entry = layout_state.entries[i];

                size_t type_size = data_type_to_size(entry.data_type);

                glVertexAttribPointer((GLuint)i, (GLint)entry.ncomponents, to_gl_enum(entry.data_type), entry.normalized, (GLsizei)layout_state.total_size, (const void*)offset);
                glEnableVertexAttribArray((GLuint)i);

                offset += type_size * entry.ncomponents;
            }

        }

        glDrawElements(to_gl_enum(spec.draw_mode), (GLsizei)(ibo_state.size / data_type_to_size(spec.index_data_type)), to_gl_enum(spec.index_data_type), 0);

        break;
    }
    case RENDER_MESSAGE_DESTROY: {
        check_resource_handle(state, header->handle);
        Resource_ID rid = *header->handle;
        Resource_Type type;
        {
            std::lock_guard lock(state.resource_meta_map_mutex);
            ST_ASSERT(state.resource_meta_map[rid].state != RESOURCE_STATE_DEAD, "Resource is already destroyed");
            type = state.resource_meta_map[rid].type;

            state.resource_meta_map[rid].state = RESOURCE_STATE_DEAD;
        }

        GL_ID gid = to_gl_id(rid, type);

        switch (type) {
            case RESOURCE_TYPE_TEXTURE2D:
            {
                glDeleteTextures(1, &gid);
                break;
            }
            case RESOURCE_TYPE_RENDERTARGET:
            {
                glDeleteFramebuffers(1, &gid);
                break;
            }
            case RESOURCE_TYPE_SHADER:
            {
                glDeleteProgram(gid);
                break;
            }
            case RESOURCE_TYPE_BUFFER:
            {
                glDeleteBuffers(1, &gid);
                break;
            }
            case RESOURCE_TYPE_BUFFER_LAYOUT:
            {
                glDeleteVertexArrays(1, &gid);
                break;
            }
            default:
            {
                INTENTIONAL_CRASH("Unhandled resource type destroy");
                break;
            }
        }

        
        break;
    }
    case __INTERNAL_RENDER_MESSAGE_MAP_BUFFER:
    {   
        const auto& spec = *(Render_Context::_Map_Command*)data;
        check_resource_handle_is(state, spec.buffer_hnd, RESOURCE_TYPE_BUFFER);

        Resource_ID bufferid = *spec.buffer_hnd;

        auto& buffer_state = state.buffers[bufferid];
        GL_ID glbuffer = to_gl_id(bufferid, RESOURCE_TYPE_BUFFER);

        GL_ID gl_buffer_type = to_gl_enum(buffer_state.buffer_type);

        glBindBuffer(gl_buffer_type, glbuffer);

        ST_ASSERT(buffer_state.size, "Trying to map null data in buffer");
        ST_ASSERT(!buffer_state.mapped, "Buffer is already mapped");

        void* buffer = glMapBuffer(gl_buffer_type, to_gl_enum(spec.access));
        glBindBuffer(gl_buffer_type, 0);

        buffer_state.mapped = true;

        spec.promise->result = buffer;
        spec.promise->done = true;
        spec.promise->cond.notify_all();

        break;
    }
    case __INTERNAL_RENDER_MESSAGE_UNMAP_BUFFER:
    {
        const auto& spec = *(Render_Context::_Unmap_Command*)data;
        check_resource_handle_is(state, spec.buffer_hnd, RESOURCE_TYPE_BUFFER);

        Resource_ID bufferid = *spec.buffer_hnd;

        auto& buffer_state = state.buffers[bufferid];
        GL_ID glbuffer = to_gl_id(bufferid, RESOURCE_TYPE_BUFFER);

        GL_ID gl_buffer_type = to_gl_enum(buffer_state.buffer_type);

        glBindBuffer(gl_buffer_type, glbuffer);
        glUnmapBuffer(gl_buffer_type);
        glBindBuffer(gl_buffer_type, 0);

        buffer_state.mapped = false;

        break;
    }
    default:
        ST_ASSERT(false);
        break;
    }
    
}



const Resource_Meta_Info& Render_Context::get_resource_meta(Resource_Handle hnd) const {
    ST_ASSERT(__internal, "Renderer not ready");
    ST_DEBUG_ASSERT(hnd);

    Resource_ID rid = *hnd;
    auto&  gl_state = *(GL_State*)__internal;

    std::lock_guard lock(gl_state.resource_meta_map_mutex);
    
    ST_DEBUG_ASSERT(gl_state.resource_meta_map.contains(rid), "Invalid resource ID");

    return gl_state.resource_meta_map[rid];
}
Resource_State Render_Context::get_resource_state(Resource_Handle hnd) const {
    ST_ASSERT(__internal, "Renderer not ready");
    ST_DEBUG_ASSERT(hnd);

    ST_DEBUG_ASSERT(_id_allocator.contains(hnd), "Invalid resource handle");


    auto&  gl_state = *(GL_State*)__internal;
    
    if (gl_state.resource_meta_map.contains(*hnd)) {
        return get_resource_meta(hnd).state;
    }

    return RESOURCE_STATE_BUSY; // Still waiting for creation
}

void Render_Context::__internal_init() {
    _os_context->make_current();
    if (!glad_initialized) {
        ST_ASSERT(gladLoadGL(), "Failed loading GLAD");
        glad_initialized = true;

        log_info("Initialized GLAD");

        #ifdef _ST_CONFIG_DEBUG
        glEnable              ( GL_DEBUG_OUTPUT );
        glDebugMessageCallback( gl_debug_callback, 0 );
        log_info("Set up OpenGL Debug callback");
        #endif
    }
    
    __internal = ST_NEW(GL_State);

    // TODO:
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    

    std::lock_guard lock(_env_mutex);

    GLint major, minor;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);

    char* version = (char*)ST_MEM(16);
    memset(version, 0, 16);
    sprintf(version, "OpenGL %i.%i", major, minor);
    
    _env.version = version;
    _env.version_major = major;
    _env.version_minor = minor;
    _env.vendor = (const char*)glGetString(GL_VENDOR);
    _env.hardware = (const char*)glGetString(GL_RENDERER);
    _env.driver = (const char*)glGetString(GL_VERSION);
    _env.shading_version = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
}
void Render_Context::__internal_render() {
    auto& state = *(GL_State*)__internal;

    _render_thread.traverse_commands<Render_Command>([&](Render_Command* header, void* data) {
        switch (header->type)
        {
            case RENDER_COMMAND_TYPE_CREATE:
                handle_creation(state, header, data);
                break;
            case RENDER_COMMAND_TYPE_SUBMIT:
                handle_message(state, header ,data);
                break;
            case RENDER_COMMAND_TYPE_SET:
                handle_set(state, header ,data);
                break;
            default:
                ST_ASSERT(false);
                break;
        }

        return header->size;
    });

    
}
void Render_Context::__internal_shutdown() {
    ST_DELETE((GL_State*)__internal);

    ST_FREE((char*)_env.version, 16);
}

NS_END(renderer);
NS_END(engine);
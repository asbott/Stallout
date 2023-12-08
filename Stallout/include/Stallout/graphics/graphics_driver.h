#pragma once //

#include "Stallout/memory.h"
#include "Stallout/timing.h"
#include "Stallout/utils.h"
#include "Stallout/containers.h"

#include "Stallout/graphics/graphics_commands.h"

#include <mz_vector.hpp>

NS_BEGIN(stallout);NS_BEGIN(os);NS_BEGIN(graphics);
struct Device_Context;
NS_END(stallout);NS_END(os); NS_END(graphics);

NS_BEGIN(stallout);
NS_BEGIN(graphics);

using namespace os::graphics;

struct Graphics_Driver;

struct Mapping_Promise;

enum Target_Context_State {
    TARGET_CONTEXT_STATE_AVAILABLE,
    TARGET_CONTEXT_STATE_RENDERER_ACQUIRED,
    TARGET_CONTEXT_STATE_CLIENT_ACQUIRED,
    TARGET_CONTEXT_STATE_REQUEST
};

struct ST_API Graphics_Driver {
    
    struct Environment {
        const char *vendor, *hardware, *driver, *version, *shading_version;
        u32 version_major, version_minor;

        u32 max_texture_slots;
        u32 max_texture_size;
    };

    struct _Mapping_Promise;

    utils::Double_Buffered_Thread* _render_thread;
    bool _running = true;
    Block_Allocator _id_allocator;
    Duration _frame_time;
    Hash_Map<Resource_Handle, _Mapping_Promise*> _mapping_promises;
    Block_Allocator _mapping_promise_allocator;
    std::condition_variable _ready_cond;
    bool _ready = false;
    Environment _env;
    mutable std::mutex _env_mutex;
    Device_Context* _target = NULL;
    std::mutex command_mutex;

    void* __internal = NULL;

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
    

    Graphics_Driver(Device_Context* target, size_t buffer_size = 1000 * 1000 * 100);
    ~Graphics_Driver();

    void wait_ready();
    
    Resource_Handle create(Resource_Type resource_type, const void* data, size_t data_size);
    void submit(Render_Message message, const void* data, size_t data_size);

    // TODO (2023-11-21): #fix #api #rendering 
    // Shouldnt need to specify resource type because its already
    // stored in driver state. 
    void set(Resource_Handle hnd, Resource_Type resource_type, const void* data, size_t data_size);
    void append(Resource_Handle hnd, Resource_Type resource_type, const void* spec, size_t spec_size, const void* data, size_t data_size);
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
    template <typename type_t>
    void append(Resource_Handle hnd, const type_t& spec, const void* data) {
        static_assert(std::is_trivially_copyable<type_t>());
        this->append(hnd, type_t::type, &spec, sizeof(spec), data, spec.data_size);
    }

    void sync(); // Wait for render thread to finish current command buffer

    struct _Query_Result {
        byte_t ptr[16];
    };
    bool query(Query_Type type, _Query_Result* result);

    template <typename type_t>
    bool query(Query_Type type, type_t* out_result) {
        _Query_Result _res;
        if (!this->query(type, &_res)) {
            return false;
        }

        memcpy(out_result, _res.ptr, std::min(sizeof(type_t), sizeof(_res.ptr)));
        return true;
    }
    
    // TODO (2023-11-16): #unfinished #graphics #synchronization #concurrency
    // Implement a better way to map buffers. 
    // Maybe just send a callback with map_buffer which is invoked
    // on the render thread and afterwards the buffer is unmapped.
    // Would be more like "access_buffer".

    // Deprecated, don't use
    void map_buffer(Resource_Handle buffer_hnd, Buffer_Access_Mode access_Mode, const std::function<void(void*)>& result_callback);
    void unmap_buffer(Resource_Handle buffer_hnd);

    const Environment& get_environment() const;

    void swap_command_buffers();

    void set_target(Device_Context* target);
    Device_Context* get_current_target() const;

    //////////////////////////////////////////
    // Implemented per graphics API

    const Resource_Meta_Info& get_resource_meta(Resource_Handle hnd) const;
    Resource_State get_resource_state(Resource_Handle hnd) const;

    bool __internal_query(Query_Type type, _Query_Result* result);

    // Internal
    void __internal_init();
    void __internal_handle_command(Render_Command* header, void* data); 
    void __internal_shutdown();
    void __internal_on_context_change();
    void __internal_on_command_send(Render_Command* header, const void* data);

    void __enter_loop(std::mutex*);

    //////////////////////////////////////////



    // Helper functions

    void swap_render_buffers() {
        spec::submit::Swap_Render_Buffers spec;
        spec.dc = this->get_current_target();
        this->submit(spec);
    }

    void set_clear_color(const mz::color& clear_color, Resource_Handle render_texture = 0) {
        spec::submit::Set_Clear_Color spec;
		spec.clear_color = clear_color;
        spec.render_texture = render_texture;
		this->submit(spec);
    }

    Resource_Handle create_shader(const char* vert_src, const char* pixel_src) {
        spec::create::Shader shader_spec;
        shader_spec.vertex_source = vert_src;
        shader_spec.pixel_source = pixel_src;
        return this->create(shader_spec);
    }
    Resource_Handle create_buffer(Buffer_Type type, Buffer_Usage usage) {
        spec::create::Buffer spec;
        spec.buffer_type = type;
        spec.buffer_usage = usage;
        return this->create(spec);
    }
    Resource_Handle create_buffer_layout(const std::initializer_list<Buffer_Layout_Entry>& entries) {
        stallout::graphics::spec::create::Buffer_Layout spec(entries);
        return this->create(spec);
    }
    Resource_Handle create_texture(
            u32 width, u32 height, u32 channels, 
            Texture2D_Format input_format, 
            Data_Type type = DATA_TYPE_UBYTE, 
            Texture2D_Format internal_format = TEXTURE2D_FORMAT_RGBA, 
            Texture_Filter_Mode min_filter = TEXTURE_FILTER_MODE_LINEAR, 
            Texture_Filter_Mode mag_filter = TEXTURE_FILTER_MODE_NEAREST, 
            Mipmap_Mode mipmap = MIPMAP_MODE_LINEAR, 
            Texture_Wrap_Mode wrap_mode = TEXTURE_WRAP_MODE_CLAMP_TO_BORDER) {

        spec::create::Texture2D spec;
        spec.width = width;
        spec.height = height;
        spec.channels = channels;
        spec.input_format = input_format;
        spec.component_type = type;
        spec.internal_format = internal_format;
        spec.min_filter_mode = min_filter;
        spec.mag_filter_mode = mag_filter;
        spec.mipmap_mode = mipmap;
        spec.wrap_mode = wrap_mode;
        
        return this->create(spec);
    }
    Resource_Handle create_render_texture(
            u32 width, u32 height, u32 channels, 
            Texture2D_Format input_format, 
            Data_Type type = DATA_TYPE_UBYTE, 
            Texture2D_Format internal_format = TEXTURE2D_FORMAT_RGBA, 
            Texture_Filter_Mode min_filter = TEXTURE_FILTER_MODE_LINEAR, 
            Texture_Filter_Mode mag_filter = TEXTURE_FILTER_MODE_NEAREST, 
            Mipmap_Mode mipmap = MIPMAP_MODE_NONE, 
            Texture_Wrap_Mode wrap_mode = TEXTURE_WRAP_MODE_CLAMP_TO_BORDER) {

        spec::create::Render_Texture2D spec;
        spec.tex_spec.width = width;
        spec.tex_spec.height = height;
        spec.tex_spec.channels = channels;
        spec.tex_spec.input_format = input_format;
        spec.tex_spec.component_type = type;
        spec.tex_spec.internal_format = internal_format;
        spec.tex_spec.min_filter_mode = min_filter;
        spec.tex_spec.mag_filter_mode = mag_filter;
        spec.tex_spec.mipmap_mode = mipmap;
        spec.tex_spec.wrap_mode = wrap_mode;
        
        return this->create(spec);
    }

    void set_buffer(Resource_Handle hbuffer, void* data, size_t size) {
        this->set(hbuffer, RESOURCE_TYPE_BUFFER, data, size);
    }
    void append_to_buffer(Resource_Handle hbuffer, size_t offset, const void* data, size_t data_size) {
        if (data_size == 0) return;
        spec::append::Buffer spec;
        spec.offset = offset;
        spec.data_size = data_size;
        this->append(hbuffer, spec, data);
    }
    void set_texture2d(Resource_Handle htexture, void* data, size_t size) {
        this->set(htexture, RESOURCE_TYPE_TEXTURE2D, data, size);
    }
    void append_to_texture2d(Resource_Handle htexture, u32 xoffset, u32 yoffset, u32 width, u32 height, u32 channels, void* data, Texture2D_Format format, Data_Type type) {
        if (width + height == 0) return;
        spec::append::Texture2D spec;
        spec.xoffset = xoffset;
        spec.yoffset = yoffset;
        spec.width = width;
        spec.height = height;
        spec.format = format;
        spec.storage_type = type;
        spec.channels = channels;
        spec.data_size = spec.width * spec.height * spec.channels;
        this->append(htexture, spec, data);
    }
    
    void set_uniform_block(Resource_Handle hshader, Resource_Handle hubo, const char* block_name, size_t bind_index) {
        spec::submit::Bind_Shader_Uniform_Buffer spec;
        spec.shader_hnd = hshader;
		spec.buffer_hnd = hubo;
		spec.bind_index = bind_index;
		spec.block_name = block_name;

		this->submit(spec);
    }

    void set_texture_slot(size_t slot, Resource_Handle htexture) {
        spec::submit::Bind_Texture2D spec;
		spec.hnd_texture = htexture;
		spec.slot = slot;

		this->submit(spec);
    }

    void clear(Clear_Flags clear_flags, Resource_Handle render_texture = 0) {
        spec::submit::Clear clear_spec;
		clear_spec.clear_flags = clear_flags;
        clear_spec.render_texture = render_texture;
		this->submit(clear_spec);
    }

    void clear(Clear_Flags clear_flags, const mz::fcolor16& clear_color, Resource_Handle render_texture = 0) {
        this->set_clear_color(clear_color, render_texture);
        this->clear(clear_flags, render_texture);
    }

    void draw_indexed(Resource_Handle hlayout, Resource_Handle hvertex_buffer, Resource_Handle hindex_buffer, Resource_Handle hshader, size_t index_count, Data_Type indices_type = DATA_TYPE_UINT, size_t indices_offset = 0, Draw_Mode draw_mode = DRAW_MODE_TRIANGLES, Resource_Handle render_texture = 0) {
        spec::submit::Draw_Indexed spec;
        spec.layout = hlayout;
        spec.vbo = hvertex_buffer;
        spec.ibo = hindex_buffer;
        spec.shader = hshader;
        spec.index_data_type = indices_type;
        spec.draw_mode = draw_mode;
        spec.indices_offset = indices_offset;
        spec.index_count = index_count;
        spec.render_texture = render_texture;

        this->submit(spec);
    }

    template <typename type_t>
    void set_buffer(Resource_Handle hbuffer, type_t* data) {
        this->set_buffer(hbuffer, data, sizeof(type_t));
    }
    template <typename type_t>
    void append_to_buffer(Resource_Handle hbuffer, size_t offset, type_t* data) {
        this->append_to_buffer(hbuffer, offset, data, sizeof(type_t));
    }

    void set_blending(Blend_Equation eq, Blend_Func_Factor src_color_factor, Blend_Func_Factor dst_color_factor, Blend_Func_Factor src_alpha_factor, Blend_Func_Factor dst_alpha_factor) {
        spec::submit::Set_Blending spec;
        spec.equation = eq;
        spec.src_color_factor = src_color_factor;
        spec.dst_color_factor = dst_color_factor;
        spec.src_alpha_factor = src_alpha_factor;
        spec.dst_alpha_factor = dst_alpha_factor;

        this->submit(spec);
    }
    void set_blending(Blend_Equation eq, Blend_Func_Factor src_color_factor, Blend_Func_Factor dst_color_factor) {
        this->set_blending(eq, src_color_factor, dst_color_factor, src_color_factor, dst_color_factor);
    }
    void disable_blending() {
        spec::submit::Set_Blending spec;
        spec.equation = BLEND_EQUATION_NONE;
        this->submit(spec);
    }

    void enable(Renderer_Setting_Flags flags, Resource_Handle render_texture = 0) {
        spec::submit::Toggle spec;
        spec.enabled = true;
        spec.settings = flags;
        spec.render_texture = render_texture;
        this->submit(spec);
    }
    void disable(Renderer_Setting_Flags flags, Resource_Handle render_texture = 0) {
        spec::submit::Toggle spec;
        spec.enabled = false;
        spec.settings = flags;
        spec.render_texture = render_texture;
        this->submit(spec);
    }

    void set_polygon_mode(Polygon_Face face, Polygon_Mode mode) {
        spec::submit::Set_Polygon_Mode spec;
        spec.face = face;
        spec.mode = mode;
        this->submit(spec);
    }

    void set_viewport(mz::s32vec2 pos, mz::s32vec2 size) {
        spec::submit::Set_Viewport spec;
        spec.pos = pos;
        spec.size = size;
        this->submit(spec);
    }

    void set_scissor_box(mz::irect rect) {
        spec::submit::Set_Scissor_Box spec;
        spec.rect = rect;
        this->submit(spec);
    }

    void generate_mipmap(Resource_Handle htexture) {
        spec::submit::Generate_Mipmap spec;
        spec.htexture = htexture;

        this->submit(spec);
    }
};

NS_END(graphics);
NS_END(stallout);
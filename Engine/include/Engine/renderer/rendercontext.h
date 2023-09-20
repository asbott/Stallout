#pragma once

#include "Engine/memory.h"
#include "Engine/timing.h"
#include "Engine/utils.h"

#include "os/graphics.h"

#include "Engine/renderer/rendercommands.h"

NS_BEGIN(engine);
NS_BEGIN(renderer);








struct Mapping_Promise;

struct ST_API Render_Context {
    
    struct Environment {
        const char* vendor, *hardware, *driver, *version, *shading_version;
        u32 version_major, version_minor;
    };

    struct _Mapping_Promise;

    utils::Double_Buffered_Thread _render_thread;
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

    //////////////////////////////////////////
    // Implemented per graphics API

    const Resource_Meta_Info& get_resource_meta(Resource_Handle hnd) const;
    Resource_State get_resource_state(Resource_Handle hnd) const;

    // Internal
    void __internal_init();
    void __internal_render(); 
    void __internal_shutdown();

    void __enter_loop(std::mutex*);

    //////////////////////////////////////////



    // Renderer API

    os::Window* get_window() const {
        return this->_os_window;
    }

    void set_clear_color(const mz::color& clear_color) {
        spec::submit::Set_Clear_Color spec;
		spec.clear_color = clear_color;
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
        engine::renderer::spec::create::Buffer_Layout spec(entries);
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

    void set_buffer(Resource_Handle hbuffer, void* data, size_t size) {
        this->set(hbuffer, RESOURCE_TYPE_BUFFER, data, size);
    }
    void set_texture2d(Resource_Handle htexture, void* data, size_t size) {
        this->set(htexture, RESOURCE_TYPE_TEXTURE2D, data, size);
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

    void clear(Clear_Flags clear_flags) {
        spec::submit::Clear clear_spec;
		clear_spec.clear_flags = clear_flags;
		this->submit(clear_spec);
    }

    void draw_indexed(Resource_Handle hlayout, Resource_Handle hvertex_buffer, Resource_Handle hindex_buffer, Resource_Handle hshader, Data_Type indices_type = DATA_TYPE_UINT, Draw_Mode draw_mode = DRAW_MODE_TRIANGLES) {
        spec::submit::Draw_Indexed spec;
        spec.layout = hlayout;
        spec.vbo = hvertex_buffer;
        spec.ibo = hindex_buffer;
        spec.shader = hshader;
        spec.index_data_type = indices_type;
        spec.draw_mode = draw_mode;

        this->submit(spec);
    }

    template <typename type_t>
    void set_buffer(Resource_Handle hbuffer, type_t* data) {
        this->set_buffer(hbuffer, data, sizeof(type_t));
    }
};

NS_END(renderer);
NS_END(engine);
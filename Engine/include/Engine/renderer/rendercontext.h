#pragma once

#include "Engine/memory.h"
#include "Engine/timing.h"
#include "Engine/utils.h"

#include "os/graphics.h"

#include "Engine/renderer/rendercommands.h"

#include <mz_vector.hpp>

NS_BEGIN(engine);
NS_BEGIN(renderer);




struct Render_Context;
struct Render_Window;

typedef std::function<bool(Render_Window*, os::Window_Event_Type, void* param, void* userdata)> render_window_callback_t;

// Frontend for os::Window. s window calls
// to the render thread in associated context
struct ST_API Render_Window {
    struct Window_Event_Callback {
        render_window_callback_t fn;
        void* user_data = NULL;
    };
    // This may be null while waiting for render thread.
    // It should be fine as long as all calls to _backend
    // are on the render thread. Void* because it's not
    // meant to be used directly here only passed as a
    // handle
    os::Window* _backend; 
    utils::Double_Buffered_Thread* _render_thread;
    Render_Context *const _context;
    engine::Hash_Map<os::Window_Event_Type, engine::Array<Window_Event_Callback>> _specific_event_callbacks; 
    engine::Array<Window_Event_Callback> _event_callbacks;
    engine::Array<Render_Window*> _children;
    Render_Window* _parent;
    bool _dead = false;
    os::graphics::OS_Graphics_Context* _os_context;

    // This is state which needs to be queried from the
    // backend which is done in __query_backend() on the
    // render thread. It is readonly. It's only possible
    // to change state in the backend with functions

    Render_Window(Render_Context* context, utils::Double_Buffered_Thread* render_thread, os::Window* backend);
    Render_Window(Render_Window* parent, os::Window* backend);
    ~Render_Window();

    void add_event_callback(render_window_callback_t callback, void* userdata = NULL);
    void add_event_callback(os::Window_Event_Type event, render_window_callback_t callback, void* userdata = NULL);

    void set_parent(Render_Window* new_parent);
    Render_Window* add_child(os::Window_Init_Spec spec);

    void dispatch_event(u32 type, void* param);

    bool exit_flag() const;

    void poll_events();
    void swap_buffers();

    void set_position(const mz::s32vec2& pos);
    mz::s32vec2 get_position();
    void set_size(const mz::s32vec2& sz);
    mz::s32vec2 get_size();

    mz::s32vec2 screen_to_client(mz::s32vec2 screen) const;
    mz::s32vec2 client_to_screen(mz::s32vec2 client) const;

    bool has_captured_mouse() const;
    void capture_mouse();
    void release_mouse();

    bool is_input_down(os::Input_Code code) const; 

    void set_visibility(bool visible);
    void set_focus(bool focused = true);
    void set_title(const char* title);
    void set_alpha(float alpha);

    bool is_visible() const;
    bool is_focused() const;
    bool is_minimized() const;
    bool is_hovered() const;
    void* get_monitor() const;

    void _init_backend(os::Window* backend);
    void _send(std::function<void(os::Window*)>);
    void __query_backend(os::Window* backend);
};

struct Promise {

    bool done = false;
    std::mutex mtx;
    std::condition_variable cond;

    void wait() {
        std::unique_lock lock(mtx);
        cond.wait(lock, [&]() { return done; });
    }
};
typedef std::function<void()> priority_task_t;

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

    struct Priority_Task {
        priority_task_t task;
        Promise* promise;
    };
    Queue<Priority_Task> _priority_queue;
    std::mutex _priority_mutex;
    
    Render_Window* _frontend_window; // For use on main thread, send command to render thread
    os::Window* __backend_window; // For use on render thread

    Render_Window* _current_target;

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

    void swap_command_buffers();

    void set_target(Render_Window* target);
    Render_Window* get_current_target() const;

    void do_now(priority_task_t task) {
        Promise promise;
        {
            std::lock_guard lock(_priority_mutex);
            _priority_queue.push({ task, &promise });
        }
        // TODO: (2023-09-05) #unfinished #hacky
        // Should just rewrite double buffered thread
        // to deal with priority commands. Maybe completely
        // refactor to just wrap around a Worker_Thread
        _render_thread._read_condition.notify_one();
        promise.wait();
    }

    //////////////////////////////////////////
    // Implemented per graphics API

    const Resource_Meta_Info& get_resource_meta(Resource_Handle hnd) const;
    Resource_State get_resource_state(Resource_Handle hnd) const;

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

    // Internal
    void __internal_init();
    void __internal_handle_command(Render_Command* header, void* data); 
    void __internal_shutdown();

    void __enter_loop(std::mutex*);

    //////////////////////////////////////////



    // Renderer API

    Render_Window* get_window() const {
        return this->_frontend_window;
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

    void draw_indexed(Resource_Handle hlayout, Resource_Handle hvertex_buffer, Resource_Handle hindex_buffer, Resource_Handle hshader, size_t index_count, Data_Type indices_type = DATA_TYPE_UINT, size_t indices_offset = 0, Draw_Mode draw_mode = DRAW_MODE_TRIANGLES) {
        spec::submit::Draw_Indexed spec;
        spec.layout = hlayout;
        spec.vbo = hvertex_buffer;
        spec.ibo = hindex_buffer;
        spec.shader = hshader;
        spec.index_data_type = indices_type;
        spec.draw_mode = draw_mode;
        spec.indices_offset = indices_offset;
        spec.index_count = index_count;

        this->submit(spec);
    }

    template <typename type_t>
    void set_buffer(Resource_Handle hbuffer, type_t* data) {
        this->set_buffer(hbuffer, data, sizeof(type_t));
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

    void enable(Renderer_Setting_Flags flags) {
        spec::submit::Toggle spec;
        spec.enabled = true;
        spec.settings = flags;
        this->submit(spec);
    }
    void disable(Renderer_Setting_Flags flags) {
        spec::submit::Toggle spec;
        spec.enabled = false;
        spec.settings = flags;
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
};

NS_END(renderer);
NS_END(engine);
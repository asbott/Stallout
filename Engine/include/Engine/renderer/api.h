#pragma once

#include "Engine/renderer/rendercontext.h"

NS_BEGIN(engine);
NS_BEGIN(renderer);

// Thin wrapper around Render_Context with functions for commands
struct Renderer_API {
    Render_Context* renderer;

    Renderer_API(Render_Context* renderer)
        : renderer(renderer) {}

    void wait_ready() const {
        return renderer->wait_ready();
    }

    os::Window* get_window() const {
        return renderer->_os_window;
    }

    void swap_buffers() {
        renderer->swap_buffers();
    }

    void set_clear_color(const mz::color& clear_color) {
        spec::submit::Set_Clear_Color spec;
		spec.clear_color = clear_color;
		renderer->submit(spec);
    }

    Resource_Handle create_shader(const char* vert_src, const char* pixel_src) {
        spec::create::Shader shader_spec;
        shader_spec.vertex_source = vert_src;
        shader_spec.pixel_source = pixel_src;
        return renderer->create(shader_spec);
    }
    Resource_Handle create_buffer(Buffer_Type type, Buffer_Usage usage) {
        spec::create::Buffer spec;
        spec.buffer_type = type;
        spec.buffer_usage = usage;
        return renderer->create(spec);
    }
    Resource_Handle create_buffer_layout(const std::initializer_list<Buffer_Layout_Entry>& entries) {
        engine::renderer::spec::create::Buffer_Layout spec(entries);
        return renderer->create(spec);
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
        
        return renderer->create(spec);
    }

    void set_buffer(Resource_Handle hbuffer, void* data, size_t size) {
        renderer->set(hbuffer, RESOURCE_TYPE_BUFFER, data, size);
    }
    void set_texture2d(Resource_Handle htexture, void* data, size_t size) {
        renderer->set(htexture, RESOURCE_TYPE_TEXTURE2D, data, size);
    }
    
    void set_uniform_block(Resource_Handle hshader, Resource_Handle hubo, const char* block_name, size_t bind_index) {
        spec::submit::Bind_Shader_Uniform_Buffer spec;
        spec.shader_hnd = hshader;
		spec.buffer_hnd = hubo;
		spec.bind_index = bind_index;
		spec.block_name = block_name;

		renderer->submit(spec);
    }

    void map_buffer(Resource_Handle buffer_hnd, Buffer_Access_Mode access_mode, const std::function<void(void*)>& result_callback) {
        renderer->map_buffer(buffer_hnd, access_mode, result_callback);
    }
    void unmap_buffer(Resource_Handle buffer_hnd) {
        renderer->unmap_buffer(buffer_hnd);
    }

    void set_texture_slot(size_t slot, Resource_Handle htexture) {
        spec::submit::Bind_Texture2D spec;
		spec.hnd_texture = htexture;
		spec.slot = slot;

		renderer->submit(spec);
    }

    void clear(Clear_Flags clear_flags) {
        spec::submit::Clear clear_spec;
		clear_spec.clear_flags = clear_flags;
		renderer->submit(clear_spec);
    }

    void draw_indexed(Resource_Handle hlayout, Resource_Handle hvertex_buffer, Resource_Handle hindex_buffer, Resource_Handle hshader, Data_Type indices_type = DATA_TYPE_UINT, Draw_Mode draw_mode = DRAW_MODE_TRIANGLES) {
        spec::submit::Draw_Indexed spec;
        spec.layout = hlayout;
        spec.vbo = hvertex_buffer;
        spec.ibo = hindex_buffer;
        spec.shader = hshader;
        spec.index_data_type = indices_type;
        spec.draw_mode = draw_mode;

        renderer->submit(spec);
    }

    void destroy(Resource_Handle hresource) {
        renderer->destroy(hresource);
    }

    template <typename type_t>
    void set_buffer(Resource_Handle hbuffer, type_t* data) {
        this->set_buffer(hbuffer, data, sizeof(type_t));
    }


    
};

NS_END(renderer);
NS_END(engine);
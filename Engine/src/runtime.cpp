#include "pch.h"

#include "Engine/runtime.h"

#include "Engine/logger.h"
#include "Engine/memory.h"
#include "Engine/containers.h"
#include "Engine/timing.h"
#include "Engine/renderer/rendercontext.h"
#include "Engine/utils.h"

#include "os/modules.h"
#include "os/io.h"
#include "os/oswindow.h"

#if _ST_RUN_TESTS
    #include "Engine/debug/tests.h"
#endif

NS_BEGIN(engine);
NS_BEGIN(runtime);

// Runtime config
std::ofstream log_stream;
f64 log_fps_intervall = 3; // seconds

// Runtime state
s32 runtime_status = 0;

// Runtime resources
Array<os::Module*> modules;
engine::renderer::Render_Context* renderer;

void main_loop();

void start() {

    // Initialize log

    log_stream.open("output");
	init_logger(log_stream);
	set_logger_level(spdlog::level::debug);

    // Initialize allocators

    Global_Allocator::init(1024 * 1000 * 1000); // Reserve 1GB

	// Load & initialize modules

	path_str_t modules_path = "";
	sprintf(modules_path, "%s/module_list", os::io::get_exe_dir().str);

	Array<New_String> module_names;

	auto status = os::io::read_all_lines(modules_path, &module_names);

	if (status != IO_STATUS_OK) {
		runtime_status |= RUNTIME_STATUS_ERROR;
		return;
	}

	for (const auto& module_name : module_names) {
		path_str_t module_path = "";
		sprintf(module_path, "%s/%s.dll", os::io::get_exe_dir().str, module_name.str);
		modules.push_back(ST_NEW(os::Module, module_path));
		if (modules.back()->_status != os::MODULE_STATUS_OK) {
			log_error("Failed loading module '{}': {}", module_name.str, os::Module_Status_string(modules.back()->_status));
		} else {
			modules.back()->init();
			log_trace("Initialized module '{}'", module_name.str);
		}
	}

	
	// Run tests after initialization
    #ifdef _ST_RUN_TESTS
	run_tests();
	#endif

	// Initialize renderer
	renderer = ST_NEW(engine::renderer::Render_Context, 1024 * 1000 * 100);

	renderer->wait_ready();
	auto env = renderer->get_environment();
	log_info("Renderer is ready!\nVendor: {}\nHardware: {}\nDrivers: {}\nVersion: {}\nShading Lang Version: {}",
	         env.vendor, env.hardware, env.driver, env.version, env.shading_version);

	runtime_status |= RUNTIME_STATUS_RUNNING;

	main_loop();

	// Deinitialize

	for (os::Module* module : modules) {
		if(module->deinit) module->deinit();
		ST_DELETE(module);
	}
	modules.clear();

	ST_DELETE(renderer);
}

void shutdown() {
    runtime_status &= ~RUNTIME_STATUS_RUNNING;
}

void main_loop() {
	
	Timer frame_timer;
	Timer log_fps_timer;
	f64 min_frametime = 1.0 / 1000; // Too high fps causes opengl state to crash..........	

	{
		engine::renderer::spec::submit::Set_Clear_Color spec;
		spec.clear_color = mz::color16(1.0f, 0.0f, 0.0f, 1.0f);
		renderer->submit(spec);
	}

	// Create shader
	engine::renderer::spec::create::Shader shader_spec;
	shader_spec.vertex_source = R"(
		layout (location = 0) in vec3 pos;
		layout (location = 1) in vec2 tex_coord_vert;

		out vec2 tex_coord_frag;

		void main()
		{
			gl_Position = vec4(pos, 1.0);
			tex_coord_frag = vec2(tex_coord_vert.x, 1.0 - tex_coord_vert.y);
		}
	)";
	shader_spec.pixel_source = R"(
		out vec4 color_result;

		in vec2 tex_coord_frag;

		layout(std140) uniform TextureBlock
		{
			int texture1;
		};

		void main()
		{
			//color_result = vec4(0.0, float(texture1), 0.0, 1.0);
			color_result = texture(textures[texture1], tex_coord_frag);
		}
	)";
	engine::renderer::Resource_Handle hshader = renderer->create(shader_spec);

	// Create & populate  ubo, vbo, ibo, layout
	float vertices[] = {
        // positions         // texture coords
         0.5f,  0.5f, 0.0f,  1.0f, 1.0f,
         0.5f, -0.5f, 0.0f,  1.0f, 0.0f,
        -0.5f, -0.5f, 0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f, 0.0f,  0.0f, 1.0f
    };

    unsigned int indices[] = {
        0, 1, 3,
        1, 2, 3
    };

	engine::renderer::spec::create::Buffer ubo_spec, vbo_spec, ibo_spec;

	ubo_spec.buffer_type = engine::renderer::BUFFER_TYPE_UNIFORM_BUFFER;
	ubo_spec.buffer_usage = engine::renderer::BUFFER_USAGE_DYNAMIC_DRAW;

	vbo_spec.buffer_type = engine::renderer::BUFFER_TYPE_ARRAY_BUFFER;
	vbo_spec.buffer_usage = engine::renderer::BUFFER_USAGE_STATIC_DRAW;

	ibo_spec.buffer_type = engine::renderer::BUFFER_TYPE_ELEMENT_ARRAY_BUFFER;
	ibo_spec.buffer_usage = engine::renderer::BUFFER_USAGE_STATIC_DRAW;

	engine::renderer::spec::create::Buffer_Layout layout_spec(
		{
			{ 3, engine::renderer::DATA_TYPE_FLOAT }, // Position, 3 floats
			{ 2, engine::renderer::DATA_TYPE_FLOAT }, // Tex coord, 2 floats
		}
	);
	
	engine::renderer::Resource_Handle hubo 	  = renderer->create(ubo_spec);
	engine::renderer::Resource_Handle hvbo 	  = renderer->create(vbo_spec);
	engine::renderer::Resource_Handle hibo    = renderer->create(ibo_spec);
	engine::renderer::Resource_Handle hlayout = renderer->create(layout_spec);

	
	renderer->set(hubo, engine::renderer::RESOURCE_TYPE_BUFFER, 0, sizeof(int)); // Initialize emptys data
	renderer->set(hvbo, engine::renderer::RESOURCE_TYPE_BUFFER, &vertices, sizeof(vertices));
	renderer->set(hibo, engine::renderer::RESOURCE_TYPE_BUFFER, &indices, sizeof(indices));

	{
		engine::renderer::spec::submit::Bind_Shader_Uniform_Buffer spec;
		spec.shader_hnd = hshader;
		spec.buffer_hnd = hubo;
		spec.bind_index = 0;
		spec.block_name = "TextureBlock";

		renderer->submit(spec);
	}

	int texture_sampler = 0; // We will bind the texture to slot 0
	renderer->map_buffer(hubo, engine::renderer::BUFFER_ACCESS_MODE_WRITEONLY, [&](void* buffer) {

		memcpy(buffer, &texture_sampler, sizeof(texture_sampler));

		renderer->unmap_buffer(hubo);
	});

	int width, height, channels;
	void* texture_data = engine::utils::load_image_from_file("test.jpg", &width, &height, &channels, 4);

	if (!texture_data) {
		log_error("Failed loading texture");
		return;
	}

	engine::renderer::Texture2D_Format input_format = engine::renderer::TEXTURE2D_FORMAT_RGBA;

	engine::renderer::spec::create::Texture2D texture_spec;
	texture_spec.width = width;
	texture_spec.height = height;
	texture_spec.channels = 4;
	texture_spec.component_type = engine::renderer::DATA_TYPE_UBYTE;
	texture_spec.input_format = input_format;
	texture_spec.internal_format = engine::renderer::TEXTURE2D_FORMAT_RGBA;

	engine::renderer::Resource_Handle htexture = renderer->create(texture_spec);

	renderer->set(htexture, engine::renderer::RESOURCE_TYPE_TEXTURE2D, texture_data, width * height * 4);
	
	{
		engine::renderer::spec::submit::Bind_Texture2D spec;
		spec.hnd_texture = htexture;
		spec.slot = 0;

		renderer->submit(spec);
	}

	engine::utils::free_image(texture_data);

    while(runtime_status & RUNTIME_STATUS_RUNNING) {

		auto last_frame_time = frame_timer.record();
        frame_timer.reset();

		for (os::Module* module : modules) {
			if (module->_status == os::MODULE_STATUS_OK) {
				module->update(last_frame_time);
			}
		}

		if (log_fps_timer.record().get_seconds() >= log_fps_intervall) {
			//log_info("FPS: {:0.0f}\nFrametime: {:0.4f}ms", 1 / last_frame_time.get_seconds(), last_frame_time.get_milliseconds());
			log_fps_timer.reset();
		}

		engine::renderer::spec::submit::Clear clear_spec;
		clear_spec.clear_flags = engine::renderer::CLEAR_FLAG_COLOR;
		renderer->submit(clear_spec);

		{
			engine::renderer::spec::submit::Draw_Indexed spec;
			spec.draw_mode = engine::renderer::DRAW_MODE_TRIANGLES;
			spec.ibo = hibo;
			spec.vbo = hvbo;
			spec.layout = hlayout;
			spec.shader = hshader;
			spec.index_data_type = engine::renderer::DATA_TYPE_UINT;

			renderer->submit(spec);
		}

		

		if (renderer->_os_window->exit_flag()) {
			runtime_status &= ~RUNTIME_STATUS_RUNNING;
		}

		renderer->swap_buffers();

		while (frame_timer.record().get_seconds() < min_frametime);
    }

	renderer->destroy(hshader);
	renderer->destroy(hubo);
	renderer->destroy(hvbo);
	renderer->destroy(hibo);
	renderer->destroy(hlayout);
	renderer->destroy(htexture);

	log_info("App loop exited as expected");
}

s32 get_status_flags() {
	return runtime_status;
}

NS_END(runtime);
NS_END(engine);
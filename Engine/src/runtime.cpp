#include "pch.h"

#include "Engine/runtime.h"

#include "Engine/logger.h"
#include "Engine/memory.h"
#include "Engine/containers.h"
#include "Engine/timing.h"
#include "Engine/renderer/api.h"
#include "Engine/utils.h"
#include "Engine/audio/audiocontext.h"

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

engine::audio::Audio_Context* audio_engine;
engine::audio::audio_clip_t short_clip;
engine::audio::Audio_Player* short_player = NULL;
engine::audio::audio_clip_t long_clip;
engine::audio::Audio_Player* long_player = NULL;

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

	audio_engine = ST_NEW(engine::audio::Audio_Context);

	short_clip = audio_engine->create_clip_from_file("test.wav");
	short_player = audio_engine->create_player();

	long_clip = audio_engine->create_clip_from_file("forest.mp3");
	long_player = audio_engine->create_player();

	runtime_status |= RUNTIME_STATUS_RUNNING;

	short_player->looping = true;
	long_player->looping = true;

	short_player->play(short_clip);
	long_player->play(long_clip);

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

	renderer->set_clear_color(mz::color16(1.0f, 0.0f, 0.0f, 1.0f));

	// Create shader
	const char* vert_src = R"(
		layout (location = 0) in vec3 pos;
		layout (location = 1) in vec2 tex_coord_vert;

		out vec2 tex_coord_frag;

		void main()
		{
			gl_Position = vec4(pos, 1.0);
			tex_coord_frag = vec2(tex_coord_vert.x, 1.0 - tex_coord_vert.y);
		}
	)";
	const char* pixel_src = R"(
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
	auto hshader = renderer->create_shader(vert_src, pixel_src);

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
	
	auto hubo 	 = renderer->create_buffer(engine::renderer::BUFFER_TYPE_UNIFORM_BUFFER, engine::renderer::BUFFER_USAGE_DYNAMIC_DRAW);
	auto hvbo 	 = renderer->create_buffer(engine::renderer::BUFFER_TYPE_ARRAY_BUFFER, engine::renderer::BUFFER_USAGE_STATIC_DRAW);
	auto hibo    = renderer->create_buffer(engine::renderer::BUFFER_TYPE_ELEMENT_ARRAY_BUFFER, engine::renderer::BUFFER_USAGE_STATIC_DRAW);
	auto hlayout = renderer->create_buffer_layout({
		{ 3, engine::renderer::DATA_TYPE_FLOAT }, // Position, 3 floats
		{ 2, engine::renderer::DATA_TYPE_FLOAT }, // Tex coord, 2 floats
	});

	renderer->set_buffer(hubo, nullptr, sizeof(int));
	renderer->set_buffer(hvbo, &vertices);
	renderer->set_buffer(hibo, &indices);

	renderer->set_uniform_block(hshader, hubo, "TextureBlock", 0);

	int texture_sampler = 0; // We will bind the texture to slot 0
	renderer->map_buffer(hubo, engine::renderer::BUFFER_ACCESS_MODE_WRITEONLY, [&](void* buffer) {
		memcpy(buffer, &texture_sampler, sizeof(texture_sampler));

		renderer->unmap_buffer(hubo);
	});

	int width, height, channels;
	void* texture_data = engine::utils::load_image_from_file("test.png", &width, &height, &channels, 4);

	if (!texture_data) {
		log_error("Failed loading texture");
		return;
	}

	auto htexture = renderer->create_texture(width, height, channels, engine::renderer::TEXTURE2D_FORMAT_RGBA);
	renderer->set_texture2d(htexture, texture_data, width * height * 4);
	renderer->set_texture_slot(0, htexture);

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

		if (short_player->get_timer().record().get_seconds() >= 1.0) {
			short_player->stop();
		}

		renderer->clear(engine::renderer::CLEAR_FLAG_COLOR);

		renderer->draw_indexed(hlayout, hvbo, hibo, hshader);

		if (renderer->get_window()->exit_flag()) {
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
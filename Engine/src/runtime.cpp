#include "pch.h"

#include "Engine/runtime.h"

#include "Engine/logger.h"
#include "Engine/memory.h"
#include "Engine/containers.h"
#include "Engine/timing.h"
#include "Engine/utils.h" 
#include "Engine/audio/audiocontext.h"
#include "Engine/renderer/rendercontext.h"
#include "Engine/renderer/imgui_renderer.h"

#include "os/modules.h"
#include "os/io.h"
#include "os/oswindow.h"

#include <mz_matrix.hpp>
#include <mz_algorithms.hpp>

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

void game_loop();

void start() {

    // Initialize log

    log_stream.open("output");
	init_logger(log_stream);
	set_logger_level(spdlog::level::debug);

    // Initialize allocators
    Global_Allocator::init(1024 * 1000 * 1000); // Reserve 1GB

	#ifdef _ST_RUN_TESTS
	run_tests();
	#endif

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
		auto exe_dir = os::io::get_exe_dir();
		sprintf(module_path, "%s/%s.dll", exe_dir.str, module_name.str);
		modules.push_back(stnew (os::Module)(module_path));
		if (modules.back()->_status != os::MODULE_STATUS_OK) {
			log_error("Failed loading module '{}': {}", module_name.str, os::Module_Status_string(modules.back()->_status));
		} else {
			auto init_result = modules.back()->init();
			(void)init_result;
			ST_DEBUG_ASSERT(init_result == 0, "Module '{}' signalled failed");
			log_trace("Initialized module '{}'", module_name.str);
		}
	}
	runtime_status |= RUNTIME_STATUS_RUNNING;

	game_loop();

	// Deinitialize

	for (os::Module* module : modules) {
		if(module->deinit) module->deinit();
		ST_DELETE(module);
	}
	modules.clear();
}

void shutdown() {
    runtime_status &= ~RUNTIME_STATUS_RUNNING;
}

void game_loop() {
	
	Timer frame_timer;
	Timer log_fps_timer;
	f64 min_frametime = 1.0 / 1000; // Too high fps causes opengl state to crash..........	

    while(runtime_status & RUNTIME_STATUS_RUNNING) {

		auto last_frame_time = frame_timer.record();
        frame_timer.reset();

		if (log_fps_timer.record().get_seconds() >= log_fps_intervall) {
			//log_info("FPS: {:0.0f}\nFrametime: {:0.4f}ms", 1 / last_frame_time.get_seconds(), last_frame_time.get_milliseconds());
			
			log_fps_timer.reset();
		}

		for (os::Module* module : modules) {
			if (module->_status == os::MODULE_STATUS_OK) {
				if (module->update(last_frame_time) != 0) {
					runtime_status &= ~RUNTIME_STATUS_RUNNING;
				}
			}
		}

		while (frame_timer.record().get_seconds() < min_frametime);
    }

	log_info("App loop exited as expected");
}

s32 get_status_flags() {
	return runtime_status;
}

NS_END(runtime);
NS_END(engine);
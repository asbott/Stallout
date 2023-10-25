#include "pch.h"

#include "Engine/runtime.h"

#include "Engine/logger.h"
#include "Engine/memory.h"
#include "Engine/containers.h"
#include "Engine/timing.h"
#include "Engine/utils.h" 
#include "Engine/audio/audiocontext.h"
#include "Engine/graphics/graphics_driver.h"
#include "Engine/graphics/imgui_renderer.h"

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
    Global_Allocator::init(1024 * 1000 * 100); 

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
	f64 min_frametime = 1.0 / 100000; // Too high fps causes opengl state to crash..........	

    while(runtime_status & RUNTIME_STATUS_RUNNING) {

		tm_func();

		auto last_frame_time = frame_timer.record();
        frame_timer.reset();

		if (log_fps_timer.record().get_seconds() >= log_fps_intervall) {
			//log_info("FPS: {:0.0f}\nFrametime: {:0.4f}ms", 1 / last_frame_time.get_seconds(), last_frame_time.get_milliseconds());
			
			log_fps_timer.reset();
		}

		if (os::is_input_down(os::INPUT_CODE_LSHIFT) && os::is_input_down(os::INPUT_CODE_P) && os::is_input_down(os::INPUT_CODE_M) && os::is_input_down(os::INPUT_CODE_C)) {
			Global_Allocator::reset_stats();
		}

		if (os::is_input_down(os::INPUT_CODE_LSHIFT) && os::is_input_down(os::INPUT_CODE_P) && os::is_input_down(os::INPUT_CODE_M)) {
			auto& stats = Global_Allocator::get_stats();
#define PC(x) ((x) * 100.0)
#define KB(x) ((x) / 1024.0)
#define MCS(x) ((x) * 1000.0)
			log_info(
				R"(MEMORY STATS
Preallocated: {} kb
In use (tracked): {:.1f} kb

Total allocated: {:.1f} kb
Total allocations: {}
Total time: {:.4f}

Avg Allocation Size: {:.1f}kb
Avg Allocation Time: {:.4f}mcs

Usage
Cached: {:.1f} kb
Fast: {:.1f} kb
Small: {:.1f} kb
Common: {:.1f} kb
Heap: {:.1f} kb
Growing: {:.1f} kb

Avg Time
Cached: {:.4f}mcs
Fast: {:.4f}mcs
Small: {:.4f}mcs
Common: {:.4f}mcs
Heap: {:.4f}mcs
Growing: {:.4f}mcs

Total allocated
Cached: {:.1f} kb
Fast: {:.1f} kb
Small: {:.1f} kb
Common: {:.1f} kb
Heap: {:.1f} kb
Growing: {:.1f} kb

Allocated distribution
Cached: {:.2f}%
Fast: {:.2f}%
Small: {:.2f}%
Common: {:.2f}%
Heap: {:.2f}%
Growing: {:.2f}%

Num Allocations distribution
Cached: {:.2f}%
Fast: {:.2f}%
Small: {:.2f}%
Common: {:.2f}%
Heap: {:.2f}%
Growing: {:.2f}%

Time distribution
Cached: {:.2f}%
Fast: {:.2f}%
Small: {:.2f}%
Common: {:.2f}%
Heap: {:.2f}%
Growing: {:.2f}%
)",  
KB(stats.preallocated), 
KB(stats.in_use()), 
KB(stats.total_amount()), 
stats.total_allocations(), 
stats.total_time(), 

KB(stats.total_amount() / std::max<size_t>(stats.total_allocations(), 1)),
MCS(stats.total_time() / std::max<size_t>(stats.total_allocations(), 1)),

KB(stats.cache_usage()),
KB(stats.fast_usage()),
KB(stats.small_usage()),
KB(stats.common_usage()),
KB(stats.heap_usage()),
KB(stats.growing_usage()),

MCS(stats.cache_avg_time()),
MCS(stats.fast_avg_time()),
MCS(stats.small_avg_time()),
MCS(stats.common_avg_time()),
MCS(stats.heap_avg_time()),
MCS(stats.growing_avg_time()),

KB(stats.cache_amount),
KB(stats.fast_amount),
KB(stats.small_amount),
KB(stats.common_amount),
KB(stats.heap_amount),
KB(stats.growing_amount),

PC(stats.cache_amount_ratio()),
PC(stats.fast_amount_ratio()),
PC(stats.small_amount_ratio()),
PC(stats.common_amount_ratio()),
PC(stats.heap_amount_ratio()),
PC(stats.growing_amount_ratio()),

PC(stats.cache_allocations_ratio()),
PC(stats.fast_allocations_ratio()),
PC(stats.small_allocations_ratio()),
PC(stats.common_allocations_ratio()),
PC(stats.heap_allocations_ratio()),
PC(stats.growing_allocations_ratio()),

PC(stats.cache_time_ratio()),
PC(stats.fast_time_ratio()),
PC(stats.small_time_ratio()),
PC(stats.common_time_ratio()),
PC(stats.heap_time_ratio()),
PC(stats.growing_time_ratio())

);
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
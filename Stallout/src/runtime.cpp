#include "pch.h"

#include "Stallout/runtime.h"

#include "Stallout/logger.h"
#include "Stallout/containers.h"
#include "Stallout/timing.h"
#include "Stallout/gameutils.h"
#include "Stallout/prototyping.h"

#include "os/modules.h"
#include "os/io.h"
#include "os/oswindow.h"

#include <mz_matrix.hpp>
#include <mz_algorithms.hpp>

#if _ST_RUN_TESTS
    #include "Stallout/debug/tests.h"
#endif

NS_BEGIN(stallout);
NS_BEGIN(runtime);

// Runtime config
std::ofstream log_stream;
f64 log_fps_intervall = 3; // seconds

// Runtime state
s32 runtime_status_flags = 0;
s32 runtime_status_code = 0;

f32 max_fps = 480.f;

// Runtime resources
Array<os::Module*> modules;

s32 get_status_code() {
	return runtime_status_code;
}

void game_loop();

void init() {
	// Initialize log
    log_stream.open("output");
	init_log_system(log_stream);
	set_logger_level(ST_MODULE_NAME, spdlog::level::debug);
    // Initialize allocators
    Global_Allocator::init(1024 * 1000 * 100); 
}

void start() {

	#ifdef _ST_RUN_TESTS
	run_tests();
	#endif

	// Load & initialize modules

	path_str_t modules_path = "";
	// TODO
	sprintf(modules_path, "%s/module_list", os::io::get_exe_dir().str);

	Array<String> module_names;

	auto status = os::io::read_all_lines(modules_path, &module_names);

	if (status != IO_STATUS_OK) {
		log_critical("Failed reading modules in '{}'", modules_path);
		runtime_status_flags |= RUNTIME_STATUS_ERROR;
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
			if (modules.back()->init) {
				auto init_result = modules.back()->init();
				if (init_result != 0) {
					runtime_status_code = init_result;
					if (init_result < 0) runtime_status_flags |= RUNTIME_STATUS_ERROR;
					return;
				}
			}
			
			log_trace("Initialized module '{}'", module_name.str);
		}
	}
	runtime_status_flags |= RUNTIME_STATUS_RUNNING;

	game_loop();

	if (runtime_status_code != 0) {
		return;
	}

	for (os::Module* module : modules) {
		if(module->_status == os::MODULE_STATUS_OK &&  module->deinit) {
			auto result = module->deinit();
			if (result != 0) {
				runtime_status_code = result;
				if (result < 0) {
					runtime_status_flags |= RUNTIME_STATUS_ERROR;
					return;
				}	
			}
		}
		ST_DELETE(module);
	}
	modules.clear();


	proto::shutdown();
}

void shutdown() {
    runtime_status_flags &= ~RUNTIME_STATUS_RUNNING;
}


void log_mem() {
#ifdef ST_ENABLE_MEMORY_TRACKING
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
#endif // ST_ENABLE_MEMORY_TRACKING
}


void game_loop() {
	
	Timer frame_timer;
	f64 min_frametime = 1.0 / (f64)max_fps;

    while(runtime_status_flags & RUNTIME_STATUS_RUNNING) {

		tm_func();

		auto last_frame_time = frame_timer.record();
        frame_timer.reset();

		if (os::is_input_down(os::INPUT_CODE_LSHIFT) && os::is_input_down(os::INPUT_CODE_P) && os::is_input_down(os::INPUT_CODE_M) && os::is_input_down(os::INPUT_CODE_C)) {
			Global_Allocator::reset_stats();
		}

		if (os::is_input_down(os::INPUT_CODE_LSHIFT) && os::is_input_down(os::INPUT_CODE_P) && os::is_input_down(os::INPUT_CODE_M)) {
			log_mem();
		}

		if (proto::need_window_management()) {
			proto::get_window()->new_frame((f32)last_frame_time.get_seconds());
		}

		for (os::Module* module : modules) {
			if (module->_status == os::MODULE_STATUS_OK) {
				if (module->update) {
					auto result = module->update(last_frame_time);
					if (result != 0) {
						runtime_status_code = result;
						runtime_status_flags &= ~RUNTIME_STATUS_RUNNING;
						if (result < 0) {
							runtime_status_flags |= RUNTIME_STATUS_ERROR;
							return;
						}	
					}
				}	
				
			}
		}

		if (proto::need_window_management()) {
			proto::get_window()->render();
		}

		while (frame_timer.record().get_seconds() < min_frametime);
    }

	log_info("App loop exited as expected");
}

s32 get_status_flags() {
	return runtime_status_flags;
}

// Sets global fps limit, meaning it affects all loaded modules.
void set_fps_limit(f32 fps_limit) {
	max_fps = fps_limit;
}

NS_END(runtime);
NS_END(stallout);
#include "pch.h"
#include "os/modules.h"
#include "os/io.h"


#include "Engine/logger.h"
#include "Engine/memory.h"
#include "Engine/debug/tests.h"

#define _ST_RUN_TESTS 1

/*
DO

Basic memory allocation (Global, prealloc budget, fallback)

Som kind of module list generated in build time

*/

std::ofstream log_stream;
int main(/*char** argv, int argc*/) {
	log_stream.open("output");
	init_logger(log_stream);
	Global_Allocator::init();

	#if _ST_RUN_TESTS && !(_ST_DISABLE_ASSERTS)
	run_tests();
	#endif
	
	path_str_t sandbox_path = "";
	sprintf(sandbox_path, "%s/%s", os::io::get_exe_dir().str, "Sandbox.dll");
	os::Module* test_mod = ST_NEW(os::Module, sandbox_path);

	test_mod->init();

	test_mod->update(5);

	ST_DELETE(test_mod);

	return 0;
}
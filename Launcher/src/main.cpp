#include "pch.h"
#include "os/modules.h"
#include "os/io.h"


#include "Engine/logger.h"

/*
DO

Basic memory allocation (Global, prealloc budget, fallback)

Som kind of module list generated in build time

*/

std::ofstream log_stream;
int main(/*char** argv, int argc*/) {
	log_stream.open("output");
	init_logger(log_stream);
	
	path_str_t sandbox_path = "";
	sprintf(sandbox_path, "%s/%s", os::io::get_exe_dir().str, "Sandbox.dll");
	os::Module test_mod(sandbox_path);

	test_mod.init();

	test_mod.update(5);

	return 0;
}
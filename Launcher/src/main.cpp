#include "pch.h"
/*#include "os/modules.h"
#include "os/io.h"


#include "Engine/logger.h"
#include "Engine/debug/tests.h"
#include "Engine/containers.h"*/

#include "Engine/runtime.h"
#include "Engine/logger.h"


int main(/*char** argv, int argc*/) {
	
	engine::runtime::start();

	if (engine::runtime::get_status_flags() & RUNTIME_STATUS_ERROR) {
		log_error("Engine exited with errors");
	} else {
		log_info("Engine runtime exited as expected");
	}

	return 0;
}
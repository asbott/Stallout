#include "pch.h"

#include "Stallout/runtime.h"
#include "Stallout/logger.h"

int main(/*char** argv, int argc*/) {
	
	stallout::runtime::init();

	stallout::runtime::start();

	auto status_code = stallout::runtime::get_status_code();

	if (stallout::runtime::get_status_flags() & RUNTIME_STATUS_ERROR) {
		log_error("Stallout runtime exited with code {}", status_code);
	} else {
		log_info("Stallout runtime exited with code {}", status_code);
	}

	return status_code;
}
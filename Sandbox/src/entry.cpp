#include "pch.h"

#include <Engine/logger.h>

export_function(int) init() {
    log_error("SANDBOX INIT");

    return 0;
}

export_function(int) update(float delta_time) {
    log_warn("SANDBOX UPDATE {}", (double)delta_time);

    return 0;
}
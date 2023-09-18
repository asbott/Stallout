#pragma once

// For basics in client modules

#include <Engine/logger.h>
#include <Engine/timing.h>

#ifdef _ST_OS_WINDOWS
    #define export_function(ret) extern "C" __declspec(dllexport) ret __cdecl
#elif defined(_OS_LINUX)
    #define export_function(ret) extern "C" ret
#endif
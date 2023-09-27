#pragma once

// For basics in client modules

#include <os/io.h>

#include <Engine/logger.h>
#include <Engine/timing.h>
#include <Engine/maths.h>

#include <Engine/renderer.h>
#include <Engine/audio.h>

#ifdef _ST_OS_WINDOWS
    #define export_function(ret) extern "C" __declspec(dllexport) ret __cdecl
#elif defined(_OS_LINUX)
    #define export_function(ret) extern "C" ret
#endif
#pragma once

// For basics in client modules

#include <os/graphics.h>
#include <os/io.h>

#include <Engine/logger.h>
#include <Engine/timing.h>
#include <Engine/maths.h>

#include "os/oswindow.h"
#include <Engine/renderer.h>
#include <Engine/audio.h>

#include <mz_vector.hpp>
#include <mz_matrix.hpp>
#include <mz_algorithms.hpp>

namespace st {
    using namespace engine;
    namespace gfx = engine::graphics;

    typedef mz::fvec2 vec2;
    typedef mz::fvec2 fvec2;

    typedef mz::svec2 svec2;
    typedef mz::svec2 ivec2;
    typedef mz::uvec2 uvec2;
    
    typedef mz::fvec3 vec3;
    typedef mz::fvec3 fvec3;

    typedef mz::svec3 svec3;
    typedef mz::svec3 ivec3;
    typedef mz::uvec3 uvec3;

    typedef mz::fvec4 vec4;
    typedef mz::fvec4 fvec4;

    typedef mz::svec4 svec4;
    typedef mz::svec4 ivec4;
    typedef mz::uvec4 uvec4;

    typedef mz::fvec4 rect;
    typedef mz::fvec4 frect;

    typedef mz::svec4 srect;
    typedef mz::svec4 irect;
    typedef mz::uvec4 urect;
    
}

#ifdef _ST_OS_WINDOWS
    #define export_function(ret) extern "C" __declspec(dllexport) ret __cdecl
#elif defined(_OS_LINUX)
    #define export_function(ret) extern "C" ret
#endif

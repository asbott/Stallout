#pragma once //

// For basics in client modules

#include <Stallout/os.h>

#include <Stallout/logger.h>
#include <Stallout/timing.h>
#include <Stallout/maths.h>

#include <Stallout/graphics.h>
#include <Stallout/audio.h>

#include <mz_vector.hpp>
#include <mz_matrix.hpp>
#include <mz_algorithms.hpp>

namespace st {
    using namespace stallout;
    namespace gfx = stallout::graphics;

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

    typedef mz::fray2d ray2d;

    typedef mz::fcolor16 color;  

    inline String get_module_dir() {
        String dir = os::io::get_workspace_dir();
        dir.concat("/%s", ST_MODULE_NAME);

        return dir;
    }
}

#ifdef _ST_OS_WINDOWS
    #define export_function(ret) extern "C" __declspec(dllexport) ret __cdecl
#elif defined(_OS_LINUX)
    #define export_function(ret) extern "C" ret
#endif
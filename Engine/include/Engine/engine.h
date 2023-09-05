#pragma once

#ifdef _ST_OS_WINDOWS
    #define st_public(ret) extern "C" __declspec(dllexport) ret __cdecl
#elif defined(_OS_LINUX)
    #define module_function(ret) extern "C" ret
#endif
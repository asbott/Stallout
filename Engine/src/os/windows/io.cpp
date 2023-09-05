#include "pch.h"

#include "io.h"

#include "Windows.h"

NS_BEGIN(os)
NS_BEGIN(io)

New_String ST_API get_exe_path() {
    New_String result(MAX_PATH);
    GetModuleFileNameA(NULL, result.str, MAX_PATH);
    return result;
}

NS_END(io)
NS_END(os)
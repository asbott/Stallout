#include "pch.h"

#include "compiler/stcompiler.h"

#include "os/io.h"

NS_BEGIN(engine);
NS_BEGIN(compiler);

ST_Compiler::ST_Compiler() {

}
ST_Compiler::~ST_Compiler() {
    
}

Compile_Result compile_file(const char* path) {
    os::io::File_Info  file_info;

    os::io::get_file_info(path, &file_info);

    return COMPILE_RESULT_OK;
}

NS_END(compiler);
NS_END(engine);
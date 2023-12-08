#pragma once //

NS_BEGIN(stallout);
NS_BEGIN(compiler);

enum Compile_Result {
    COMPILE_RESULT_OK,
    COMPILE_RESULT_FILE_NOT_FOUND
};

struct ST_Compiler {

    ST_Compiler();
    ~ST_Compiler();

    Compile_Result compile_file(const char* path);  
};

NS_END(compiler);
NS_END(stallout);
#pragma once

#include "Engine/stringutils.h"
#include "Engine/containers.h"

#if defined(_WIN32) || defined(_WIN64)
    #include <Windows.h>
    #define ST_MAX_PATH MAX_PATH
#elif defined(__linux__) || defined(__unix__)
    #include <limits.h>
    #if defined(PATH_MAX)
        #define ST_MAX_PATH PATH_MAX
    #else
        #define ST_MAX_PATH 320
    #endif
#elif defined(__APPLE__) && defined(__MACH__)
    #include <sys/syslimits.h>
    #if defined(PATH_MAX)
        #define ST_MAX_PATH PATH_MAX
    #else
        #define ST_MAX_PATH 320
    #endif
#else
    #define ST_MAX_PATH 320
#endif


enum Io_Status {
    IO_STATUS_OK,
    IO_STATUS_INVALID_PATH,
    IO_STATUS_CANT_OPEN_FILE,
    IO_STATUS_NO_SUCH_DIRECTORY,
    IO_STATUS_OPERATION_FAILED,
    IO_STATUS_INVALID_BUFFER
};

NS_BEGIN(os)
NS_BEGIN(io)

using New_String = engine::New_String;

New_String ST_API get_exe_path();

struct File_Info {
    size_t file_size;
};

Io_Status ST_API get_file_info(const char* path, File_Info* result);

// Read operations r+
Io_Status ST_API read_all_bytes(const char* path, byte_t* buffer, size_t size);
Io_Status ST_API read_as_string(const char* path, char* buffer, size_t size);

// Write operations w+
Io_Status ST_API write_bytes(const char* path, const byte_t* buffer, size_t size);
Io_Status ST_API write_string(const char* path, const char* buffer, size_t size);

// Append operations a+
Io_Status ST_API append_bytes(const char* path, byte_t* buffer, size_t size);
Io_Status ST_API append_string(const char* path, char* buffer, size_t size);

// Replace dst if it doesnt exist
Io_Status ST_API copy(const char* src_path, const char* dst_path);
Io_Status ST_API remove(const char* path);

// Get the number of files and subdirectories in given directory
Io_Status ST_API count_directory_entries(const char* dir_path, size_t* result);

// Get all paths to all files and subdirectories in given directory
Io_Status ST_API scan_directory(const char* dir_path, char** result, size_t max_num_result = 100, size_t max_path = MAX_PATH);

Io_Status ST_API to_absolute(const char* path, char* abs_path, size_t max_path = ST_MAX_PATH);

bool ST_API exists(const char* path);
bool ST_API is_file(const char* path);
bool ST_API is_directory(const char* path);

// NOT OS SPECIFIC

Io_Status ST_API read_all_lines(const char* path, engine::Array<New_String>* result);

New_String ST_API get_exe_dir();

// This is different from release build
// and debug build, only use for testing
// purposes.
New_String ST_API get_workspace_dir();

// Path manipulation
New_String ST_API get_directory(const char* path); 
New_String ST_API get_filename(const char* path);
New_String ST_API get_filename_without_extension(const char* path);
New_String ST_API get_file_extension(const char* path);

Io_Status ST_API read_as_string(const char* path, engine::New_String* str);
Io_Status ST_API write_string(const char* path, const engine::New_String& str);
Io_Status ST_API append_string(const char* path, const engine::New_String& str);

NS_END(io)
NS_END(os)
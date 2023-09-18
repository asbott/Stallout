#pragma once

#include "Engine/stringutils.h"
#include "Engine/containers.h"

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
Io_Status ST_API write_bytes(const char* path, byte_t* buffer, size_t size);
Io_Status ST_API write_string(const char* path, char* buffer, size_t size);

// Append operations a+
Io_Status ST_API append_bytes(const char* path, byte_t* buffer, size_t size);
Io_Status ST_API append_string(const char* path, char* buffer, size_t size);

// Replace dst if it doesnt exist
Io_Status ST_API copy(const char* src_path, const char* dst_path);
Io_Status ST_API remove(const char* path);

// Get the number of files and subdirectories in given directory
Io_Status ST_API count_directory_entries(const char* dir_path, size_t* result);

// Get all paths to all files and subdirectories in given directory
Io_Status ST_API scan_directory(const char* dir_path, char** result);


// NOT OS SPECIFIC

Io_Status ST_API read_all_lines(const char* path, engine::Array<New_String>* result);

New_String ST_API get_exe_dir();

// Path manipulation
New_String ST_API get_directory(const char* path); 
New_String ST_API get_filename(const char* path);
New_String ST_API get_filename_without_extension(const char* path);
New_String ST_API get_file_extension(const char* path);

NS_END(io)
NS_END(os)
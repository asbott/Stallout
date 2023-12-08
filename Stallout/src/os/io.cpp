#include "pch.h"

#include "os/io.h"

NS_BEGIN(stallout);
NS_BEGIN(os)
NS_BEGIN(io)

Io_Status load_file(const char* path, File*& result) {
    File_Info fifo;

    auto status = get_file_info(path, &fifo);
    if (status != IO_STATUS_OK) return status;

    byte_t* data = (byte_t*)ST_MEMF(fifo.file_size + 1, GLOBAL_ALLOC_FLAG_LARGE);
    status = read_all_bytes(path, data, fifo.file_size);
    if (status != IO_STATUS_OK) return status;

    data[fifo.file_size] = '\0';

    char* path_copy = (char*)ST_STRING_MEM(strlen(path) + 1);
    strcpy(path_copy, path);

    result = stnew (File)(data, fifo, path_copy);

    return IO_STATUS_OK;
}
Io_Status close_file(File*& pfile, size_t write_size) {
    if (!pfile) return IO_STATUS_OPERATION_FAILED;

    auto status = IO_STATUS_OK;

    if (write_size) {
        status = write_bytes(pfile->path, (byte_t*)pfile->data, std::min(write_size, pfile->info.file_size));
    }

    ST_FREE(pfile->data, pfile->info.file_size);
    ST_FREE((char*)pfile->path, strlen(pfile->path) + 1);

    pfile = NULL;

    return status;
}

Io_Status read_all_lines(const char* path, stallout::Array<String>* result) {
    
    String content;
    auto status = read_as_string(path, &content);
    if (status != IO_STATUS_OK) return status;

    // Portability
    content.replace_all("\r\n", '\n');

    split_string(content.str, '\n', result);

    return IO_STATUS_OK;
}

String get_directory(str_ptr_t path) {
    // Find the last occurrence of / or \\ in the path
    const char* lastSlash = strrchr(path, '/');
    const char* lastBackSlash = strrchr(path, '\\');
    const char* lastSeparator = (lastSlash > lastBackSlash) ? lastSlash : lastBackSlash;

    if (lastSeparator == nullptr) {
        // No slashes found, return an empty string
        return String("");
    }

    // Calculate the length of the directory path and create a new string
    size_t directoryLength = lastSeparator - path;
    String directory(directoryLength);

    // Copy the directory part into the new string
    std::strncpy(directory.str, path, directoryLength);
    directory.str[directoryLength] = '\0';  // Null-terminate the string

    return directory;
}

String get_exe_dir() {
    return get_directory(get_exe_path().str);
}

String get_workspace_dir() {
    auto exe_dir = get_exe_dir();

    char wks_dir[ST_MAX_PATH];

    sprintf(wks_dir, "%s%s", exe_dir.str, "/../../../..");

    char abs[ST_MAX_PATH];
    if (to_absolute(wks_dir, abs) != IO_STATUS_OK) {
        strcpy(abs, wks_dir);
    }

    return String(abs);
}



String get_filename(const char* path) {
    // Find the last occurrence of / or \\ in the path
    const char* lastSlash = strrchr(path, '/');
    const char* lastBackSlash = strrchr(path, '\\');
    const char* lastSeparator = (lastSlash > lastBackSlash) ? lastSlash : lastBackSlash;

    if (lastSeparator == nullptr) {
        // No slashes found, return the full path as the filename
        return String(path);
    }

    // Create a new string for the filename part
    String filename(lastSeparator + 1);
    
    return filename;
}
String get_filename_without_extension(const char* path) {
    // Get the filename from the path
    String filename = get_filename(path);

    // Find the last occurrence of '.' in the filename
    const char* lastDot = strrchr(filename.str, '.');
    
    if (lastDot == nullptr) {
        // No dot found, return the whole filename
        return filename;
    }

    // Calculate the length of the filename without the extension
    size_t lengthWithoutExtension = lastDot - filename.str;

    // Create a new string to hold the filename without extension
    String filenameWithoutExtension(lengthWithoutExtension);

    // Copy the characters into the new string
    strncpy(filenameWithoutExtension.str, filename.str, lengthWithoutExtension);
    filenameWithoutExtension.str[lengthWithoutExtension] = '\0';  // Null-terminate the string
    
    return filenameWithoutExtension;
}
String get_file_extension(const char* path) {
    // Find the last occurrence of '.' in the path
    const char* lastDot = strrchr(path, '.');

    if (lastDot == nullptr) {
        // No dot found, return an empty string
        return String("");
    }

    // Create a new string for the extension part
    String extension(lastDot + 1);
    
    return extension;
}


Io_Status read_as_string(const char* path, stallout::String* str) {
    File_Info fifo;
    auto result = get_file_info(path, &fifo);

    if (result != IO_STATUS_OK) return result;

    *str = stallout::String(fifo.file_size + 1);
    result = read_as_string(path, str->str, fifo.file_size + 1);

    return result;
}
Io_Status write_string(const char* path, const stallout::String& str) {
    return write_string(path, str.str, str.len());
}
Io_Status append_string(const char* path, const stallout::String& str) {
    return append_string(path, str.str, str.len());
}

NS_END(io)
NS_END(os)
NS_END(stallout)
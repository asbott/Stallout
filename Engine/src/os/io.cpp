#include "pch.h"

#include "os/io.h"

NS_BEGIN(os)
NS_BEGIN(io)

Io_Status read_all_lines(const char* path, engine::Array<New_String>* result) {
    File_Info finfo;

    auto status = get_file_info(path, &finfo);
    if (status != IO_STATUS_OK) return status;

    New_String content(finfo.file_size + 1);
    status = read_as_string(path, content.str, finfo.file_size + 1);
    if (status != IO_STATUS_OK) return status;

    auto new_str = New_String(content);

    // Portability
    new_str.replace("\r\n", '\n');

    split_string(new_str.str, '\n', result);

    return IO_STATUS_OK;
}

New_String get_directory(str_ptr_t path) {
    // Find the last occurrence of / or \\ in the path
    const char* lastSlash = strrchr(path, '/');
    const char* lastBackSlash = strrchr(path, '\\');
    const char* lastSeparator = (lastSlash > lastBackSlash) ? lastSlash : lastBackSlash;

    if (lastSeparator == nullptr) {
        // No slashes found, return an empty string
        return New_String("");
    }

    // Calculate the length of the directory path and create a new string
    size_t directoryLength = lastSeparator - path;
    New_String directory(directoryLength);

    // Copy the directory part into the new string
    std::strncpy(directory.str, path, directoryLength);
    directory.str[directoryLength] = '\0';  // Null-terminate the string

    return directory;
}

New_String get_exe_dir() {
    return get_directory(get_exe_path().str);
}

New_String get_workspace_dir() {
    auto exe_dir = get_exe_dir();

    size_t new_len = exe_dir.len() + strlen("/../../../..") + 1;

    New_String wks_dir(new_len);

    sprintf(wks_dir.str, "%s%s", exe_dir.str, "/../../../..");

    return wks_dir;
}



New_String get_filename(const char* path) {
    // Find the last occurrence of / or \\ in the path
    const char* lastSlash = strrchr(path, '/');
    const char* lastBackSlash = strrchr(path, '\\');
    const char* lastSeparator = (lastSlash > lastBackSlash) ? lastSlash : lastBackSlash;

    if (lastSeparator == nullptr) {
        // No slashes found, return the full path as the filename
        return New_String(path);
    }

    // Create a new string for the filename part
    New_String filename(lastSeparator + 1);
    
    return filename;
}
New_String get_filename_without_extension(const char* path) {
    // Get the filename from the path
    New_String filename = get_filename(path);

    // Find the last occurrence of '.' in the filename
    const char* lastDot = strrchr(filename.str, '.');
    
    if (lastDot == nullptr) {
        // No dot found, return the whole filename
        return filename;
    }

    // Calculate the length of the filename without the extension
    size_t lengthWithoutExtension = lastDot - filename.str;

    // Create a new string to hold the filename without extension
    New_String filenameWithoutExtension(lengthWithoutExtension);

    // Copy the characters into the new string
    strncpy(filenameWithoutExtension.str, filename.str, lengthWithoutExtension);
    filenameWithoutExtension.str[lengthWithoutExtension] = '\0';  // Null-terminate the string
    
    return filenameWithoutExtension;
}
New_String get_file_extension(const char* path) {
    // Find the last occurrence of '.' in the path
    const char* lastDot = strrchr(path, '.');

    if (lastDot == nullptr) {
        // No dot found, return an empty string
        return New_String("");
    }

    // Create a new string for the extension part
    New_String extension(lastDot + 1);
    
    return extension;
}

NS_END(io)
NS_END(os)
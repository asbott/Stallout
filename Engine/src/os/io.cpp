#include "pch.h"

#include "os/io.h"

NS_BEGIN(os)
NS_BEGIN(io)

New_String ST_API get_directory(str_ptr_t path) {
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

New_String ST_API get_exe_dir() {
    return get_directory(get_exe_path().str).str;
}

NS_END(io)
NS_END(os)
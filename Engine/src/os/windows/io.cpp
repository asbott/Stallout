#include "pch.h"

#include "os/io.h"
#include "os/windows/winutils.h"

#include "Engine/memory.h"

NS_BEGIN(os)
NS_BEGIN(io)

New_String get_exe_path() {
    New_String result(MAX_PATH);
    WIN32_CALL(GetModuleFileNameA(NULL, result.str, MAX_PATH));
    return result;
}

Io_Status get_file_info(const char* path, File_Info* result) {
    HANDLE hFile = CreateFileA(path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        SetLastError(0);
        return IO_STATUS_INVALID_PATH;
    }
    DWORD fileSize = GetFileSize(hFile, NULL);
    CloseHandle(hFile);
    result->file_size = fileSize;
    return IO_STATUS_OK;
}

Io_Status read_all_bytes(const char* path, byte_t* buffer, size_t size) {
    if (!buffer) {
        return IO_STATUS_INVALID_BUFFER;
    }

    HANDLE hFile = CreateFileA(path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        SetLastError(0);
        return IO_STATUS_INVALID_PATH;
    }

    DWORD bytesRead;
    if (!ReadFile(hFile, buffer, (DWORD)size, &bytesRead, NULL)) {
        CloseHandle(hFile);
        SetLastError(0);
        return IO_STATUS_OPERATION_FAILED;
    }

    CloseHandle(hFile);
    return IO_STATUS_OK;
}

Io_Status read_as_string(const char* path, char* buffer, size_t size) {
    char* raw_buffer = (char*)ST_MEM(size);
    auto result = read_all_bytes(path, (byte_t*)raw_buffer, size);
    if (result != IO_STATUS_OK) {
        ST_FREE(raw_buffer, size);
        return result;
    }

    size_t dest_idx = 0;
    for (size_t src_idx = 0; src_idx < size && dest_idx < size - 1; src_idx++) {
        if (raw_buffer[src_idx] == '\r' && src_idx + 1 < size && raw_buffer[src_idx + 1] == '\n') {
            src_idx++;
        }
        buffer[dest_idx++] = raw_buffer[src_idx];
    }

    buffer[dest_idx] = '\0';

    ST_FREE(raw_buffer, size);
    return IO_STATUS_OK;
}

Io_Status write_bytes(const char* path, const byte_t* buffer, size_t size) {
    if (!buffer) {
        return IO_STATUS_INVALID_BUFFER;
    }

    HANDLE hFile = CreateFileA(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        SetLastError(0);
        return IO_STATUS_INVALID_PATH;
    }

    DWORD bytesWritten;
    if (!WriteFile(hFile, buffer, (DWORD)size, &bytesWritten, NULL)) {
        CloseHandle(hFile);
        SetLastError(0);
        return IO_STATUS_OPERATION_FAILED;
    }

    CloseHandle(hFile);
    return IO_STATUS_OK;
}

Io_Status write_string(const char* path, const char* buffer, size_t size) {
    return write_bytes(path, reinterpret_cast<const byte_t*>(buffer), size);
}

Io_Status append_bytes(const char* path, byte_t* buffer, size_t size) {
    // Similar to write_bytes, but with OPEN_ALWAYS flag and setting file pointer to end
    if (!buffer) {
        return IO_STATUS_INVALID_BUFFER;
    }

    HANDLE hFile = CreateFileA(path, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        SetLastError(0);
        return IO_STATUS_INVALID_PATH;
    }

    SetFilePointer(hFile, 0, NULL, FILE_END);

    DWORD bytesWritten;
    if (!WriteFile(hFile, buffer, (DWORD)size, &bytesWritten, NULL)) {
        CloseHandle(hFile);
        SetLastError(0);
        return IO_STATUS_OPERATION_FAILED;
    }

    CloseHandle(hFile);
    return IO_STATUS_OK;
}

Io_Status append_string(const char* path, char* buffer, size_t size) {
    return append_bytes(path, reinterpret_cast<byte_t*>(buffer), size);
}

Io_Status copy(const char* src_path, const char* dst_path) {
    if (!CopyFileA(src_path, dst_path, FALSE)) {
        SetLastError(0);
        return IO_STATUS_OPERATION_FAILED;
    }
    return IO_STATUS_OK;
}

Io_Status remove(const char* path) {
    if (DeleteFileA(path)) {
        return IO_STATUS_OK;
    } else {
        SetLastError(0);
        return IO_STATUS_OPERATION_FAILED;
    }
}

Io_Status count_directory_entries(const char* dir_path, size_t* result) {
    if (!result) {
        return IO_STATUS_INVALID_BUFFER;
    }

    // Create a new buffer to hold dir_path with "\\*" appended
    size_t buffer_len = strlen(dir_path) + 3;  // Two characters for "\\*" and one for null-terminator
    char* search_path = (char*)ST_MEM(buffer_len);

    strcpy(search_path, dir_path);
    strcat(search_path, "\\*");

    WIN32_FIND_DATAA findFileData;
    HANDLE hFind = FindFirstFileA(search_path, &findFileData);

    // Cleanup the allocated buffer
    ST_FREE(search_path, buffer_len);

    if (hFind == INVALID_HANDLE_VALUE) {
        SetLastError(0);
        return IO_STATUS_NO_SUCH_DIRECTORY;
    }

    *result = 0;
    do {
        (*result)++;
    } while (FindNextFileA(hFind, &findFileData) != 0);
    
    FindClose(hFind);
    return IO_STATUS_OK;
}

Io_Status scan_directory(const char* dir_path, char** result, size_t max_num_result, size_t max_path) {
    if (!result) {
        return IO_STATUS_INVALID_BUFFER;
    }

    // Assumes *result has allocated max_num_result * max_path

    size_t buffer_len = strlen(dir_path) + 3;  // Two characters for "\\*" and one for null-terminator
    char* search_path = (char*)ST_MEM(buffer_len);

    strcpy(search_path, dir_path);
    strcpy(search_path + strlen(dir_path), "\\*");

    WIN32_FIND_DATAA findFileData;
    HANDLE hFind = FindFirstFileA(search_path, &findFileData);

    ST_FREE(search_path, buffer_len);

    if (hFind == INVALID_HANDLE_VALUE) {
        SetLastError(0);
        return IO_STATUS_NO_SUCH_DIRECTORY;
    }

    int i = 0;
    do {
        size_t name_len = std::min(strlen(findFileData.cFileName), max_path - 1);
        memcpy(result[i], findFileData.cFileName, name_len);
        result[i][name_len] = '\0';

        i++;
    } while (FindNextFileA(hFind, &findFileData) != 0 && i < max_num_result);

    FindClose(hFind);
    return IO_STATUS_OK;
}

Io_Status to_absolute(const char* path, char* abs_path, size_t max_path) {
    DWORD length = GetFullPathNameA(path, (DWORD)max_path, abs_path, NULL);
    if (length == 0) {
        SetLastError(0);
        return IO_STATUS_INVALID_PATH;
    }
    return IO_STATUS_OK;
}

bool exists(const char* path) {
    DWORD dwAttrib = GetFileAttributesA(path);
    win32::clear_error();
    return dwAttrib != INVALID_FILE_ATTRIBUTES;   
}
bool is_file(const char* path) {
    DWORD dwAttrib = GetFileAttributesA(path);
    win32::clear_error();
    return (dwAttrib != INVALID_FILE_ATTRIBUTES && 
           !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}
bool is_directory(const char* path) {
    DWORD dwAttrib = GetFileAttributesA(path);
    win32::clear_error();
    return (dwAttrib != INVALID_FILE_ATTRIBUTES && 
           (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

NS_END(io)
NS_END(os)
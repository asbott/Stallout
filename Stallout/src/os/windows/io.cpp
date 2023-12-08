#include "pch.h"

#include "os/io.h"
#include "os/windows/winutils.h"

#include "Stallout/memory.h"
NS_BEGIN(stallout)
NS_BEGIN(os)
NS_BEGIN(io)

String get_exe_path() {
    String result(MAX_PATH + 1);
    WIN32_CALL(GetModuleFileName(NULL, result.str, MAX_PATH));
    result.replace_all("\\", '/');
    return result;
}

Io_Status get_file_info(const char* path, File_Info* result) {
    HANDLE hFile = CreateFile(path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
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

    HANDLE hFile = CreateFile(path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
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
    auto result = read_all_bytes(path, (byte_t*)buffer, size);
    if (result != IO_STATUS_OK) {
        return result;
    }

    buffer[size-1] = '\0';

    return IO_STATUS_OK;
}

Io_Status write_bytes(const char* path, const byte_t* buffer, size_t size) {
    if (!buffer) {
        return IO_STATUS_INVALID_BUFFER;
    }

    HANDLE hFile = CreateFile(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        DWORD error = GetLastError();
        SetLastError(0);
        switch (error) {
            case ERROR_PATH_NOT_FOUND:
            case ERROR_FILE_NOT_FOUND:
                return IO_STATUS_INVALID_PATH;
            case ERROR_INVALID_DRIVE:
                return IO_STATUS_INVALID_DRIVE;
            case ERROR_ACCESS_DENIED:
                return IO_STATUS_ACCESS_DENIED;
            default:
                return IO_STATUS_OPERATION_FAILED;
        }
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

    HANDLE hFile = CreateFile(path, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
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
    if (!CopyFile(src_path, dst_path, FALSE)) {
        SetLastError(0);
        return IO_STATUS_OPERATION_FAILED;
    }
    return IO_STATUS_OK;
}

Io_Status move(const char* src_path, const char* dst_path) {
    if (!MoveFile(src_path, dst_path)) {
        SetLastError(0);
        return IO_STATUS_OPERATION_FAILED;
    }
    return IO_STATUS_OK;
}

Io_Status remove(const char* path) {
    if (DeleteFile(path)) {
        return IO_STATUS_OK;
    } else {
        SetLastError(0);
        return IO_STATUS_OPERATION_FAILED;
    }
}

Io_Status create_dir(const char* path) {
    if (is_directory(path)) return IO_STATUS_OK;
    if (GetFileAttributes(path) != INVALID_FILE_ATTRIBUTES) {
        return IO_STATUS_OK;
    }

    char tmp[MAX_PATH];
    char* p = NULL;
    size_t len;

    strncpy(tmp, path, sizeof(tmp));
    len = strlen(tmp);

    for (p = tmp + 1; (size_t)(p - tmp) < len; p++) {
        if (*p == '\\' || *p == '/') {
            *p = '\0';

            if (GetFileAttributes(tmp) == INVALID_FILE_ATTRIBUTES) {
                if (!CreateDirectory(tmp, NULL)) {
                    return IO_STATUS_OPERATION_FAILED;
                }
            }

            *p = '\\'; 
        }
    }

    if (!CreateDirectory(tmp, NULL)) {
        if (GetLastError() == ERROR_ALREADY_EXISTS) {
            return IO_STATUS_OK;
        }
        return IO_STATUS_OPERATION_FAILED;
    }

    return IO_STATUS_OK;
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
    HANDLE hFind = FindFirstFile(search_path, &findFileData);

    // Cleanup the allocated buffer
    ST_FREE(search_path, buffer_len);

    if (hFind == INVALID_HANDLE_VALUE) {
        SetLastError(0);
        return IO_STATUS_NO_SUCH_DIRECTORY;
    }

    *result = 0;
    do {
        (*result)++;
    } while (FindNextFile(hFind, &findFileData) != 0);
    
    FindClose(hFind);
    return IO_STATUS_OK;
}

Io_Status scan_directory(const char* dir_path, char* buffer, size_t max_num_result, size_t max_path) {
    if (!buffer) {
        return IO_STATUS_INVALID_BUFFER;
    }

    size_t buffer_len = strlen(dir_path) + 3;  // Two characters for "\\*" and one for null-terminator
    char* search_path = (char*)ST_MEM(buffer_len);

    strcpy(search_path, dir_path);
    strcat(search_path, "\\*");

    WIN32_FIND_DATAA findFileData;
    HANDLE hFind = FindFirstFile(search_path, &findFileData);

    ST_FREE(search_path, buffer_len);

    if (hFind == INVALID_HANDLE_VALUE) {
        return IO_STATUS_NO_SUCH_DIRECTORY;
    }

    size_t buffer_offset = 0;
    int i = 0;
    do {
        size_t name_len = std::min(strlen(findFileData.cFileName), max_path - 1);
        if (buffer_offset + name_len + 1 > max_num_result * max_path) {
            // Ensure there's enough space in the buffer
            FindClose(hFind);
            return IO_STATUS_INVALID_BUFFER;
        }

        memcpy(buffer + buffer_offset, findFileData.cFileName, name_len);
        buffer_offset += name_len;
        buffer[buffer_offset] = '\0';
        buffer_offset++;

        i++;
    } while (FindNextFile(hFind, &findFileData) != 0 && i < max_num_result);

    FindClose(hFind);
    return IO_STATUS_OK;
}

Io_Status to_absolute(const char* path, char* abs_path, size_t max_path) {
    DWORD length = GetFullPathName(path, (DWORD)max_path, abs_path, NULL);
    
    stallout::string_replace(abs_path, "\\", '/');

    if (length == 0) {
        SetLastError(0);
        return IO_STATUS_INVALID_PATH;
    }
    return IO_STATUS_OK;
}

bool exists(const char* path) {
    DWORD dwAttrib = GetFileAttributes(path);
    win32::clear_error();
    return dwAttrib != INVALID_FILE_ATTRIBUTES;   
}
bool is_file(const char* path) {
    DWORD dwAttrib = GetFileAttributes(path);
    win32::clear_error();
    return exists(path) && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
}
bool is_directory(const char* path) {
    DWORD dwAttrib = GetFileAttributes(path);
    win32::clear_error();
    return exists(path) && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
}

stallout::Array<stallout::String> get_root_dirs() {
    stallout::Array<stallout::String> arr;

    DWORD buf_sz = WIN32_CALL(GetLogicalDriveStrings(0, nullptr)); 
    ST_DEBUG_ASSERT(buf_sz, "Win32 error");

    char* buffer = (char*)ST_STRING_MEM(buf_sz);
    if (WIN32_CALL(GetLogicalDriveStrings(buf_sz, buffer))) { 
        char* drive = buffer;
        while (*drive) {
            drive[2] = 0; // Remove last slash
            arr.push_back(drive);

            drive += strlen(drive) + 1; 
        }
    }
    ST_FREE(buffer, buf_sz);

    return arr;
}

NS_END(io)
NS_END(os)
NS_END(stallout)
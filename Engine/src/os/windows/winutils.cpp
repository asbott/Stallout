#include "pch.h"

#include "os/windows/winutils.h"

#include "Windows.h"

#include "Engine/logger.h"

NS_BEGIN(os)
NS_BEGIN(win32);

void assert_no_errors(bool uncaught, const char* file, u32 line, const char* action) {
    (void)file;
    (void)line;
    (void)uncaught;
    DWORD error_code = GetLastError();
    
    LPSTR error_message = nullptr;

    if (error_code != 0) {
        FormatMessageA(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, // no source buffer needed as we're formatting a system message
            error_code,
            MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), // Specify language
            (LPSTR)&error_message,
            0, // minimum size for output buffer
            NULL
        );
    }

    ST_ASSERT(error_code == 0, "{}Win32 error has occured:\nAction: {}\nCode: {}\nMessage: {}\nLocation: {}:{}", uncaught ? "UNCAUGHT " : "", action, error_code, error_message, file, line);

    if (error_code != 0) {
        MessageBoxA(NULL, error_message, "Error", MB_OK | MB_ICONERROR);
        exit(EXIT_FAILURE);
    }
}

void clear_error() {
    SetLastError(0);
}

NS_END(win32);
NS_END(os);
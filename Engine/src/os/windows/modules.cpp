#include "pch.h"

#include "os/modules.h"

#include "windows.h"
#include "os/windows/winutils.h"

#define INIT_MOD_FN(name) { if (auto pfn = GetProcAddress(win32_handle, #name)) this->name = (name##_fn_t)pfn;}

NS_BEGIN(os)

Module::Module(str_ptr_t path) {
    // Reset the last-error code.
    SetLastError(0);

    // Load DLL
    const HINSTANCE win32_handle = LoadLibraryA(path);
    DWORD dwError = GetLastError();  // Immediately fetch the last-error code.

    __os_handle = static_cast<void*>(win32_handle);

    if (dwError == 0) {
        INIT_MOD_FN(init);
        INIT_MOD_FN(update);
        INIT_MOD_FN(deinit);
        _status = MODULE_STATUS_OK;
        os::win32::clear_error(); // Functions are optional
    } else {
        switch(dwError) {
            case ERROR_FILE_NOT_FOUND:
                _status = MODULE_STATUS_FILE_NOT_FOUND;
                break;
            case ERROR_MOD_NOT_FOUND:
                _status = MODULE_STATUS_PATH_NOT_FOUND;
                break;
            case ERROR_INVALID_DRIVE:
                _status = MODULE_STATUS_INVALID_DRIVE;
                break;
            case ERROR_BAD_FORMAT:
                _status = MODULE_STATUS_INVALID_FORMAT;
                break;
            case ERROR_DLL_INIT_FAILED:
                _status = MODULE_STATUS_INIT_FAILED;
                break;
            case ERROR_ACCESS_DENIED:
                _status = MODULE_STATUS_ACCESS_DENIED;
                break;
            default:
                _status = MODULE_STATUS_UNKNOWN_ERROR;
                break;
        }
        os::win32::clear_error();
    }
}
Module::~Module() {
    if (__os_handle) {
        FreeLibrary(static_cast<HINSTANCE>(__os_handle));
    }
    __os_handle = NULL;
}

NS_END(os)

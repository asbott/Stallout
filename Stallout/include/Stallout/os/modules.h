#pragma once //

#include "Stallout/timing.h"

NS_BEGIN(stallout);
NS_BEGIN(os);

#define _DECL_MOD_FN(n, ...) typedef int (*n##_fn_t)(__VA_ARGS__); n##_fn_t n = NULL;

enum ST_API Module_Status : s8 {
    MODULE_STATUS_UNSET,
    MODULE_STATUS_UNLOADED,
    MODULE_STATUS_OK,
    MODULE_STATUS_FILE_NOT_FOUND,
    MODULE_STATUS_INVALID_FORMAT,
    MODULE_STATUS_INIT_FAILED,
    MODULE_STATUS_ACCESS_DENIED,
    MODULE_STATUS_PATH_NOT_FOUND,
    MODULE_STATUS_INVALID_DRIVE,
    MODULE_STATUS_UNKNOWN_ERROR,
};
inline str_ptr_t Module_Status_string(Module_Status value) {
    switch (value) {
        case MODULE_STATUS_OK:
            return "OK";
        case MODULE_STATUS_FILE_NOT_FOUND:
            return "File Not Found";
        case MODULE_STATUS_INVALID_FORMAT:
            return "Invalid Format";
        case MODULE_STATUS_INIT_FAILED:
            return "Initialization Failed";
        case MODULE_STATUS_ACCESS_DENIED:
            return "Access Denied";
        case MODULE_STATUS_PATH_NOT_FOUND:
            return "Path Not Found";
        case MODULE_STATUS_INVALID_DRIVE:
            return "Invalid Drive";
        case MODULE_STATUS_UNKNOWN_ERROR:
            return "Unknown Error";
        default:
            return "Missing error string";
    }
}

// TODO (2023-11-02): #API
// This should be more low level and just load
// function pointers and should be renamed to
// "Library" or "Shared_Library"

struct ST_API Module {

    Module(str_ptr_t path);
    ~Module();

    _DECL_MOD_FN (init);
    _DECL_MOD_FN (deinit);
    _DECL_MOD_FN (update, stallout::Duration);


    Module_Status _status = MODULE_STATUS_UNSET;

    void* __os_handle = 0;
};

NS_END(os);
NS_END(stallout);
#include "pch.h"

#include "os/modules.h"

#include <dlfcn.h>
NS_BEGIN(stallout)
NS_BEGIN(os)

Module::Module(const char* path) {
    // Load the shared library (.so file)
    void* linux_handle = dlopen(path, RTLD_NOW);

    if (linux_handle) {
        // Obtain function addresses
        this->init = (init_fn_t) dlsym(linux_handle, "init");
        this->update = (update_fn_t) dlsym(linux_handle, "update");

        _os_handle = linux_handle;
        
        if (!this->init || !this->update) {
            // Function not found, unload shared library
            dlclose(linux_handle);
            _os_handle = nullptr;
        }
    } else {
        _os_handle = nullptr;
    }
}

Module::~Module() {
    // Unload the shared library
    if (_os_handle) {
        dlclose(_os_handle);
    }
}

NS_END(os)
NS_END(stallout)
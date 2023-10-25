#include "pch.h"

#include "os/os.h"

#include "os/windows/winutils.h"

NS_BEGIN(os);

void clear_errors() {
    ::os::win32::clear_error();
}

NS_END(os);
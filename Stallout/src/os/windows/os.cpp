#include "pch.h"

#include "os/os.h"

#include "os/windows/winutils.h"

NS_BEGIN(stallout)
NS_BEGIN(os);

void clear_errors() {
    ::stallout::os::win32::clear_error();
}

NS_END(os);
NS_END(stallout)
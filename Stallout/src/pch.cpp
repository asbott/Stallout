#include "pch.h"

void* operator new(size_t sz) {
    return malloc(sz);
}
#pragma once

#include <stdint.h>

typedef uint8_t  u8;
typedef int8_t   s8;
typedef uint16_t u16;
typedef int16_t  s16;
typedef uint32_t u32;
typedef int32_t  s32;
typedef uint64_t u64;
typedef int64_t  s64;


typedef float    f32;
typedef double   f64;

typedef u8  byte_t;
typedef s8  sbyte_t;

template <size_t LEN = 1024>
using str_t = char[LEN];

#define __def_str_t(nbytes) using str##nbytes##_t = str_t<nbytes>

__def_str_t(16);
__def_str_t(32);
__def_str_t(64);
__def_str_t(128);
__def_str_t(256);
__def_str_t(512);
__def_str_t(1024);

typedef str_t<260> path_str_t;

typedef const char* str_ptr_t;


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

#ifdef _ST_OS_WINDOWS 
    typedef str_t<260> path_str_t;
#else
    typedef str1024_t path_str_t;
#endif
typedef str128_t name_str_t;

typedef const char* str_ptr_t;



struct New_String {
	char* str;

	bool __owner = true;

	inline New_String(str_ptr_t src) {
		const size_t size = strlen(src) + 1;
		str = static_cast<char*>(malloc(size));
		if (str != 0) {
			memset(str, 0, size);
			strcpy(str, src);
		}
		
	}
	inline New_String(const New_String& other) {
		const auto& src = other.str;
		const size_t size = strlen(src) + 1;
		str = static_cast<char*>(malloc(size));
		if (str != 0) {
			memset(str, 0, size);
			strcpy(str, src);
		}
	}
	inline New_String(size_t initial_len = 8) {
		str = static_cast<char*>(malloc(initial_len + 1));
		if (str != 0) memset(str, 0, initial_len + 1);
	}
	inline ~New_String() {
		if (__owner) free(str);
	}

	bool equals(const New_String& other) const { 
		return this->equals(other.str);
	}
	bool equals(str_ptr_t other) const {
		return strcmp(str, other) == 0;
	}

	// Releases ownership and does not delete string on destruction
	char* release() {
		__owner = false;
		return str;
	}

	inline size_t len() const { return strlen(reinterpret_cast<str_ptr_t>(str)); }
};


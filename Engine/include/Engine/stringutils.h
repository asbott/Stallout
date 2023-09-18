#pragma once

#include "Engine/memory.h"
#include "Engine/containers.h"

NS_BEGIN(engine);

// Dynamically allocated string, memory managed
// Only for basic storage and basic manipulation;
// does not handle buffer reallocation (resizing).
// Basically just a wrapper around a char* which
// takes ownership of the memory
struct New_String {
	char* str;
	size_t _buffer_size;

	bool __owner = true;

	inline New_String(size_t buffer_size) {
		str = static_cast<char*>(ST_MEM(buffer_size));
		_buffer_size = buffer_size;
	}
	inline New_String(const char* src) {
		const size_t size = strlen(src) + 1;
		_buffer_size = size;
		str = static_cast<char*>(ST_MEM(size));
		if (str != 0) {
			strcpy(str, src);
		}
		
	}
	inline New_String(const New_String& other) {
		const auto& src = other.str;
		const size_t size = strlen(src) + 1;
		_buffer_size = size;
		str = static_cast<char*>(ST_MEM(size));
		if (str != 0) {
			strcpy(str, src);
		}
	}
	inline ~New_String() {
		if (__owner) ST_FREE(str, _buffer_size);
	}

	bool equals(const New_String& other) const { 
		return this->equals(other.str);
	}
	bool equals(const char* other) const {
		return strcmp(str, other) == 0;
	}

	void replace(const char* a, char b) {
		char* ptr = str;
		while((ptr = strstr(ptr, a)) != NULL) {
			*ptr = b;        // Replace \r with \n
			memmove(ptr + 1, ptr + 2, strlen(ptr + 2) + 1);  // Shift to remove \n
		}
	}

	// Releases ownership and does not delete string on destruction
	char* release() {
		__owner = false;
		return str;
	}

	inline size_t len() const { return strlen(reinterpret_cast<const char*>(str)); }

	friend std::ostream& operator<<(std::ostream& os, const New_String& ns) {
        os << ns.str;
        return os;
    }
};

void ST_API split_string(const char* str, const char* delimiter, Array<New_String>* result);
inline void split_string(const char* str, char delimiter, Array<New_String>* result) {
	char delim[2] { delimiter, '\0' };
	split_string(str, delim, result);
}

NS_END(engine);
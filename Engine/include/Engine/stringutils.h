#pragma once

#include "Engine/memory.h"
#include "Engine/containers.h"

NS_BEGIN(engine);

// TODO (2023-10-04): #performance #strings
// Use a fast buffer for small strings ~16 bytes
// to avoid dynamic allocations.

// Dynamically allocated string, memory managed
// Only for basic storage and basic manipulation;
// does not handle buffer reallocation (resizing).
// Basically just a wrapper around a char* which
// takes ownership of the memory
struct New_String {
	char* str;
	size_t _buffer_size;

	bool __owner = true;

	inline New_String() {
		str = static_cast<char*>(ST_STRING_MEM(1));
		_buffer_size = 1;
		memset(str, 0, 1);
	}
	explicit inline New_String(size_t len) {
		size_t buffer_size = len + 1;
		str = static_cast<char*>(ST_STRING_MEM(buffer_size));
		_buffer_size = buffer_size;
		memset(str, 0, _buffer_size);
	}
	inline New_String(const char* src) {
		const size_t size = strlen(src) + 1;
		_buffer_size = size;
		str = static_cast<char*>(ST_STRING_MEM(size));
		if (str != 0) {
			strcpy(str, src);
		}
		
	}
	inline New_String(const New_String& other) {
		const auto& src = other.str;
		const size_t size = strlen(src) + 1;
		_buffer_size = size;
		str = static_cast<char*>(ST_STRING_MEM(size));
		if (str != 0) {
			strcpy(str, src);
		}
	}
	New_String& operator=(const New_String& other) {
		if (this != &other) {
			if (__owner && str) {
				ST_FREE(str, _buffer_size);
			}

			
			_buffer_size = other._buffer_size;
			str = static_cast<char*>(ST_STRING_MEM(_buffer_size));
			if (str != nullptr) {
				strcpy(str, other.str);
			}
			__owner = true;
		}
		return *this;
	}
	New_String(New_String&& other) noexcept : 
    str(other.str), 
    _buffer_size(other._buffer_size), 
    __owner(other.__owner) {
		other.str = nullptr;
		other._buffer_size = 0;
		other.__owner = false;
	}
	New_String& operator=(New_String&& other) noexcept {
		if (this != &other) {
			if (__owner && str) {
				ST_FREE(str, _buffer_size);
			}

			str = other.str;
			_buffer_size = other._buffer_size;
			__owner = other.__owner;

			other.str = nullptr;
			other._buffer_size = 0;
			other.__owner = false;
		}
		return *this;
	}

	char operator[](size_t pos) {
		ST_DEBUG_ASSERT(pos < this->len());
		return str[pos];
	}

	friend bool operator ==(const New_String& a, const New_String& b) {
		return a.str == b.str || strcmp(a.str, b.str) == 0;
	}
	friend bool operator !=(const New_String& a, const New_String& b) {
		return a.str != b.str && strcmp(a.str, b.str) != 0;
	}
	friend bool operator ==(const New_String& a, const char* b) {
		return a.str == b || strcmp(a.str, b) == 0;
	}
	friend bool operator !=(const New_String& a, const char* b) {
		return a.str != b && strcmp(a.str, b) != 0;
	}
	friend bool operator ==(const char* a, const New_String& b) {
		return a == b.str || strcmp(a, b.str) == 0;
	}
	friend bool operator !=(const char* a, const New_String& b) {
		return a != b.str && strcmp(a, b.str) != 0;
	}

	New_String& concat(const char* src) {
		size_t full_len = strlen(src) + strlen(str);
		if (_buffer_size > full_len) {
			strcat(str, src);
		} else {
			char* new_str = (char*)ST_FAST_MEM(full_len + 1);
			new_str[full_len] = '\0';
		
			sprintf(new_str, "%s%s", str, src);

			if (__owner) {
				ST_FREE(str, _buffer_size);
			}

			str = new_str;
			_buffer_size = full_len  + 1;
			__owner = true;
		}

		return *this;
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

	bool contains (const char* s) {
		return strstr(str, s) != NULL;
	}
	bool contains(char c) {
		char s[2] = { c, '\0' };

		return contains(s);
	}
	size_t count(const char* s) {
		size_t count = 0;
		const char* temp = str;

		while ((temp = strstr(temp, s)) != NULL) {
			count++;
			temp += strlen(s);
		}

		return count;
	}
	size_t count(char c) {
		char s[2] = { c, '\0' };

		return count(s);
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
inline void split_string(const New_String& str, char delimiter, Array<New_String>* result) {
	split_string(str.str, delimiter, result);
}

inline bool try_parse_int(const char* str, s64* result) {
    if (!str || !result) return false;

    char *end;
    errno = 0; // reset errno to 0 before call
    s64 value = strtoll(str, &end, 10); // 10 for base 10

    // Check for conversion errors or if no digits were found
    if (errno || str == end) return false;

    *result = value;
    return true;
}
template <typename int_t>
inline bool try_parse_int(const char* str, int_t* result) {
    static_assert(std::is_integral_v<int_t>, "type is not integral");

	s64 s64_result;
	if (try_parse_int(str, &s64_result)) {
		*result = (int_t)s64_result;
		return true;
	}
	return false;
}
inline bool try_parse_f64(const char* str, f64* result) {
    if (!str || !result) return false;

    char *end;
    errno = 0;
    double value = strtod(str, &end);

    if (errno || str == end) return false;

    *result = value;
    return true;
}
inline bool try_parse_f32(const char* str, f32* result) {
    if (!str || !result) return false;

    f64 f64_result;

	if (try_parse_f64(str, &f64_result)) {
		*result = (f32)f64_result;
		return true;
	}
	return false;
}

NS_END(engine);

namespace std {

template <>
struct hash<const char*> {
    size_t operator()(const char* str) const {
        size_t hash = 2166136261u;  // FNV offset basis
        const size_t prime = 16777619u;  // FNV prime

        while (*str) {
            hash ^= static_cast<size_t>(*str++);
            hash *= prime;
        }

        return hash;
    }
};

}  // end of namespace std

namespace std {

    template<>
    struct hash<engine::New_String> {
    	size_t operator()(const engine::New_String& str) const {
			return std::hash<const char*>()(str.str);
    	}
    };

}
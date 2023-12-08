#pragma once //

#include "Stallout/memory.h"
#include "Stallout/containers.h"

#include "spdlog/fmt/ostr.h"
#include "spdlog/fmt/fmt.h"



NS_BEGIN(stallout);

// TODO (2023-10-04): #performance #strings
// Use a fast buffer for small strings ~16 bytes
// to avoid dynamic allocations.

typedef s32 lchar;

inline std::ostream& operator<<(std::ostream& os, const lchar* s) {

	const lchar* p = s;
	while (*p != 0) {
		os << (char)*p;
		p++;
	}

	return os;
}

ST_API size_t u32strlen(const lchar* s);
ST_API lchar* u32strcpy(lchar* dst, const lchar* src);
ST_API int u32strcmp(const lchar* a, const lchar* b);
ST_API lchar* u32strcat(lchar* dst, const lchar* src);
ST_API const lchar* u32strstr(const lchar* a, const lchar* b);

// Assumes "utf32" buffer is large enough to store result.
ST_API lchar* utf16_to_utf32(const char16_t* utf16, lchar* utf32, size_t input_len = 0);
// Assumes "ascii" buffer is large enough to store result.
ST_API char* utf16_to_ascii(const char16_t* utf16, char* ascii);
// Assumes "utf16" buffer is large enough to store result.
ST_API char16_t* ascii_to_utf16(const char* ascii, char16_t* utf16);
// Assumes "ascii" buffer is large enough to store result.
ST_API char* utf32_to_ascii(const lchar* utf32, char* ascii);
// Assumes "utf32" buffer is large enough to store result.
ST_API lchar* ascii_to_utf32(const char* ascii, lchar* utf32);
// Assumes "utf16" buffer is large enough to store result.
ST_API char16_t* utf32_to_utf16(const lchar* utf32, char16_t* utf16);
// Assumes "utf32" buffer is large enough to store result.
ST_API lchar* wstring_to_utf32(const wchar_t* wstr, lchar* utf32);

template <typename char_t>
inline size_t strlen(const char_t* s) {
	if constexpr (sizeof(char_t) == 1) {
		return ::strlen((char*)s);
	} else {
		return u32strlen((lchar*)s);
	}
}
template <typename char_t>
inline char_t* strcpy(char_t* dst, const char_t* src) {
	if constexpr (sizeof(char_t) == 1) {
		return (char_t*)::strcpy((char*)dst, (char*)src);
	} else {
		return (char_t*)u32strcpy((lchar*)dst, (lchar*)src);
	}
}
template <typename char_t>
inline int strcmp(const char_t* a, const char_t* b) {
	if constexpr (sizeof(char_t) == 1) {
		return ::strcmp((char*)a, (char*)b);
	} else {
		return u32strcmp((lchar*)a, (lchar*)b);
	}
}
template <typename char_t>
inline char_t* strcat(char_t* dst, const char_t* src) {
	if constexpr (sizeof(char_t) == 1) {
		return (char_t*)::strcat((char*)dst, (char*)src);
	} else {
		return (char_t*)u32strcat((lchar*)dst, (lchar*)src);
	}
}
template <typename char_t>
inline const char_t* strstr(const char_t* a, const char_t* b) {
	if constexpr (sizeof(char_t) == 1) {
		return (const char_t*)::strstr((char*)a, (char*)b);
	} else {
		return (const char_t*)u32strstr((lchar*)a, (lchar*)b);
	}
}

template <typename char_t_, typename Sub_Type>
struct String_Impl {

	typedef char_t_ char_t;

	typedef String_Impl<char_t, Sub_Type> Base;

	char_t* str = NULL;
	size_t _buffer_size = 0;

	bool __owner = true;

	void reserve(size_t num_chars) {
		size_t sz = (num_chars + 1) * sizeof(char_t);

		if (sz > _buffer_size) {

			char_t* new_str = (char_t*)ST_STRING_MEM(sz);
			memset(new_str, 0, sz);

			if (str) {
				stallout::strcpy(new_str, str);

				if (__owner) {
					ST_FREE(str, _buffer_size);
				}
			}
			
			str = new_str;
			_buffer_size = sz;
			__owner = true;
		}
	}

	String_Impl& copy_from(const char_t*src, size_t len = 0) {
		if (!src) src = (const char_t*)"\0\0\0";

		len = len ? len : stallout::strlen(src);

		reserve(len);

		memcpy(str, src, (len+1) * sizeof(char_t));

		return *this;
	}
	

	inline String_Impl() {
		reserve(1);
	}
	explicit inline String_Impl(size_t len) {
		reserve(len);
	}


	inline String_Impl(const char_t* src, size_t len = 0) {
		copy_from(src, len);
	}
	inline String_Impl(const String_Impl& other) {
		copy_from(other.str);
	}


	String_Impl& operator=(const String_Impl& other) {
		return copy_from(other.str);
	}
	String_Impl& operator=(const char_t* src) {
		return copy_from(src);
	}

	String_Impl(String_Impl&& other) noexcept : 
		str(other.str), _buffer_size(other._buffer_size), __owner(other.__owner) {
		other.str = nullptr;
		other._buffer_size = 0;
		other.__owner = false;
	}
	String_Impl& operator=(String_Impl&& other) noexcept {
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

	operator Sub_Type&() {
		return *((Sub_Type*)this);
	}


	char_t operator[](size_t pos) {
		ST_DEBUG_ASSERT(pos < this->len());
		return str[pos];
	}

	explicit operator bool() const {
		return str && stallout::strlen(str) > 0;
	}

	inline ~String_Impl() {
		if (__owner) ST_FREE(str, _buffer_size);
	}

	bool equals(const String_Impl& other) const { 
		return this->equals(other.str);
	}
	bool equals(const char_t* other) const {
		return stallout::strcmp(str, other) == 0;
	}

	void replace_all(const char_t* a, char_t b) {
		size_t len = stallout::strlen(a);
		if (!len) return;
		char_t* ptr = str;

		while (*ptr != 0) {
			if (memcmp(ptr, a, len * sizeof(char_t)) == 0) {
				*ptr = b;

				size_t remaining_len = stallout::strlen(ptr+1);
				memmove(ptr + 1, ptr + len, remaining_len * sizeof(char_t)); // Includes null terminator

				ptr += 2;
			} else {
				++ptr; 
			}
		}
	}
	void replace_all(const Sub_Type& a, char_t b) {
		size_t len = a.len();
		if (!len) return;
		char_t* ptr = str;

		while (*ptr != 0) {
			if (memcmp(ptr, a.str, len * sizeof(char_t)) == 0) {
				*ptr = b;

				size_t remaining_len = stallout::strlen(ptr+1);
				memmove(ptr + 1, ptr + len, remaining_len * sizeof(char_t)); // Includes null terminator

				ptr += 2;
			} else {
				++ptr; 
			}
		}
	}

	bool contains (const Sub_Type& s) {
		return stallout::strstr(str, s.str) != NULL;
	}
	bool contains (const char_t* s) {
		return stallout::strstr(str, s) != NULL;
	}
	bool contains(char_t c) {
		char_t s[2] = { c, '\0' };

		return contains(s);
	}
	size_t count(const char_t* s) {
		if (!s) return 0;
		size_t slen = stallout::strlen(s);
		if (!slen || slen > len()) return 0;
		const char_t* p = str;
		size_t c = 0;
		while (*p) {

			const char_t* ps = s;

			size_t offset = 0;

			bool mismatch = false;
			while (*ps) {

				if (*ps != *(p + offset)) {
					mismatch = true;
					break;
				}

				ps++;
				offset++;
			}

			if (!mismatch) {
				c++;
				p += slen;
				continue;
			}

			p++;
		}

		return c;
	}
	size_t count(char_t c) {
		char_t s[2] = { c, '\0' };

		return count(s);
	}
	s64 first_index(char_t c) {
		for (s64 i = 0; i < (s64)stallout::strlen(str); i++) {
			if (str[i] == c) return i;
		}

		return -1;
	}
	s64 last_index(char_t c) {
		s64 last = -1;
		for (s64 i = 0; i < (s64)stallout::strlen(str); i++) {
			if (str[i] == c) last = i;
		}

		return last;
	}

	String_Impl sub_string(size_t index, size_t sub_len = 0) {
		size_t len = stallout::strlen(str);
		
		if (index >= len) return (const char_t*)"\0\0\0";

		String_Impl result(str + index, sub_len);

		return result;
	}

	char_t* sub_string_view(size_t index) {
		size_t len = stallout::strlen(str);

		if (index >= len) return NULL;

		return str + index;
	}

	// Releases ownership and does not delete string on destruction
	char_t* release() {
		__owner = false;
		return str;
	}

	inline size_t len() const { return stallout::strlen(str); }

	inline friend bool operator ==(const Sub_Type& a, const Sub_Type& b) {
		return a.str == b.str || stallout::strcmp(a.str, b.str) == 0;
	}
	friend bool operator !=(const Sub_Type& a, const Sub_Type& b) {
		return a.str != b.str && stallout::strcmp(a.str, b.str) != 0;
	}
	friend bool operator ==(const Sub_Type& a, const char_t* b) {
		return a.str == b || stallout::strcmp(a.str, b) == 0;
	}
	friend bool operator !=(const Sub_Type& a, const char_t* b) {
		return a.str != b && stallout::strcmp(a.str, b) != 0;
	}
	friend bool operator ==(const char_t* a, const Sub_Type& b) {
		return a == b.str || stallout::strcmp(a, b.str) == 0;
	}
	friend bool operator !=(const char_t* a, const Sub_Type& b) {
		return a != b.str && stallout::strcmp(a, b.str) != 0;
	}

	
};

struct LString;
struct _LString_Carry {
	lchar* str = NULL;;
	size_t sz = 0;

	_LString_Carry() {}

	_LString_Carry(_LString_Carry&& src) {
		this->str = src.str;
		src.str = NULL;
	}

	~_LString_Carry() {
		if (str) {
			ST_FREE(str, sz);
		}
	}

	void clear() {
		str = NULL;
	}
};

struct ST_API String : public String_Impl<char, String> {

	using Base::Base;
	using Base::operator=;
	using Base::operator bool;

	//~String() { Base::~String_Impl(); }

	_LString_Carry to_utf32();

	String(const LString& src);
	String& operator=(const LString& src);

	String& concat(const char* fmt, ...);

	String& concat(char c);

	ssize_t insert(size_t index, const char* fmt, ...);

	ssize_t insert(size_t index, char c);

	friend std::ostream& operator<<(std::ostream& os, const String& s) {
		return os << s.str;
	}
};

struct ST_API LString : public String_Impl<lchar, LString> {

	using Base::Base;
	using Base::operator=;
	using Base::operator bool;

	String to_ascii();

	LString(const String& src);
	LString& operator=(const String& src);
	LString(const wchar_t* src);
	LString& operator=(const wchar_t* src);

	LString(const _LString_Carry& u32str);

	LString& concat(const lchar* src);

	LString& concat(const LString& src);

	LString& concat(lchar c);

	ssize_t insert(size_t index, const lchar* src);

	ssize_t insert(size_t index, const LString& src);

	ssize_t insert(size_t index, lchar c);

	LString& copy_from(const char* src, size_t len = 0);
	LString& copy_from(const wchar_t* src, size_t len = 0);

	LString(const char* src, size_t len = 0);

	LString& operator=(const char* src);

	friend std::ostream& operator<<(std::ostream& os, const LString& s) {
		const lchar* p = s.str;
		while (*p != 0) {
			os << (char)*p;
			p++;
		}

		return os;
	}
};



ST_API void string_replace(char* str, const char* a, char b);

ST_API bool string_starts_with(const char* src, const char* str);

void ST_API split_string(const char* str, const char* delimiter, Array<String>* result);
ST_API void split_string(const char* str, char delimiter, Array<String>* result);
ST_API void split_string(const String& str, char delimiter, Array<String>* result);

ST_API bool string_ends_with(const char* str, const char* substr);

ST_API bool try_parse_int(const char* str, s64* result);

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

ST_API bool try_parse_f64(const char* str, f64* result);
ST_API bool try_parse_f32(const char* str, f32* result);

ST_API bool is_numeric(char c);
ST_API bool is_numeric(const char* str);
ST_API bool is_alpha(char c);
ST_API bool is_alpha(const char* str);

NS_END(stallout);

namespace std {

    template<>
    struct hash<stallout::String> {
    	size_t operator()(const stallout::String& str) const {
			size_t hash = 2166136261u;  // FNV offset basis
			const size_t prime = 16777619u;  // FNV prime

			auto p = str.str;

			while (*p) {
				hash ^= static_cast<size_t>(*p++);
				hash *= prime;
			}

			return hash;
    	}
    };

	template<>
    struct hash<stallout::LString> {
    	size_t operator()(const stallout::LString& str) const {
			size_t hash = 2166136261u;  
			const size_t prime = 16777619u; 

			auto p = str.str;

			while (*p) {
				hash ^= static_cast<size_t>(*p++);
				hash *= prime;
			}

			return hash;
    	}
    };

}
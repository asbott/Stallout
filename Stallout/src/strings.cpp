#include "pch.h"

#include "strings.h"

#include <string>
#include <filesystem>

NS_BEGIN(stallout);

size_t u32strlen(const lchar* s) {
	const lchar* p = s;
	while (*p++ != 0);

	return p - s - 1;
}
lchar* u32strcpy(lchar* dst, const lchar* src) {

	if (!dst || !src) return NULL;

	const size_t len_src = u32strlen(src);

	if (!len_src) return NULL;

	memcpy(dst, src, (len_src + 1) * sizeof(lchar));

	return dst;
}
int u32strcmp(const lchar* a, const lchar* b) {

	ST_DEBUG_ASSERT(a && b);

	while (*a && *b) {
		if (*a != *b) {
			return (*a > *b) ? 1 : -1;
		}
		++a;
		++b;
	}

	if (*a == *b) {
		return 0;
	}

	return (*a == 0) ? -1 : 1;
}
lchar* u32strcat(lchar* dst, const lchar* src) {
	if (dst && src) memcpy(dst + u32strlen(dst), src, (u32strlen(src) + 1) * sizeof(lchar));
	return dst;
}
const lchar* u32strstr(const lchar* a, const lchar* b) {
	if (!a || !b) return NULL;

	size_t alen = u32strlen(a);
	size_t blen = u32strlen(b);

	if (blen > alen) return NULL;

	for (size_t i = 0; i <= alen - blen; i++) {
		if (memcmp(a + i, b, blen) == 0) return a + i;
	}

	return NULL;
}

char* utf16_to_ascii(const char16_t* utf16, char* ascii) {
	const char16_t* src = utf16;
	char* dst = ascii;

	while (*src) {
		char16_t codepoint = *src++;
		*dst++ = static_cast<char>(codepoint);
	}

	*dst = 0; 
	return ascii;
}

char16_t* ascii_to_utf16(const char* ascii, char16_t* utf16) {
	
	const char* src = ascii;
	char16_t* dst = utf16;

	while (*src) {
		*dst++ = static_cast<lchar>(*src++);
	}

	*dst = 0;

	return utf16;
}

int is_surrogate(char16_t uc) { return (uc - 0xd800u) < 2048u; }
int is_high_surrogate(char16_t uc) { return (uc & 0xfffffc00) == 0xd800; }
int is_low_surrogate(char16_t uc) { return (uc & 0xfffffc00) == 0xdc00; }

lchar surrogate_to_utf32(char16_t high, char16_t low) { 
	return ((high - 0xD800) << 10) + (low - 0xDC00) + 0x10000; 
}

lchar* utf16_to_utf32(const char16_t *utf16, lchar* utf32, size_t input_len) 
{	
	if (!input_len) {
		const char16_t* p = utf16;
		while(*p++);
		input_len = p - utf16 - 1;
	};
	log_debug("{}", input_len);

	const unsigned char* bytes = reinterpret_cast<const unsigned char*>(utf16);

	for (size_t i = 0; i < input_len * sizeof(wchar_t); ++i) {
		std::cout << "0x" << std::hex << std::setfill('0') << std::setw(2) 
			<< static_cast<int>(bytes[i]) << ' ';
		if ((i + 1) % sizeof(wchar_t) == 0) std::cout << std::endl;
	}
	std::cout << "\n";

	const char16_t * const end = utf16 + input_len;
	while (utf16 < end) {
		const char16_t uc = *utf16++;
		log_debug("wchar: {}", (int)uc);
		if (!is_surrogate(uc)) {
			*utf32++ = uc; 
		} else {
			if (is_high_surrogate(uc) && utf16 < end && is_low_surrogate(*utf16))
				*utf32++ = surrogate_to_utf32(uc, *utf16++);
			else
				*utf32++ = (char)uc;
		}
	}
	*utf32 = 0;

	return utf32;
}

char* utf32_to_ascii(const lchar* utf32, char* ascii) {
	const lchar* src = utf32;
	char* dst = ascii;

	while (*src) {
		lchar codepoint = *src++;
		*dst++ = static_cast<char>(codepoint);
	}

	*dst = 0; 
	return ascii;
}

lchar* ascii_to_utf32(const char* ascii, lchar* utf32) {
	const char* src = ascii;
	lchar* dst = utf32;

	while (*src) {
		*dst++ = static_cast<lchar>(*src++);
	}

	*dst = 0;

	return utf32;
}

inline char16_t* utf32_to_utf16(const lchar* utf32, char16_t* utf16) {
	const lchar* src = utf32;
	char16_t* dst = utf16;

	while (*src) {
		uint32_t codepoint = *src++;

		if (codepoint <= 0xFFFF) {
			// Codepoint fits in a single UTF-16 unit
			*dst++ = static_cast<char16_t>(codepoint);
		} else {
			// Codepoint needs a surrogate pair in UTF-16
			codepoint -= 0x10000;
			*dst++ = static_cast<char16_t>((codepoint >> 10) + 0xD800); // High surrogate
			*dst++ = static_cast<char16_t>((codepoint & 0x3FF) + 0xDC00); // Low surrogate
		}
	}

	*dst = 0; 
	return utf16;
}
lchar* wstring_to_utf32(const wchar_t* wstr, lchar* utf32) {
	std::wstring stlstr(wstr);

	std::filesystem::path p(stlstr);

	auto stl32 = p.generic_u32string();

	size_t i = 0;
	for (auto c32 : stl32) {
		utf32[i++] = (lchar)c32;
	}
	utf32[i] = 0;

	return utf32;
}

_LString_Carry String::to_utf32() {
	_LString_Carry u32str;
	u32str.str = (lchar*)ST_STRING_MEM(sizeof(lchar) * (len() + 1));
	u32str.sz = sizeof(lchar) * (len() + 1);

	ascii_to_utf32(str, u32str.str);

	return u32str;
}

String::String(const LString& src) {
	reserve(src.len());
	utf32_to_ascii(src.str, str);
}
String& String::operator=(const LString& src) {
	reserve(src.len());
	utf32_to_ascii(src.str, str);
	return *this;
}

String& String::concat(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);

	va_list args_copy;
	va_copy(args_copy, args);
	int cat_len = vsnprintf(NULL, 0, fmt, args_copy);
	va_end(args_copy);

	size_t old_len = strlen(str);
	size_t new_len = old_len + cat_len;

	reserve(new_len); 

	::vsnprintf(str + old_len, cat_len + 1, fmt, args);  // +1 for null terminator

	va_end(args);

	return *this;
}

String& String::concat(char c) {
	char s[] { c, '\0' };
	return concat(s);
}

ssize_t String::insert(size_t index, const char* fmt, ...) {
	if (!fmt) return -1;
	if (index > strlen(str)) index = strlen(str);  // Ensure index is within bounds

	va_list args;
	va_start(args, fmt);

	va_list args_copy;
	va_copy(args_copy, args);
	int required_length = vsnprintf(NULL, 0, fmt, args_copy) + 1;
	va_end(args_copy);

	char* temp_buffer = (char*)ST_STRING_MEM(required_length);
	vsnprintf(temp_buffer, required_length, fmt, args);

	size_t full_len = strlen(temp_buffer) + strlen(str);

	bool realloc = full_len + 1 > _buffer_size;

	if (realloc) {
		char* new_str = (char*)ST_STRING_MEM(full_len + 1);
		strncpy(new_str, str, index); // Copy the first part
		new_str[index] = '\0';
		::strcat(new_str, temp_buffer);  // Insert the new part
		::strcat(new_str, str + index);  // Append the rest

		if (__owner) {
			ST_FREE(str, _buffer_size);
		}

		str = new_str;
		_buffer_size = full_len + 1;
		__owner = true;

	} else {
		size_t old_length = strlen(str);
		size_t insert_length = strlen(temp_buffer);

		memmove(str + index + insert_length, str + index, old_length - index + 1);

		memcpy(str + index, temp_buffer, insert_length);
	}

	ST_FREE(temp_buffer, required_length);
	va_end(args);

	return index + required_length - 1;
}

ssize_t String::insert(size_t index, char c) {
	char s[] { c, '\0' };
	return insert(index, s);
}

String LString::to_ascii() {
	String ascii;

	ascii.reserve(len());

	utf32_to_ascii(str, ascii.str);

	return std::move(ascii);
}

LString::LString(const String& src) {
	reserve(src.len());
	ascii_to_utf32(src.str, str);
}
LString& LString::operator=(const String& src) {
	reserve(src.len());
	ascii_to_utf32(src.str, str);
	return *this;
}
LString::LString(const wchar_t* src) {
	copy_from(src);
}
LString& LString::operator=(const wchar_t* src) {
	copy_from(src);
	return *this;
}

LString::LString(const _LString_Carry& u32str) {
	Base::copy_from(u32str.str);
}

LString& LString::concat(const lchar* src) {

	reserve((u32strlen(str) + u32strlen(src)));

	u32strcat(str, src);

	return *this;
}

LString& LString::concat(const LString& src) {
	return concat(src.str);
}

LString& LString::concat(lchar c) {
	lchar s[] { c, '\0' };
	return concat(s);
}

ssize_t LString::insert(size_t index, const lchar* src) {
	if (!src) return -1;

	size_t this_len = u32strlen(str);
	size_t src_len = u32strlen(src);
	size_t new_len = this_len + src_len;

	if (index > this_len) index = this_len; 

	reserve(new_len);

	auto insert_start = str + index;
	auto insert_end = str + index + src_len;
	auto new_end = str + new_len;

	memcpy(insert_end, insert_start, (new_end - insert_end) * sizeof(lchar));

	memcpy(str + index, src, src_len * sizeof(lchar));
	str[this_len + src_len] = 0;

	return index + src_len - 1;
}

ssize_t LString::insert(size_t index, const LString& src) {
	return insert(index, src.str);
}

ssize_t LString::insert(size_t index, lchar c) {
	lchar s[] { c, '\0' };
	return insert(index, s);
}

LString& LString::copy_from(const char* src, size_t len) {
	if (!src) src = "";

	len = len ? len : strlen(src);

	reserve(len);

	const char* p = src;
	while (*p != 0) {
		str[p - src] = *p;
		p++;
	}

	str[len] = 0;

	return *this;
}
LString& LString::copy_from(const wchar_t* src, size_t len) {
	len = len ? len : wcslen(src);
	reserve(len);
	wstring_to_utf32(src, str);
	return *this;
}

LString::LString(const char* src, size_t len) {
	copy_from(src, len);
}

LString& LString::operator=(const char* src) {
	return copy_from(src);
}


void string_replace(char* str, const char* a, char b) {
	size_t len = strlen(a);

	if(len == 0)
		return;

	char* ptr = str;
	while((ptr = ::strstr(ptr, a)) != NULL) {
		*ptr = b;
		memmove(ptr + 1, ptr + len, strlen(ptr + len) + 1);
		++ptr;
	}
}

bool string_starts_with(const char* src, const char* str) {
	return strlen(src) >= strlen(str) && memcmp(src, str, strlen(str)) == 0;
}

void split_string(const char* str, const char* delimiter, Array<String>* result) {

	ST_ASSERT(result);

	if (!str || !delimiter) return;

	String copy_str = String(str);

	char* line = strtok(copy_str.str, delimiter);
	while( line != NULL ) {
		result->push_back(String(line));
		line = strtok(NULL, delimiter);
	}
}
void split_string(const char* str, char delimiter, Array<String>* result) {
	char delim[2] { delimiter, '\0' };
	split_string(str, delim, result);
}
void split_string(const String& str, char delimiter, Array<String>* result) {
	split_string(str.str, delimiter, result);
}

bool string_ends_with(const char* str, const char* substr) {

	auto sub_len = strlen(substr);
	auto str_len = strlen(str);

	if (sub_len > str_len) return false;

	size_t offset = str_len - sub_len;

	for (size_t i = 0; i < sub_len; i++) {
		if (str[offset + i] != substr[i]) return false;
	}

	return true;
}

bool try_parse_int(const char* str, s64* result) {
	if (!str || !result) return false;

	char *end;
	errno = 0; // reset errno to 0 before call
	s64 value = strtoll(str, &end, 10); // 10 for base 10

	// Check for conversion errors or if no digits were found
	if (errno || str == end) return false;

	*result = value;
	return true;
}

bool try_parse_f64(const char* str, f64* result) {
	if (!str || !result) return false;

	char *end;
	errno = 0;
	double value = strtod(str, &end);

	if (errno || str == end) return false;

	*result = value;
	return true;
}
bool try_parse_f32(const char* str, f32* result) {
	if (!str || !result) return false;

	f64 f64_result;

	if (try_parse_f64(str, &f64_result)) {
		*result = (f32)f64_result;
		return true;
	}
	return false;
}

bool is_numeric(char c) {
	return isdigit(c) || c == '.';
}
bool is_numeric(const char* str) {
	for (int i = 0; i < strlen(str); i++) {
		if (i == 0 && str[i] == '-' && strlen(str) > 1) continue;
		if (!is_numeric(str[i])) return false;
	}
	return true;
}
bool is_alpha(char c) {
	return isalpha(c);
}
bool is_alpha(const char* str) {
	for (int i = 0; i < strlen(str); i++) {
		if (!is_alpha(str[i])) return false;
	}
	return true;
}

NS_END(stallout);
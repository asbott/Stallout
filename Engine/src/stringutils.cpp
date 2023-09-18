#include "pch.h"

#include "stringutils.h"

NS_BEGIN(engine);

void split_string(const char* str, const char* delimiter, Array<New_String>* result) {

    ST_ASSERT(result);

    if (!str || !delimiter) return;

    New_String copy_str = New_String(str);

	char* line = strtok(copy_str.str, delimiter);
	while( line != NULL ) {
		result->push_back(New_String(line));
		line = strtok(NULL, delimiter);
	}
}

NS_END(engine);
#include "pch.h"

#include "os/env.h"

#include <Windows.h>

#include <assert.h>

NS_BEGIN(os);
NS_BEGIN(env);

Cache_Size get_cache_size() {
    DWORD bufferSize = 0;
    BOOL result = GetLogicalProcessorInformation(NULL, &bufferSize);
    DWORD lastError = GetLastError();
    assert(result == TRUE || lastError == ERROR_INSUFFICIENT_BUFFER && "Failed to query buffer size for GetLogicalProcessorInformation");

    std::vector<SYSTEM_LOGICAL_PROCESSOR_INFORMATION> buffer(bufferSize / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION));
    result = GetLogicalProcessorInformation(&buffer[0], &bufferSize);
    assert(result == TRUE && "Failed to get processor information");

    Cache_Size cs;

    for (const auto& info : buffer) {
        if (info.Relationship == RelationCache) {
            switch (info.Cache.Level) {
                case 1:
                    cs.L1 = info.Cache.Size;
                    break;
                case 2:
                    cs.L2 = info.Cache.Size;
                    break;
                case 3:
                    cs.L3 = info.Cache.Size;
                    break;
                default:
                   
                    break;
            }
        }
    }

    return cs;
}

NS_END(env);
NS_END(os);
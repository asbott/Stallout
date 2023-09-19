#pragma once

NS_BEGIN(os);
NS_BEGIN(env);

struct Cache_Size {
    size_t L1, L2, L3;
};

// Per thread
ST_API Cache_Size get_cache_size();

NS_END(env);
NS_END(os);
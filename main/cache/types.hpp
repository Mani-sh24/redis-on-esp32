#ifndef CACHE_TYPES_HPP
#define CACHE_TYPES_HPP

#include <string>
#include <chrono>

struct CacheItem
{
    std::string value;
    std::chrono::steady_clock::time_point expiry;
};

#endif // CACHE_TYPES_HPP

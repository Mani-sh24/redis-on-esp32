#ifndef CACHE_HPP
#define CACHE_HPP

#include <mutex>
#include <unordered_map>
#include <queue>
#include <chrono>
#include <atomic>
#include <string>
#include <string_view>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "cache/types.hpp"

class Cache
{
private:
    std::mutex mtx;
    std::atomic<bool> running{false};
    TaskHandle_t cleanerHandle = nullptr;
    int cleanerInterval = 5;

    std::priority_queue<std::pair<std::chrono::steady_clock::time_point, std::string>,
                        std::vector<std::pair<std::chrono::steady_clock::time_point, std::string>>,
                        std::greater<>> pq;
    std::unordered_map<std::string, CacheItem> m_cache_storage;

private:
    static void cleanerTask(void *arg);

public:
    Cache(bool autoCleanup = false, int intervalSeconds = 5);
    void startCleaner();
    void stopCleaner();
    bool exists(std::string_view key);
    void put(std::string_view key, std::string_view value, long long expiry_ms = -1);
    std::string get(std::string_view key);
    void remove(std::string_view key);
    void cleanExpired();
    ~Cache();
};

#endif // CACHE_HPP

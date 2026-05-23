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

struct CacheItem
{
    std::string value;
    std::chrono::steady_clock::time_point expiry;
};

class Cache
{
private:
    std::mutex mtx;
    std::atomic<bool> running{false};
    TaskHandle_t cleanerHandle=nullptr;
    int cleanerInterval=5;

    std::priority_queue<std::pair<std::chrono::steady_clock::time_point,std::string>,std::vector<std::pair<std::chrono::steady_clock::time_point,std::string>>,std::greater<>> pq;
    std::unordered_map<std::string,CacheItem> m_cache_storage;

private:
    static void cleanerTask(void *arg)
    {
        Cache *cache=(Cache*)arg;

        while(cache->running)
        {
            cache->cleanExpired();

            vTaskDelay(pdMS_TO_TICKS(cache->cleanerInterval*1000));
        }

        cache->cleanerHandle=nullptr;
        vTaskDelete(NULL);
    }

public:
    Cache(bool autoCleanup=false,int intervalSeconds=5)
    {
        cleanerInterval=intervalSeconds;
        m_cache_storage.reserve(128);

        if(autoCleanup) startCleaner();
    }

    void startCleaner()
    {
        if(running.exchange(true)) return;

        xTaskCreate(cleanerTask,"cache_cleaner",2048,this,3,&cleanerHandle);
    }

    void stopCleaner()
    {
        running=false;
    }
bool exists(std::string_view key)
{
    std::lock_guard lock(mtx);

    auto it=m_cache_storage.find(std::string(key));

    if(it==m_cache_storage.end())
        return false;

    if(std::chrono::steady_clock::now()>=it->second.expiry)
    {
        m_cache_storage.erase(it);
        return false;
    }

    return true;
}
    void put(std::string_view key,std::string_view value,long long expiry_ms=-1)
    {
        std::lock_guard lock(mtx);

        std::string keyStr(key);
        CacheItem item;

        item.value=value;

        if(expiry_ms==-1)
            item.expiry=std::chrono::steady_clock::time_point::max();
        else
        {
            item.expiry=std::chrono::steady_clock::now()+std::chrono::milliseconds(expiry_ms);
            pq.push({item.expiry,keyStr});
        }

        m_cache_storage[keyStr]=std::move(item);
    }

    std::string get(std::string_view key)
    {
        std::lock_guard lock(mtx);

        auto it=m_cache_storage.find(std::string(key));

        if(it==m_cache_storage.end()) return {};

        if(std::chrono::steady_clock::now()>=it->second.expiry)
        {
            m_cache_storage.erase(it);
            return {};
        }

        return it->second.value;
    }

    void remove(std::string_view key)
    {
        std::lock_guard lock(mtx);
        m_cache_storage.erase(std::string(key));
    }

    void cleanExpired()
    {
        std::lock_guard lock(mtx);

        auto now=std::chrono::steady_clock::now();

        while(!pq.empty())
        {
            auto [expiry,key]=pq.top();

            if(expiry>now) break;

            pq.pop();

            auto it=m_cache_storage.find(key);

            if(it!=m_cache_storage.end() && it->second.expiry==expiry)
                m_cache_storage.erase(it);
        }
    }

    ~Cache()
    {
        stopCleaner();
    }
};

#endif
#pragma once

#include "CacheConfig.h"

#include <cstddef>
#include <cstdint>

struct CacheLine {
    bool valid = false;
    std::uint64_t tag = 0;
    std::uint64_t lastUsed = 0;  // LRU: "time" of the last use
};

class Cache {
private:
    CacheConfig config_;

    std::size_t numLines_ = 0;
    unsigned offsetBits_ = 0;
    unsigned indexBits_ = 0;
    std::uint64_t indexMask_ = 0;

    CacheLine* lines_ = nullptr;  // array of numLines_ lines, owned by this object

    std::uint64_t time_ = 0;  // LRU: increases on every access

    std::uint64_t hits_ = 0;
    std::uint64_t misses_ = 0;

public:
    explicit Cache(const CacheConfig& cfg);
    ~Cache();

    // The cache owns raw memory: forbid copying to avoid a double delete[].
    Cache(const Cache&) = delete;
    Cache& operator=(const Cache&) = delete;

    bool access(std::uint64_t addr);  // true = hit, false = miss
    void reset();                     // empty all lines, counters = 0

    std::uint64_t hits() const;
    std::uint64_t misses() const;
    double hitRate() const;
};
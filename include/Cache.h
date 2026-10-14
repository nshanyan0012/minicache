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

    CacheLine* lines_ = nullptr;  // OWNED: created with new[], freed with delete[]
    Cache* next_ = nullptr;       // NOT owned: the next level (e.g. L2), never deleted here

    std::uint64_t time_ = 0;

    std::uint64_t hits_ = 0;
    std::uint64_t misses_ = 0;

public:
    explicit Cache(const CacheConfig& cfg);
    ~Cache();

    // The cache owns raw memory: forbid copying to avoid a double delete[].
    Cache(const Cache&) = delete;
    Cache& operator=(const Cache&) = delete;

    // Connect the next level. The next cache must live longer than this one.
    void setNext(Cache* next) { next_ = next; }

    bool access(std::uint64_t addr);  // true = hit, false = miss
    void reset();                     // empty all lines, counters = 0

    std::uint64_t hits() const;
    std::uint64_t misses() const;
    double hitRate() const;
};
#include "Cache.h"

#include <stdexcept>

Cache::Cache(const CacheConfig& cfg) : config_(cfg) {
    if (!config_.isValid()) {
        throw std::invalid_argument("Invalid cache configuration");
    }

    numLines_ = config_.numLines();
    offsetBits_ = config_.offsetBits();
    indexBits_ = config_.indexBits();
    indexMask_ = config_.numSets() - 1;

    lines_ = new CacheLine[numLines_]{};
}

Cache::~Cache() {
    delete[] lines_;
}

bool Cache::access(std::uint64_t addr) {
    ++time_;  // LRU: the clock ticks on every access

    const auto index = static_cast<std::size_t>((addr >> offsetBits_) & indexMask_);
    const std::uint64_t tag = addr >> (offsetBits_ + indexBits_);
    const std::size_t setStart = index * config_.associativity;

    // 1) Search for a hit.
    for (std::size_t way = 0; way < config_.associativity; ++way) {
        CacheLine& line = lines_[setStart + way];  // not const: we update lastUsed
        if (line.valid && line.tag == tag) {
            line.lastUsed = time_;  // LRU: this line is now the newest
            ++hits_;
            return true;
        }
    }

    // 2) Miss.
    ++misses_;

    // 3) Use an empty line if there is one.
    for (std::size_t way = 0; way < config_.associativity; ++way) {
        CacheLine& line = lines_[setStart + way];
        if (!line.valid) {
            line.valid = true;
            line.tag = tag;
            line.lastUsed = time_;  // LRU
            return false;
        }
    }

    // 4) Set is full: replace the Least Recently Used line (smallest lastUsed).
    std::size_t oldestWay = 0;
    for (std::size_t way = 1; way < config_.associativity; ++way) {
        if (lines_[setStart + way].lastUsed < lines_[setStart + oldestWay].lastUsed) {
            oldestWay = way;
        }
    }

    CacheLine& victim = lines_[setStart + oldestWay];
    victim.tag = tag;          // valid is already true (the set was full)
    victim.lastUsed = time_;

    return false;
}

void Cache::reset() {
    for (std::size_t i = 0; i < numLines_; ++i) {
        lines_[i] = CacheLine{};  // valid = false, tag = 0, lastUsed = 0
    }
    time_ = 0;
    hits_ = 0;
    misses_ = 0;
}

std::uint64_t Cache::hits() const {
    return hits_;
}

std::uint64_t Cache::misses() const {
    return misses_;
}

double Cache::hitRate() const {
    const std::uint64_t total = hits_ + misses_;
    if (total == 0) {
        return 0.0;
    }
    return static_cast<double>(hits_) / static_cast<double>(total);
}
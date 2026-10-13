#include "Cache.h"

#include <stdexcept>

Cache::Cache(const CacheConfig& cfg) : config_(cfg) {
    // Check first: if we throw here, nothing has been allocated yet.
    if (!config_.isValid()) {
        throw std::invalid_argument("Invalid cache configuration");
    }

    // Compute once, so access() is fast.
    numLines_ = config_.numLines();
    offsetBits_ = config_.offsetBits();
    indexBits_ = config_.indexBits();
    indexMask_ = config_.numSets() - 1;

    lines_ = new CacheLine[numLines_]{};  // all lines start empty
}

Cache::~Cache() {
    delete[] lines_;
}

bool Cache::access(std::uint64_t addr) {
    // 1) Split the address with the saved values (no recomputation).
    const auto index = static_cast<std::size_t>((addr >> offsetBits_) & indexMask_);
    const std::uint64_t tag = addr >> (offsetBits_ + indexBits_);

    const std::size_t setStart = index * config_.associativity;

    // 2) Search for a hit.
    for (std::size_t way = 0; way < config_.associativity; ++way) {
        const CacheLine& line = lines_[setStart + way];
        if (line.valid && line.tag == tag) {
            ++hits_;
            return true;
        }
    }

    // 3) Miss.
    ++misses_;

    // 4) Use an empty line if there is one.
    for (std::size_t way = 0; way < config_.associativity; ++way) {
        CacheLine& line = lines_[setStart + way];
        if (!line.valid) {
            line.valid = true;
            line.tag = tag;
            return false;
        }
    }

    // 5) Set is full: replace way 0 (Part 3 will change this to LRU).
    CacheLine& line = lines_[setStart];
    line.valid = true;
    line.tag = tag;

    return false;
}

void Cache::reset() {
    for (std::size_t i = 0; i < numLines_; ++i) {
        lines_[i].valid = false;
        lines_[i].tag = 0;
    }
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

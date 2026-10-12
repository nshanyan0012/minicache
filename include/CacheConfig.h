#pragma once

#include <cstddef>
#include <cstdint>

struct CacheConfig {
    std::size_t cacheSize;
    std::size_t lineSize;
    std::size_t associativity;

    bool isValid() const;
    std::size_t numLines() const;
    std::size_t numSets() const;
    unsigned offsetBits() const;
    unsigned indexBits() const;
};

struct AddressParts {
    std::uint64_t tag;
    std::uint64_t index;
    std::uint64_t offset;
};

AddressParts splitAddress(std::uint64_t addr, const CacheConfig& cfg);
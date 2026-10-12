#include "CacheConfig.h"

namespace {

bool isPowerOfTwo(std::size_t x) {
    return x != 0 && (x & (x - 1)) == 0;
}

unsigned log2Exact(std::size_t x) {
    unsigned bits = 0;
    while (x > 1) {
        x >>= 1;
        ++bits;
    }
    return bits;
}

}  

bool CacheConfig::isValid() const {
    return cacheSize > 0 &&
           lineSize > 0 &&
           associativity > 0 &&
           cacheSize % lineSize == 0 &&
           numLines() % associativity == 0 &&
           isPowerOfTwo(lineSize) &&
           isPowerOfTwo(numSets());
}

std::size_t CacheConfig::numLines() const {
    return cacheSize / lineSize;
}

std::size_t CacheConfig::numSets() const {
    return numLines() / associativity;
}

unsigned CacheConfig::offsetBits() const {
    return log2Exact(lineSize);
}

unsigned CacheConfig::indexBits() const {
    return log2Exact(numSets());
}

AddressParts splitAddress(std::uint64_t addr, const CacheConfig& cfg) {
    const unsigned offsetBits = cfg.offsetBits();
    const unsigned indexBits = cfg.indexBits();

    const std::uint64_t offset = addr & (cfg.lineSize - 1);
    const std::uint64_t index = (addr >> offsetBits) & (cfg.numSets() - 1);
    const std::uint64_t tag = addr >> (offsetBits + indexBits);

    return AddressParts{tag, index, offset};
}
#include "Cache.h"
#include "CacheConfig.h"

#include <iomanip>
#include <iostream>
#include <stdexcept>

void printResult(const char* name, const Cache& cache) {
    std::cout << name
              << "  hits: " << cache.hits()
              << "  misses: " << cache.misses()
              << "  hit rate: " << std::fixed << std::setprecision(2)
              << cache.hitRate() * 100.0 << "%\n";
}

int main() {
    const CacheConfig cfg{32 * 1024, 64, 8};
    const std::uint64_t setStride = 4096;  // 64 sets * 64 bytes -> always set 0

    Cache cache(cfg);

    // A. Same address 10 times
    for (int i = 0; i < 10; ++i) {
        cache.access(0x1000);
    }
    printResult("A. same address x10    ", cache);
    cache.reset();

    // B. Sequential 4 KB, 4 bytes at a time
    for (std::uint64_t addr = 0; addr < 4096; addr += 4) {
        cache.access(addr);
    }
    printResult("B. sequential 4 KB     ", cache);
    cache.reset();

    // C. 8 blocks in set 0, twice (fits in 8 ways)
    for (int round = 0; round < 2; ++round) {
        for (std::uint64_t i = 0; i < 8; ++i) {
            cache.access(i * setStride);
        }
    }
    printResult("C. 8 blocks, same set  ", cache);
    cache.reset();

    // D. 9 blocks in set 0, twice (does not fit)
    for (int round = 0; round < 2; ++round) {
        for (std::uint64_t i = 0; i < 9; ++i) {
            cache.access(i * setStride);
        }
    }
    printResult("D. 9 blocks, same set  ", cache);
    cache.reset();

    // E. Invalid config must throw
    try {
        Cache bad(CacheConfig{32 * 1024, 48, 8});
        std::cout << "E. ERROR: no exception!\n";
    } catch (const std::invalid_argument& e) {
        std::cout << "E. exception caught: " << e.what() << '\n';
    }
    // Test F: Hot block
cache.reset();

// Fill one set with 8 blocks
for (std::uint64_t i = 0; i < 8; ++i) {
    cache.access(i * 4096);
}

// Repeatedly access block 0 and a new block
for (std::uint64_t i = 8; i < 16; ++i) {
    cache.access(0);
    cache.access(i * 4096);
}

std::cout << "Test F - Hot block\n";
std::cout << "Hits: " << cache.hits() << '\n';
std::cout << "Misses: " << cache.misses() << "\n\n";

    return 0;
}
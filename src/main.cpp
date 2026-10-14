#include "Cache.h"
#include "CacheConfig.h"

#include <iomanip>
#include <iostream> 

// A at address 0, B at 32 KB: A[i] and B[i] map to the same set.
void pingPong(Cache& cache) {
    const std::uint64_t arrayB = 32 * 1024;
    for (int round = 0; round < 2; ++round) {
        for (std::uint64_t offset = 0; offset < 4096; offset += 4) {
            cache.access(offset);
            cache.access(arrayB + offset);
        }
    }
}

int main() {
    std::cout << "Ping-pong between two arrays, 32 KB cache, 64-byte lines\n\n";
    std::cout << std::left << std::setw(8) << "ways"
              << std::right << std::setw(8) << "hits"
              << std::setw(10) << "misses"
              << std::setw(12) << "hit rate" << '\n';

    for (std::size_t ways : {1, 2, 4, 8}) {
        Cache c(CacheConfig{32 * 1024, 64, ways});
        pingPong(c);
        std::cout << std::left << std::setw(8) << ways
                  << std::right << std::setw(8) << c.hits()
                  << std::setw(10) << c.misses()
                  << std::setw(11) << std::fixed << std::setprecision(2)
                  << c.hitRate() * 100.0 << "%\n";
    }

    return 0;
}
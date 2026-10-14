#include "Cache.h"
#include "CacheConfig.h"

#include <cstdint>
#include <iostream>
#include <stdexcept>

namespace {

// ---------------- Tiny test helpers ----------------

int failures = 0;

void check(bool condition, const char* name) {
    if (condition) {
        std::cout << "PASS  " << name << '\n';
    } else {
        std::cout << "FAIL  " << name << '\n';
        ++failures;
    }
}

void checkEqual(std::uint64_t actual, std::uint64_t expected, const char* name) {
    if (actual == expected) {
        std::cout << "PASS  " << name << '\n';
    } else {
        std::cout << "FAIL  " << name << ": expected " << expected
                  << ", got " << actual << '\n';
        ++failures;
    }
}

// ---------------- Shared settings and patterns ----------------

const CacheConfig kL1{32 * 1024, 64, 8};  // 64 sets, 8 ways
const std::uint64_t kSetStride = 4096;    // 64 sets * 64 bytes -> always set 0

// A at address 0, B at 32 KB. A[i] and B[i] map to the SAME set.
// Read A[0], B[0], A[1], B[1], ... over 4 KB each, two rounds.
void pingPong(Cache& cache) {
    const std::uint64_t arrayB = 32 * 1024;
    for (int round = 0; round < 2; ++round) {
        for (std::uint64_t offset = 0; offset < 4096; offset += 4) {
            cache.access(offset);           // A
            cache.access(arrayB + offset);  // B
        }
    }
}

// ---------------- CacheConfig tests ----------------

void testConfigValid() {
    const CacheConfig cfg{32 * 1024, 64, 8};
    check(cfg.isValid(), "config 32KB/64/8 is valid");
    checkEqual(cfg.numLines(), 512, "config: number of lines");
    checkEqual(cfg.numSets(), 64, "config: number of sets");
    checkEqual(cfg.offsetBits(), 6, "config: offset bits");
    checkEqual(cfg.indexBits(), 6, "config: index bits");
}

void testConfigInvalid() {
    check(!CacheConfig{32 * 1024, 48, 8}.isValid(), "config: line size 48 is invalid");
    check(!CacheConfig{0, 64, 8}.isValid(), "config: cache size 0 is invalid");
    check(!CacheConfig{24 * 1024, 64, 8}.isValid(), "config: 48 sets is invalid");
}

void testSplitAddress() {
    const AddressParts p = splitAddress(0x12345, kL1);
    checkEqual(p.tag, 18, "splitAddress: tag");
    checkEqual(p.index, 13, "splitAddress: index");
    checkEqual(p.offset, 5, "splitAddress: offset");
}

// ---------------- Cache tests ----------------

void testEmptyHitRate() {
    const Cache c(kL1);
    check(c.hitRate() == 0.0, "empty cache: hit rate is 0");
}

void testSameAddress() {
    Cache c(kL1);
    for (int i = 0; i < 10; ++i) {
        c.access(0x1000);
    }
    checkEqual(c.hits(), 9, "same address x10: hits");
    checkEqual(c.misses(), 1, "same address x10: misses");
}

void testSequential() {
    Cache c(kL1);
    for (std::uint64_t addr = 0; addr < 4096; addr += 4) {
        c.access(addr);
    }
    checkEqual(c.hits(), 960, "sequential 4KB: hits");
    checkEqual(c.misses(), 64, "sequential 4KB: misses");
}

void testFitsInSet() {
    Cache c(kL1);
    for (int round = 0; round < 2; ++round) {
        for (std::uint64_t i = 0; i < 8; ++i) {
            c.access(i * kSetStride);
        }
    }
    checkEqual(c.hits(), 8, "8 blocks in one set x2: hits");
    checkEqual(c.misses(), 8, "8 blocks in one set x2: misses");
}

void testThrashingLRU() {
    Cache c(kL1);
    for (int round = 0; round < 2; ++round) {
        for (std::uint64_t i = 0; i < 9; ++i) {
            c.access(i * kSetStride);
        }
    }
    checkEqual(c.hits(), 0, "9 blocks in one set x2 (LRU thrashing): hits");
    checkEqual(c.misses(), 18, "9 blocks in one set x2 (LRU thrashing): misses");
}

void testHotBlockLRU() {
    Cache c(kL1);
    for (std::uint64_t i = 0; i < 8; ++i) {
        c.access(i * kSetStride);
    }
    for (std::uint64_t i = 8; i < 16; ++i) {
        c.access(0);               // the hot block
        c.access(i * kSetStride);  // a new block
    }
    checkEqual(c.hits(), 8, "hot block (LRU keeps it): hits");
    checkEqual(c.misses(), 16, "hot block (LRU keeps it): misses");
}

void testReset() {
    Cache c(kL1);
    c.access(0x1000);
    c.access(0x1000);
    c.reset();
    c.access(0x1000);  // the cache was emptied, so this must be a miss
    checkEqual(c.hits(), 0, "after reset: hits");
    checkEqual(c.misses(), 1, "after reset: misses");
}

void testInvalidThrows() {
    try {
        Cache bad(CacheConfig{32 * 1024, 48, 8});
        check(false, "invalid config throws invalid_argument");
    } catch (const std::invalid_argument&) {
        check(true, "invalid config throws invalid_argument");
    }
}

void testDirectMappedConflict() {
    Cache c(CacheConfig{32 * 1024, 64, 1});
    pingPong(c);
    checkEqual(c.hits(), 0, "ping-pong, 1 way (direct-mapped): hits");
    checkEqual(c.misses(), 4096, "ping-pong, 1 way (direct-mapped): misses");
}

void testTwoWayNoConflict() {
    Cache c(CacheConfig{32 * 1024, 64, 2});
    pingPong(c);
    checkEqual(c.hits(), 3968, "ping-pong, 2 ways: hits");
    checkEqual(c.misses(), 128, "ping-pong, 2 ways: misses");
}

}  // namespace

int main() {
    testConfigValid();
    testConfigInvalid();
    testSplitAddress();
    testEmptyHitRate();
    testSameAddress();
    testSequential();
    testFitsInSet();
    testThrashingLRU();
    testHotBlockLRU();
    testReset();
    testInvalidThrows();
    testDirectMappedConflict();
    testTwoWayNoConflict();

    std::cout << '\n';
    if (failures == 0) {
        std::cout << "All tests passed\n";
        return 0;
    }
    std::cout << failures << " test(s) failed\n";
    return 1;
}
#include "CacheConfig.h"

#include <iostream>

void printConfig(const char* title, const CacheConfig& cfg) {
    std::cout << title << '\n';
    std::cout << "Valid: " << cfg.isValid() << '\n';

    if (cfg.isValid()) {
        std::cout << "Lines: " << cfg.numLines() << '\n';
        std::cout << "Sets: " << cfg.numSets() << '\n';
        std::cout << "Offset bits: " << cfg.offsetBits() << '\n';
        std::cout << "Index bits: " << cfg.indexBits() << '\n';
    }
    std::cout << '\n';
}

int main() {
    std::cout << std::boolalpha;  // print true/false instead of 1/0

    const CacheConfig cfg1{32 * 1024, 64, 8};

    printConfig("Test 1: 32 KB, 64, 8", cfg1);
    printConfig("Test 2: 48 KB, 64, 12", CacheConfig{48 * 1024, 64, 12});
    printConfig("Test 3: 32 KB, 48, 8 (line size not a power of two)", CacheConfig{32 * 1024, 48, 8});
    printConfig("Test 4: 0, 64, 8 (zero size)", CacheConfig{0, 64, 8});
    printConfig("Test 6: 3072, 48, 1 (line size not a power of two)", CacheConfig{3072, 48, 1});
    printConfig("Test 7: 24 KB, 64, 8 (48 sets, not a power of two)", CacheConfig{24 * 1024, 64, 8});

    const AddressParts parts = splitAddress(0x12345, cfg1);
    std::cout << "Test 5: splitAddress(0x12345, cfg1)\n";
    std::cout << "Tag: " << parts.tag << '\n';
    std::cout << "Index: " << parts.index << '\n';
    std::cout << "Offset: " << parts.offset << '\n';

    return 0;
}
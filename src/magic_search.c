#include "magic_search.h"

// Std includes
#include <stdlib.h>
#include <stdio.h>

// Local includes
#include "board.h"
#include "attacks.h"

uint64_t randomU64() {
    return (uint64_t) rand() << 32 | (uint64_t) rand();
}
uint64_t randomMagic() {
    return randomU64() & randomU64() & randomU64();
}

uint64_t findBishopMagic(uint32_t square) {
    uint64_t mask = bishopMasks[square], occ[512] = {}, att[512] = {}, table[512] = {};
    uint64_t subset = 0;
    occ[0] = subset;
    att[0] = getBishopAttacksSlow(square, subset);
    uint32_t size = 1, bits = bishopRelevantBits[square];
    while (subset != mask) {
        subset = (subset - mask) & mask;
        occ[size] = subset;
        att[size] = getBishopAttacksSlow(square, subset);
        size += 1;
    }
    if (size != (1ull << bits)) {
        printf("Failed to generate correct number of attacks sets\n");
        return 0;
    }

    for (;;) {
        uint64_t magic = randomMagic();
        int32_t good = 1;
        for (uint32_t i = 0; i < size; i += 1) table[i] = 0;
        for (uint32_t i = 0; i < size; i += 1) {
            uint32_t index = (uint32_t) ((magic * occ[i]) >> (64 - bits));
            if (table[index] == att[i]) continue;
            if (table[index]) {
                good = 0;
                break;
            }
            table[index] = att[i];
        }
        if (good) return magic;
    }
}

uint64_t findRookMagic(uint32_t square) {
    uint64_t mask = rookMasks[square], occ[4096] = {}, att[4096] = {}, table[4096] = {};
    uint64_t subset = 0;
    occ[0] = subset;
    att[0] = getRookAttacksSlow(square, subset);
    uint32_t size = 1, bits = rookRelevantBits[square];
    while (subset != mask) {
        subset = (subset - mask) & mask;
        occ[size] = subset;
        att[size] = getRookAttacksSlow(square, subset);
        size += 1;
    }
    if (size != (1ull << bits)) {
        printf("Failed to generate correct number of attacks sets\n");
        return 0;
    }

    for (;;) {
        uint64_t magic = randomMagic();
        int32_t good = 1;
        for (uint32_t i = 0; i < size; i += 1) table[i] = 0;
        for (uint32_t i = 0; i < size; i += 1) {
            uint32_t index = (uint32_t) ((magic * occ[i]) >> (64 - bits));
            if (table[index] == att[i]) continue;
            if (table[index]) {
                good = 0;
                break;
            }
            table[index] = att[i];
        }
        if (good) return magic;
    }
}

void bishopMagicSearch() {
    printf("Bishop magic search:\n{");
    uint32_t totalSize = 0;
    for (uint32_t i = 0; i < 64; i++) {
        printf("0x%llxull, ", findBishopMagic(i));
        totalSize += (uint32_t) (1ull << bishopRelevantBits[i]);
    }
    printf("};\nTotal size: %u\n\n", totalSize);
}

void rookMagicSearch() {
    printf("Rook magic search:\n{");
    uint32_t totalSize = 0;
    for (uint32_t i = 0; i < 64; i++) {
        printf("0x%llxull, ", findRookMagic(i));
        totalSize += (uint32_t) (1ull << rookRelevantBits[i]);
    }
    printf("};\nTotal size: %u\n\n", totalSize);
}

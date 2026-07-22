#include "magic_search.h"

// Std includes
#include <stdio.h>

// Local includes
#include "board.h"
#include "attacks.h"

static uint64_t rngState = 8817264543ull; // any fixed nonzero seed
uint64_t nextInt64() {
    rngState ^= rngState << 13;
    rngState ^= rngState >> 7;
    rngState ^= rngState << 17;
    return rngState;
}
uint64_t randomMagic() {
    return nextInt64() & nextInt64() & nextInt64();
}

uint64_t bishopAttackTable[64][512];
uint64_t rookAttackTable[64][4096];
uint64_t bishopMagics[64];
uint64_t rookMagics[64];

uint32_t tryMagic(uint64_t magic, uint64_t* table, uint64_t* occupancies, uint64_t* attacks, uint32_t count, uint32_t bits) {
    for (uint32_t i = 0; i < count; i += 1) {
        uint32_t idx = (uint32_t) ((occupancies[i] * magic) >> (64 - bits));
        if (table[idx] == attacks[i]) continue;
        if (table[idx] != 0) return i;
        table[idx] = attacks[i];
    }
    return 0;
}

uint64_t findMagic(uint64_t* table, uint64_t* occupancies, uint64_t* attacks, uint32_t count, uint32_t bits) {
    uint32_t attempt = 1, maxAttacks = 0;
    uint64_t magic = 0;
    for (;;) {
        magic = randomMagic();
        for (uint32_t i = 0; i < count; i += 1) table[i] = 0;
        uint32_t correct = tryMagic(magic, table, occupancies, attacks, count, bits);
        if (!correct) break;
        if (correct > maxAttacks) maxAttacks = correct;
        printf("\rFailed magic search, attempt %u, got max correct attacks: %u/%u.", attempt, maxAttacks, count);
        attempt += 1;
    }
    printf("\nFound magic!\n");
    return magic;
}

void magicSearch() {
    printf("Searching bishop magics...\n");
    for (uint32_t square = 0; square < 64; square += 1) {
        printf("Square %u\n", square);

        uint32_t bits = bishopRelevantBits[square];
        uint64_t occupancies[512] = {}, attacks[512] = {};
        uint32_t count = 0;
        uint64_t subset = 0;
        do {
            occupancies[count] = subset;
            attacks[count] = getBishopAttacks(square, subset);
            count += 1;
            subset = (subset - bishopMasks[square]) & bishopMasks[square];
        } while (subset != 0);

        if (count != (1ul << bits)) {
            printf("Assert failed: generated %u attacks but expected %lu\n", count, (1ul << bits));
            continue;
        }

        bishopMagics[square] = findMagic(bishopAttackTable[square], occupancies, attacks, count, bits);
    }

    printf("Searching rook magics...\n");
    for (uint32_t square = 0; square < 64; square += 1) {
        printf("Square %u\n", square);

        uint32_t bits = rookRelevantBits[square];
        uint64_t occupancies[4096] = {}, attacks[4096] = {};
        uint32_t count = 0;
        uint64_t subset = 0;
        do {
            occupancies[count] = subset;
            attacks[count] = getRookAttacks(square, subset);
            count += 1;
            subset = (subset - rookMasks[square]) & rookMasks[square];
        } while (subset != 0);

        if (count != (1ul << bits)) {
            printf("Assert failed: generated %u attacks but expected %lu\n", count, (1ul << bits));
            continue;
        }

        rookMagics[square] = findMagic(rookAttackTable[square], occupancies, attacks, count, bits);
    }
}

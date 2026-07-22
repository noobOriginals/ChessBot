#ifndef OPTIMIZED_MAGIC_SEARCH_HPP
#define OPTIMIZED_MAGIC_SEARCH_HPP

#include <cstdint>
#include <iostream>
#include <vector>
#include <functional>
#include <thread>
#include <atomic>
#include <string>

#include "board.h"
#include "attacks.h"

namespace magic_search {

const uint64_t RNG_SEED = 88172645463325252ull;
inline uint64_t nextInt64(uint64_t& rngState) {
    rngState ^= rngState << 13;
    rngState ^= rngState >> 7;
    rngState ^= rngState << 17;
    return rngState;
}
inline uint64_t randomMagic(uint64_t& rngState) {
    return nextInt64(rngState) & nextInt64(rngState) & nextInt64(rngState);
}

const uint64_t DEFAULT_ATTACK = 0;
inline uint32_t tryMagic(uint64_t magic, uint64_t* table, uint32_t& maxSize, const uint64_t* occupancies, const uint64_t* attacks, uint32_t size, uint32_t bits) {
    maxSize = 0;
    for (uint32_t i = 0; i < size; i++) {
        uint32_t idx = (uint32_t) ((magic * occupancies[i]) >> (64 - bits));
        if (idx + 1 > maxSize) maxSize = idx + 1;
        if (table[idx] == attacks[i]) continue;
        if (table[idx] != DEFAULT_ATTACK) return i;
        table[idx] = attacks[i];
    }
    return 0;
}
inline uint64_t findMagic(uint64_t& rng, uint32_t& maxSize, const uint64_t* occupancies, const uint64_t* attacks, uint32_t size, uint32_t bits) {
    uint64_t* table = new uint64_t[size];
    uint64_t magic = 0;
    while (true) {
        magic = randomMagic(rng);
        for (uint32_t i = 0; i < size; i++) table[i] = DEFAULT_ATTACK;
        if (!tryMagic(magic, table, maxSize, occupancies, attacks, size, bits)) break;
    }
    delete[] table;
    return magic;
}
inline void genPossibleBishopAttacks(uint32_t square, uint64_t* occupancies, uint64_t* attacks, uint32_t& size) {
    size = (1u << bishopRelevantBits[square]);
    uint32_t idx = 0;
    uint64_t subset = 0;
    do {
        occupancies[idx] = subset;
        attacks[idx] = getBishopAttacks(square, subset);
        subset = (subset - bishopMasks[square]) & bishopMasks[square];
        idx++;
    } while (subset != 0);
    if (idx != size) std::cout << "Gen bishop attacks failed\n";
}
inline void genPossibleRookAttacks(uint32_t square, uint64_t* occupancies, uint64_t* attacks, uint32_t& size) {
    size = (1u << rookRelevantBits[square]);
    uint32_t idx = 0;
    uint64_t subset = 0;
    do {
        occupancies[idx] = subset;
        attacks[idx] = getRookAttacks(square, subset);
        subset = (subset - rookMasks[square]) & rookMasks[square];
        idx++;
    } while (subset != 0);
    if (idx != size) std::cout << "Gen rook attacks failed\n";
}
inline std::vector<uint64_t> searchBishopMagics(uint64_t& rng, uint32_t& totalSize) {
    totalSize = 0;
    std::vector<uint64_t> magics(64);
    uint64_t table[512] = {}, occupancies[512] = {}, attacks[512] = {};
    uint32_t size;
    for (uint32_t sq = 0; sq < 64; sq++) {
        genPossibleBishopAttacks(sq, occupancies, attacks, size);
        magics[sq] = findMagic(rng, size, occupancies, attacks, size, bishopRelevantBits[sq]);
        totalSize += size;
    }
    return magics;
}
inline std::vector<uint64_t> searchRookMagics(uint64_t& rng, uint32_t& totalSize) {
    totalSize = 0;
    std::vector<uint64_t> magics(64);
    uint64_t table[4096] = {}, occupancies[4096] = {}, attacks[4096] = {};
    uint32_t size;
    for (uint32_t sq = 0; sq < 64; sq++) {
        genPossibleRookAttacks(sq, occupancies, attacks, size);
        magics[sq] = findMagic(rng, size, occupancies, attacks, size, rookRelevantBits[sq]);
        totalSize += size;
    }
    return magics;
}

}

#endif // OPTIMIZED_MAGIC_SEARCH_HPP

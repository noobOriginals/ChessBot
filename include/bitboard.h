#ifndef BITBOARD_H
#define BITBOARD_H

#include <stdlib.h>

#include "types.h"
#include "debug_utils.h"

#if defined(__cplusplus)
extern "C" {
#endif

#define FILE_A 0x101010101010101ull
#define FILE_B 0x202020202020202ull
#define FILE_C 0x404040404040404ull
#define FILE_D 0x808080808080808ull
#define FILE_E 0x1010101010101010ull
#define FILE_F 0x2020202020202020ull
#define FILE_G 0x4040404040404040ull
#define FILE_H 0x8080808080808080ull
#define RANK_1 0xffull
#define RANK_2 0xff00ull
#define RANK_3 0xff0000ull
#define RANK_4 0xff000000ull
#define RANK_5 0xff00000000ull
#define RANK_6 0xff0000000000ull
#define RANK_7 0xff000000000000ull
#define RANK_8 0xff00000000000000ull

#if defined(_MSC_VER)
    #include <intrin.h>
    #define popcountll(x) __popcnt64(x)
    #define ctzll(x) _tzcnt_u64(x)
    #define clzll(x) _lzcnt_u64(x)
#else
    #define popcountll(x) __builtin_popcountll(x)
    #define ctzll(x) __builtin_ctzll(x)
    #define clzll(x) __builtin_clzll(x)
#endif

#define bbsq(x) (1ull << (x))

typedef u64 Bitboard;

// Diagonals masks indexed by square(s) for between() method
extern Bitboard betweenTable[4096];

// Init static values
void initBitboard(void);

// Pop least significant set bit and return its index
static inline u8 popLSB(Bitboard* bb) {

    PUSH_STACK_TRACE("popLSB()");
    ASSERT_MSG(bb, "popLSB() failed: bitboard pointer cannot be NULL");
    ASSERT_MSG(*bb, "popLSB() failed: bitboard cannot be zero");
    POP_STACK_TRACE();

    u8 sq = (u8) ctzll(*bb);
    *bb ^= bbsq(sq);
    return sq;
}

// popLSB but also return the popped bit's bitboard
static inline u8 popToLSB(Bitboard* bb, Bitboard* lsb) {

    PUSH_STACK_TRACE("popToLSB()");
    ASSERT_MSG(bb, "popLSB() failed: bitboard pointer cannot be NULL");
    ASSERT_MSG(*bb, "popLSB() failed: bitboard cannot be zero");
    POP_STACK_TRACE();

    u8 sq = (u8) ctzll(*bb);
    *lsb = bbsq(sq);
    *bb ^= *lsb;
    return sq;
}

// Get the ray between two biboard squares
static inline Bitboard betweenBB(Bitboard a, Bitboard b) {

    PUSH_STACK_TRACE("betweenBB()");
    ASSERT_MSG(a, "betweenBB() failed: bitboard 'a' cannot be zero");
    ASSERT_MSG(b, "betweenBB() failed: bitboard 'b' cannot be zero");
    POP_STACK_TRACE();

    return betweenTable[(ctzll(a) << 6) + ctzll(b)];
}

// Get the ray between two index squares
static inline Bitboard betweenSQ(u8 a, u8 b) {

    PUSH_STACK_TRACE("betweenSQ()");
    ASSERT_MSG(a < 64, "betweenSQ() failed: square index 'a' cannot be greater than 63");
    ASSERT_MSG(b < 64, "betweenSQ() failed: square index 'b' cannot be greater than 63");
    POP_STACK_TRACE();

    return betweenTable[(a << 6) + b];
}

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // BITBOARD_H

#ifndef BITBOARD_H
#define BITBOARD_H

#include "types.h"

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

// Init static values
void initBitboard();

// Pop least significant set bit and return its index
u8 popLSB(Bitboard* bb);

// popLSB but also return the popped bit's bitboard
u8 popToLSB(Bitboard* bb, Bitboard* lsb);

// Get the ray between two biboard squares
Bitboard betweenBB(Bitboard a, Bitboard b);

// Get the ray between two index squares
Bitboard betweenSQ(u8 a, u8 b);

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // BITBOARD_H

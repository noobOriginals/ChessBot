#include "bitboard.h"

#include <math.h>

#include "debug_utils.h"

// Diagonals masks indexed by square(s) for between() method
static Bitboard betweenTable[4096];

// Init betweenTable
static void initBetweenTable() {
    for (i32 a = 0; a < 64; a++) {
        for (i32 b = 0; b < 64; b++) {
            betweenTable[(a << 6) + b] = 0;

            i32 dr = (b >> 3) - (a >> 3), df = (b & 7) - (a & 7);
            if (abs(dr) < 2 && abs(df) < 2) {
                continue;
            }

            i32 dir = 0;
            if (dr == 0) {
                dir = (df < 0) ? -1 : 1;
            } else if (df == 0) {
                dir = (dr < 0) ? -8 : 8;
            } else if (dr == df) {
                dir = (dr < 0) ? -9 : 9;
            } else if (dr == -df) {
                dir = (dr < 0) ? -7 : 7;
            } else {
                continue;
            }

            for (i32 sq = a + dir; sq != b; sq += dir) betweenTable[(a << 6) + b] |= bbsq(sq);
        }
    }
}

// Init static values
void initBitboard() {
    initBetweenTable();
}

// Pop least significant set bit and return its index
u8 popLSB(Bitboard* bb) {

    ASSERT_MSG(bb, "popLSB() failed: bitboard pointer cannot be NULL");
    ASSERT_MSG(*bb, "popLSB() failed: bitboard cannot be zero");

    u8 sq = (u8) ctzll(*bb);
    *bb ^= bbsq(sq);
    return sq;
}

// popLSB but also return the popped bit's bitboard
u8 popToLSB(Bitboard* bb, Bitboard* lsb) {

    ASSERT_MSG(bb, "popLSB() failed: bitboard pointer cannot be NULL");
    ASSERT_MSG(*bb, "popLSB() failed: bitboard cannot be zero");

    u8 sq = (u8) ctzll(*bb);
    *lsb = bbsq(sq);
    *bb ^= *lsb;
    return sq;
}

// Get the ray between two biboard squares
Bitboard betweenBB(Bitboard a, Bitboard b) {

    ASSERT_MSG(a, "betweenBB() failed: bitboard 'a' cannot be zero");
    ASSERT_MSG(b, "betweenBB() failed: bitboard 'b' cannot be zero");

    return betweenTable[(ctzll(a) << 6) + ctzll(b)];
}

// Get the ray between two index squares
Bitboard betweenSQ(u8 a, u8 b) {

    ASSERT_MSG(a < 64, "betweenSQ() failed: square index 'a' cannot be greater than 63");
    ASSERT_MSG(b < 64, "betweenSQ() failed: square index 'b' cannot be greater than 63");

    return betweenTable[(a << 6) + b];
}

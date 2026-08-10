#include "bitboard.h"

// Diagonals masks indexed by square(s) for between() method
Bitboard betweenTable[4096];

// Init betweenTable
static void initBetweenTable(void) {
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
void initBitboard(void) {
    initBetweenTable();
}

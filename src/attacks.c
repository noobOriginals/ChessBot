#include "attacks.h"

#include "debug_utils.h"

static const u64 bishopMagic[64];
static const u64 rookMagic[64];
static const u8 bishopShift[64];
static const u8 rookShift[64];
static const Bitboard bishopMask[64];
static const Bitboard rookMask[64];

static Bitboard pawnAttacks[128];
static Bitboard knightAttacks[64];
static Bitboard bishopAttacks[5248];
static Bitboard rookAttacks[102400];
static Bitboard kingAttacks[64];

// Init attack tables
void initAttacks() {

}

// 'Ray-Marching' aproach
Bitboard getSlidingAttacks(u8 square, Bitboard occupancy, i8 dir) {

    ASSERT_MSG(square < 64, "getKingAttacks() failed: square index out of bounds");

    Bitboard attacks = 0ull;
    i8 f = (square & 7), r = (square >> 3);
    i8 df = ((square + dir) & 7) - f, dr = ((square + dir) >> 3) - r;
    while (1) {
        square += dir;
        f += df; r += dr;
        if (f < 0 || f > 7 || r < 0 || r > 7) break;
        attacks |= bbsq(square);
        if (attacks & occupancy) break;
    }
    return attacks;
}

Bitboard getBishopAttacksSlow(u8 square, Bitboard occupancy) {

    ASSERT_MSG(square < 64, "getKingAttacks() failed: square index out of bounds");

    return
        getSlidingAttacks(square, occupancy,  9) |
        getSlidingAttacks(square, occupancy, -9) |
        getSlidingAttacks(square, occupancy,  7) |
        getSlidingAttacks(square, occupancy, -7);
}

Bitboard getRookAttacksSlow(u8 square, Bitboard occupancy) {

    ASSERT_MSG(square < 64, "getKingAttacks() failed: square index out of bounds");

    return
        getSlidingAttacks(square, occupancy,  8) |
        getSlidingAttacks(square, occupancy, -8) |
        getSlidingAttacks(square, occupancy,  1) |
        getSlidingAttacks(square, occupancy, -1);
}

Bitboard getQueenAttacksSlow(u8 square, Bitboard occupancy) {

    ASSERT_MSG(square < 64, "getKingAttacks() failed: square index out of bounds");

    return getBishopAttacksSlow(square, occupancy) | getRookAttacksSlow(square, occupancy);
}

// Fast, precomputed aproach
Bitboard getPawnAttacks(u8 square, u8 side) {

    ASSERT_MSG(square < 64, "getKingAttacks() failed: square index out of bounds");
    ASSERT_MSG(side < 2, "getKingAttacks() failed: side can only have the value 0 (zero) or 1 (one)");

    return pawnAttacks[(square << 1) | side];
}

Bitboard getKnightAttacks(u8 square) {

    ASSERT_MSG(square < 64, "getKingAttacks() failed: square index out of bounds");

    return knightAttacks[square];
}

Bitboard getBishopAttacks(u8 square, Bitboard occupancy) {

    ASSERT_MSG(square < 64, "getKingAttacks() failed: square index out of bounds");

}

Bitboard getRookAttacks(u8 square, Bitboard occupancy) {

    ASSERT_MSG(square < 64, "getKingAttacks() failed: square index out of bounds");

}

Bitboard getQueenAttacks(u8 square, Bitboard occupancy) {

    ASSERT_MSG(square < 64, "getKingAttacks() failed: square index out of bounds");

    return getBishopAttacks(square, occupancy) | getRookAttacks(square, occupancy);
}

Bitboard getKingAttacks(u8 square) {

    ASSERT_MSG(square < 64, "getKingAttacks() failed: square index out of bounds");

    return kingAttacks[square];
}

// Bulk generation of pawn pushed
Bitboard getPawnPushes(Bitboard bitboard, Bitboard occupancy, u8 side) {
    return side ? (bitboard >> 8) & ~occupancy : (bitboard << 8) & ~occupancy;
}

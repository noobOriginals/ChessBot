#ifndef ATTACKS_H
#define ATTACKS_H

#include "types.h"
#include "bitboard.h"
#include "board.h"
#include "debug_utils.h"

#if defined(__cplusplus)
extern "C" {
#endif

// Constants for initializing and using magic bitboards.
// These magics numbers and their coresponding table sizes were found with
// a simple search that I wrote myself, and they are public right here:
// https://github.com/noobOriginals/Magicbit/blob/main/search_runs/concat.txt
extern const u64 bishopMagic[64];
extern const u64 rookMagic[64];

// Values for using magic numbers, initiazied once at startup via initAttacks()
extern u8 bishopShift[64];
extern u8 rookShift[64];
extern Bitboard bishopMask[64];
extern Bitboard rookMask[64];

// Attack tables
extern Bitboard pawnAttacks[128];
extern Bitboard knightAttacks[64];
extern Bitboard bishopAttacks[5009], * bishopPtr[64];
extern Bitboard rookAttacks[102359], * rookPtr[64];
extern Bitboard kingAttacks[64];

// Init attack tables
void initAttacks(void);

// For non-sliders
Bitboard getAttacksAt(u8 square, i8 fileOff, i8 rankOffset);

// 'Ray-Marching' aproach
Bitboard getSlidingAttacks(u8 square, Bitboard occupancy, i8 fileDir, i8 rankDir);
Bitboard getBishopAttacksSlow(u8 square, Bitboard occupancy);
Bitboard getRookAttacksSlow(u8 square, Bitboard occupancy);
Bitboard getQueenAttacksSlow(u8 square, Bitboard occupancy);
Bitboard getPieceAttakcsSlow(u8 piece, u8 square, u8 side, Bitboard occupancy);

// Fast, precomputed aproach
inline Bitboard getPawnAttacks(u8 square, u8 side) {

    PUSH_STACK_TRACE("getPawnAttacks()");
    ASSERT_MSG(square < 64, "getPawnAttacks() failed: square index out of bounds");
    ASSERT_MSG(side < 2, "getPawnAttacks() failed: side can only have the value 0 (zero) or 1 (one)");
    POP_STACK_TRACE();

    return pawnAttacks[(square << 1) | side];
}

inline Bitboard getKnightAttacks(u8 square) {

    PUSH_STACK_TRACE("getKnightAttacks()");
    ASSERT_MSG(square < 64, "getKnightAttacks() failed: square index out of bounds");
    POP_STACK_TRACE();

    return knightAttacks[square];
}

inline Bitboard getBishopAttacks(u8 square, Bitboard occupancy) {

    PUSH_STACK_TRACE("getBishopAttacks()");
    ASSERT_MSG(square < 64, "getBishopAttacks() failed: square index out of bounds");

    occupancy &= bishopMask[square];
    occupancy *= bishopMagic[square];
    occupancy >>= bishopShift[square];

    POP_STACK_TRACE();

    return bishopPtr[square][occupancy];
}

inline Bitboard getRookAttacks(u8 square, Bitboard occupancy) {

    PUSH_STACK_TRACE("getRookAttacks()");
    ASSERT_MSG(square < 64, "getRookAttacks() failed: square index out of bounds");

    occupancy &= rookMask[square];
    occupancy *= rookMagic[square];
    occupancy >>= rookShift[square];

    POP_STACK_TRACE();

    return rookPtr[square][occupancy];
}

inline Bitboard getQueenAttacks(u8 square, Bitboard occupancy) {

    PUSH_STACK_TRACE("getQueenAttacks()");
    ASSERT_MSG(square < 64, "getQueenAttacks() failed: square index out of bounds");
    POP_STACK_TRACE();

    return getBishopAttacks(square, occupancy) | getRookAttacks(square, occupancy);
}

inline Bitboard getKingAttacks(u8 square) {

    PUSH_STACK_TRACE("getKingAttacks()");
    ASSERT_MSG(square < 64, "getKingAttacks() failed: square index out of bounds");
    POP_STACK_TRACE();

    return kingAttacks[square];
}

inline Bitboard getPieceAttakcs(u8 piece, u8 square, u8 side, Bitboard occupancy) {

    PUSH_STACK_TRACE("getPieceAttakcs()");
    ASSERT_MSG(piece > 1 && piece < 14, "getPieceAttakcs() failed: piece type out of bounds");
    ASSERT_MSG(square < 64, "getPieceAttakcs() failed: square index out of bounds");
    ASSERT_MSG(side < 2, "getPieceAttakcs() failed: side can only have the value 0 (zero) or 1 (one)");
    POP_STACK_TRACE();

    switch (ptype(piece)) {
        case PAWN: return getPawnAttacks(square, side);
        case KNIGHT: return getKnightAttacks(square);
        case BISHOP: return getBishopAttacks(square, occupancy);
        case ROOK: return getRookAttacks(square, occupancy);
        case QUEEN: return getQueenAttacks(square, occupancy);
        case KING: return getKingAttacks(square);
        default: return 0ull;
    }
}

// Bulk generation of pawn pushed
inline Bitboard getPawnPushes(Bitboard bitboard, Bitboard occupancy, u8 side) {

    PUSH_STACK_TRACE("getPawnPushes()");
    ASSERT_MSG(side < 2, "getPawnPushes() failed: side can only have the value 0 (zero) or 1 (one)");
    POP_STACK_TRACE();

    return side ? (bitboard >> 8) & ~occupancy : (bitboard << 8) & ~occupancy;
}

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // ATTACKS_H

#ifndef ATTACKS_H
#define ATTACKS_H

#include "types.h"
#include "bitboard.h"

#if defined(__cplusplus)
extern "C" {
#endif

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
Bitboard getPawnAttacks(u8 square, u8 side);
Bitboard getKnightAttacks(u8 square);
Bitboard getBishopAttacks(u8 square, Bitboard occupancy);
Bitboard getRookAttacks(u8 square, Bitboard occupancy);
Bitboard getQueenAttacks(u8 square, Bitboard occupancy);
Bitboard getKingAttacks(u8 square);
Bitboard getPieceAttakcs(u8 piece, u8 square, u8 side, Bitboard occupancy);

// Bulk generation of pawn pushed
Bitboard getPawnPushes(Bitboard bitboard, Bitboard occupancy, u8 side);

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // ATTACKS_H

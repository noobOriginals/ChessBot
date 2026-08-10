#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "types.h"
#include "board.h"

#if defined(__cplusplus)
extern "C" {
#endif

#define MAX_LEGAL_MOVES 218u

// Utility for movegen
void unpackStandardAttacks(u8 from, Bitboard attacks, Bitboard attackers, Move* moves, u32* count);
void unpackPawnAttacks(u8 from, Bitboard attacks, Move* moves, u32* count);
void unpackPawnPushes(Bitboard pushes, i8 dist, Bitboard pinned, Bitboard pinMask[64], Move* moves, u32* count);

// Legal movegen method
Move* getLegalMoves(Board* board, Move* moves, u32* count);

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // MOVEGEN_H

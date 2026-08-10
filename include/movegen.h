#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "types.h"
#include "board.h"

#if defined(__cplusplus)
extern "C" {
#endif

#define MAX_LEGAL_MOVES 218u

// Utility for movegen
void unpackQuietMoves(u8 from, Bitboard attacks, Move* moves, u32* count);
void unpackCaptures(u8 from, Bitboard attacks, Move* moves, u32* count);
void unpackPromoCaptures(u8 from, Bitboard attacks, Move* moves, u32* count);
void unpackPushes(Bitboard pushes, i8 dist, Bitboard pinMask[64], Move* moves, u32* count);
void unpackPromoPushes(Bitboard pushes, i8 dist, Bitboard pinMask[64], Move* moves, u32* count);
void unpackDoublePushes(Bitboard pushes, i8 dist, Bitboard pinMask[64], Move* moves, u32* count);

// Legal movegen method
Move* getLegalMoves(Board* board, Move* moves, u32* count);

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // MOVEGEN_H

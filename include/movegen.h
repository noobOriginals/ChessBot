#ifndef MOVEGEN_H
#define MOVEGEN_H

// Std includes
#include <stdint.h>

// Local includes
#include "board.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_LEGAL_MOVES 218
#define WHITE_LEGAL_ATT_CASTLE_K 0b0000000000000000000000000000000000000000000000000000000001110000ull
#define WHITE_LEGAL_ATT_CASTLE_Q 0b0000000000000000000000000000000000000000000000000000000000011100ull
#define BLACK_LEGAL_ATT_CASTLE_K 0b0111000000000000000000000000000000000000000000000000000000000000ull
#define BLACK_LEGAL_ATT_CASTLE_Q 0b0001110000000000000000000000000000000000000000000000000000000000ull
#define WHITE_LEGAL_OCC_CASTLE_K 0b0000000000000000000000000000000000000000000000000000000001100000ull
#define WHITE_LEGAL_OCC_CASTLE_Q 0b0000000000000000000000000000000000000000000000000000000000001110ull
#define BLACK_LEGAL_OCC_CASTLE_K 0b0110000000000000000000000000000000000000000000000000000000000000ull
#define BLACK_LEGAL_OCC_CASTLE_Q 0b0000111000000000000000000000000000000000000000000000000000000000ull

// Diagonals masks indexed by square for between() method
extern const uint64_t right45Mask[64];
extern const uint64_t left45Mask[64];

// Diagonals masks indexed by square for between() method
extern uint64_t rayTable[4096];

// Utility
void initRayTable();
uint64_t between(uint64_t a, uint64_t b);
uint32_t popLSB(uint64_t* bitboard);
uint32_t unpackAttacksBB(Board* board, uint32_t from, uint32_t pType, uint64_t movesbb, uint64_t attackers, Move* moves, uint32_t begin);
uint32_t unpackPawnPushesBB(uint64_t movesbb, int32_t offset, uint32_t flag, uint64_t pinMask[64], Move* moves, uint32_t begin);

// Movegen method
Move* generateLegalMoves(Board* board, Move* moves, uint32_t* size);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // MOVEGEN_H

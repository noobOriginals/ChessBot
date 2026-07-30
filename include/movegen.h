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

// Diagonals masks indexed by square for between() method
extern const uint64_t right45Mask[64];
extern const uint64_t left45Mask[64];

// Diagonals masks indexed by square for between() method
extern uint64_t rayTable[4096];

// Utility
void initRayTable();
uint64_t between(uint64_t a, uint64_t b);
uint32_t popLSB(uint64_t* bitboard);
uint64_t getCheckers(Board* board, uint32_t* count);
uint32_t unpackMovesBB(Board* board, uint32_t from, uint64_t movesbb, Move* moves, uint32_t begin);

// Movegen method
Move* generateLegalMoves(Board* board, uint32_t* size);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // MOVEGEN_H

#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "types.h"
#include "board.h"

#if defined(__cplusplus)
extern "C" {
#endif

#define MAX_LEGAL_MOVES 218

// Main movegen method
Move* generateLegalMoves(Board* board, Move* movesBuffer, u64* bufferSize);

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // MOVEGEN_H

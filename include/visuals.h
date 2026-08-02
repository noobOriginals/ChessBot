#ifndef VISUALS_H
#define VISUALS_H

#include "types.h"
#include "bitboard.h"
#include "board.h"

#if defined(__cplusplus)
extern "C" {
#endif

#define VISUAL_BUFFER_SIZE 761ull
#define VISUAL_BUFFER_INDEX_0 85ull
#define VISUAL_BUFFER_SQUARE_OFFSET 4ull
#define VISUAL_BUFFER_NEXT_RANK_FROM_LAST_FILE_OFFSET 48ull

char* getVisualBoard(const Board* board, char* buffer, u64 size);
char* getVisualBitboard(Bitboard bb, char* buffer, u64 size);

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // VISUALS_H

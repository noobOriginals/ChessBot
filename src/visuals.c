#include "visuals.h"

#include <stdio.h>
#include <string.h>

// Visual template that gets copied to the given buffer
static const char* boardVisualTemplate =
"     a   b   c   d   e   f   g   h     \n"
"   +---+---+---+---+---+---+---+---+   \n"
" 8 | x | y |   |   |   |   |   | z | 8 \n"
"   +---+---+---+---+---+---+---+---+   \n"
" 7 | w |   |   |   |   |   |   |   | 7 \n"
"   +---+---+---+---+---+---+---+---+   \n"
" 6 |   |   |   |   |   |   |   |   | 6 \n"
"   +---+---+---+---+---+---+---+---+   \n"
" 5 |   |   |   |   |   |   |   |   | 5 \n"
"   +---+---+---+---+---+---+---+---+   \n"
" 4 |   |   |   |   |   |   |   |   | 4 \n"
"   +---+---+---+---+---+---+---+---+   \n"
" 3 |   |   |   |   |   |   |   |   | 3 \n"
"   +---+---+---+---+---+---+---+---+   \n"
" 2 |   |   |   |   |   |   |   |   | 2 \n"
"   +---+---+---+---+---+---+---+---+   \n"
" 1 |   |   |   |   |   |   |   |   | 1 \n"
"   +---+---+---+---+---+---+---+---+   \n"
"     a   b   c   d   e   f   g   h     ";

char* getVisualBoard(const Board* board, char* buffer, u64 size) {
    if (!buffer) {
        fprintf(stderr, "getVisualBoard() failed: invalid buffer pointer\n");
        return NULL;
    }
    if (size < VISUAL_BUFFER_SIZE) {
        fprintf(stderr, "getVisualBoard() failed: invalid buffer size\n");
        return NULL;
    }
    memcpy(buffer, boardVisualTemplate, strlen(boardVisualTemplate));
    u64 i = VISUAL_BUFFER_INDEX_0;
    for (i8 rank = 7; rank > -1; rank--) {
        for (i8 file = 0; file < 8; file++) {
            u8 sq = (rank << 3) + file;
            if (board->mailbox[sq]) {
                switch (ptype(board->mailbox[sq])) {
                    case PAWN: buffer[i] = 'P'; break;
                    case KNIGHT: buffer[i] = 'N'; break;
                    case BISHOP: buffer[i] = 'B'; break;
                    case ROOK: buffer[i] = 'R'; break;
                    case QUEEN: buffer[i] = 'Q'; break;
                    case KING: buffer[i] = 'K'; break;
                    default: buffer[i] = ' '; break;
                }
                buffer[i] += (pside(board->mailbox[sq])) ? 'a' - 'A' : 0;
            } else {
                buffer[i] = ' ';
            }
            i += VISUAL_BUFFER_SQUARE_OFFSET;
        }
        i += VISUAL_BUFFER_NEXT_RANK_FROM_LAST_FILE_OFFSET;
    }
    buffer[VISUAL_BUFFER_SIZE - 1] = 0;
    return buffer;
}

char* getVisualBitboard(Bitboard bb, char* buffer, u64 size) {
    if (!buffer) {
        fprintf(stderr, "getVisualBoard() failed: invalid buffer pointer\n");
        return NULL;
    }
    if (size < VISUAL_BUFFER_SIZE) {
        fprintf(stderr, "getVisualBoard() failed: invalid buffer size\n");
        return NULL;
    }
    memcpy(buffer, boardVisualTemplate, strlen(boardVisualTemplate));
    u64 i = VISUAL_BUFFER_INDEX_0;
    for (i8 rank = 7; rank > -1; rank--) {
        for (i8 file = 0; file < 8; file++) {
            buffer[i] = (bb & bbsq((rank << 3) + file)) ? '*' : ' ';
            i += VISUAL_BUFFER_SQUARE_OFFSET;
        }
        i += VISUAL_BUFFER_NEXT_RANK_FROM_LAST_FILE_OFFSET;
    }
    buffer[VISUAL_BUFFER_SIZE - 1] = 0;
    return buffer;
}

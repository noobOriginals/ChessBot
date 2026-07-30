#include "movegen.h"

// Local includes
#include "attacks.h"

uint64_t getCheckers(Board* board, uint32_t* count) {
    *count = 0;
    uint64_t checkers = 0;
    uint32_t myOff = 0, opOff = BLACK_OFFSET;
    if (board->sideToMove) { myOff = BLACK_OFFSET; opOff = 0; }
#if defined(USE_PER_PIECE_BITBOARDS)
    uint64_t att = board->sidePieces[board->sideToMove ^ 0b1];
#else
    for (uint32_t pType = opOff; pType < BLACK_OFFSET + opOff; pType += 1) {
        uint64_t piece = board->pieces[pType];
        while (piece != 0) {
            uint32_t sq = popLSB(&piece);
            if (getPieceAttacks(pType, sq, board->occupancy) & board->pieces[KING + myOff]) {
                checkers |= (1ull << sq);
                *count += 1;
            }
            if (*count == 2) return checkers;
        }
    }
#endif
    return checkers;
}

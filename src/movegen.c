#include "movegen.h"

// Std includes
#include <stdlib.h>
#include <stdio.h>

// Local includes
#include "attacks.h"

// Diagonals masks indexed by square for between() method
uint64_t rayTable[4096];

// Utility
void initRayTable() {
    for (uint32_t a = 0; a < 64; a += 1) {
        for (uint32_t b = 0; b < 64; b += 1) {
            int32_t ra = a >> 3, fa = a & 7, rb = b >> 3, fb = b & 7;
            int32_t dr = rb - ra, df = fb - fa;
            rayTable[a * 64 + b] = 0;
            if ((dr < 2 && dr > -2) && (df < 2 && df > -2)) continue;
            int32_t dir = 0;
            if (dr == 0) dir = (df < 0) ? -1 : 1;
            else if (df == 0) dir = (dr < 0) ? -8 : 8;
            else if (dr == df) dir = (dr < 0) ? -9 : 9;
            else if (dr == -df) dir = (dr < 0) ? -7 : 7;
            else continue;
            for (uint32_t s = a + dir; s != b; s += dir) rayTable[a * 64 + b] |= (1ull << s);
        }
    }
}

uint64_t between(uint64_t a, uint64_t b) {
    if (!a || !b) return ~0b0ull;
    return rayTable[ctzll(a) * 64 + ctzll(b)];
}

uint32_t popLSB(uint64_t* bitboard) {
    uint32_t sq = ctzll(*bitboard);
    *bitboard ^= (1ull << sq);
    return sq;
}

uint32_t unpackAttacksBB(Board* board, uint32_t from, uint64_t movesbb, Move* moves, uint32_t begin) {
    while (movesbb != 0) {
        uint32_t to = popLSB(&movesbb);
        uint32_t flag = MOVE_QUIET;

        uint32_t distance = (to < from) ? from - to : to - from; // Distance between squares (always positive)
        uint32_t sideOffset = board->sideToMove ? BLACK_OFFSET : 0; // Offset for handling different sides
        uint32_t pType = board->mailbox[from]; // Piece type
        uint32_t capType = board->mailbox[to]; // Capture type

        if (capType != NO_PIECE) flag = MOVE_CAPTURE; // Standard piece captures

        // In case of pawn
        if (pType - sideOffset == PAWN) {
            if (to == board->epTarget) flag = MOVE_EP_CAPTURE; // TO square is ep target -> en passant
            else if ((to >> 3) == 7 || (to >> 3) == 0) { // Pawn reached last rank
                moves[begin] = (Move) from | ((Move) to << 6) | ((Move) MOVE_PROMO_CAPTURE_N << 12);
                moves[begin + 1] = (Move) from | ((Move) to << 6) | ((Move) MOVE_PROMO_CAPTURE_B << 12);
                moves[begin + 2] = (Move) from | ((Move) to << 6) | ((Move) MOVE_PROMO_CAPTURE_R << 12);
                moves[begin + 3] = (Move) from | ((Move) to << 6) | ((Move) MOVE_PROMO_CAPTURE_Q << 12);
                begin += 4;
                continue;
            }
        }

        if (pType - sideOffset == KING && distance == 2) flag = (to < from) ? MOVE_CASTLE_Q : MOVE_CASTLE_K; // King and distance = 2 -> castle

        moves[begin] = (Move) from | ((Move) to << 6) | ((Move) flag << 12);
        begin += 1;
    }
    return begin;
}

uint32_t unpackPawnPushesBB(uint64_t movesbb, int32_t offset, uint32_t flag, Move* moves, uint32_t begin) {
    while (movesbb != 0) {
        uint32_t to = popLSB(&movesbb);
        uint32_t from = to + offset;
        if ((to >> 3) == 7 || (to >> 3) == 0) { // Pawn reached last rank
            moves[begin] = (Move) from | ((Move) to << 6) | ((Move) MOVE_PROMO_N << 12);
            moves[begin + 1] = (Move) from | ((Move) to << 6) | ((Move) MOVE_PROMO_B << 12);
            moves[begin + 2] = (Move) from | ((Move) to << 6) | ((Move) MOVE_PROMO_R << 12);
            moves[begin + 3] = (Move) from | ((Move) to << 6) | ((Move) MOVE_PROMO_Q << 12);
            begin += 4;
            continue;
        }
        moves[begin] = (Move) from | ((Move) to << 6) | ((Move) flag << 12);
        begin += 1;
    }
    return begin;
}

// Movegen method
Move* generateLegalMoves(Board* board, Move* moves, uint32_t* size) {
    *size = 0;
    uint32_t myOff = (board->sideToMove ? BLACK_OFFSET : 0);
    uint64_t checkers = 0, attacked = 0, kingbb = board->pieces[KING + myOff];

#if defined(USE_PER_PIECE_BITBOARDS)
    uint64_t defenders = board->sidePieces[board->sideToMove], attackers = board->sidePieces[board->sideToMove ^ 0b1];
#else
    uint64_t defenders = 0, attackers = 0;
    for (uint32_t bb = 0; bb < TOTAL_PIECE_TYPES; bb += 1) {
        if ((bb >= BLACK_OFFSET) == board->sideToMove) {
            defenders |= board->pieces[bb];
        } else {
            attackers |= board->pieces[bb];
        }
    }
#endif

    // Go thrugh each bit of attackers to compute both attacked squares and checkers
    uint64_t attackersCopy = attackers;
    while (attackersCopy != 0) {
        uint32_t sq = popLSB(&attackersCopy);
        uint64_t attacks = getPieceAttacks(board->mailbox[sq], sq, board->occupancy & ~kingbb);
        attacked |= attacks;
        if (attacks & kingbb) checkers |= (1ull << sq);
    }

    // If double check
    if (popcountll(checkers) == 2) {
        uint32_t kingsq = ctzll(kingbb);
        uint64_t legal = getKingAttacks(kingsq) & ~defenders & ~attacked;
        *size = unpackAttacksBB(board, kingsq, legal, moves, *size);
        return moves;
    }

    uint64_t target = between(checkers, kingbb) | checkers;
    uint64_t defendersCopy = defenders;
    while (defendersCopy != 0) {
        uint32_t sq = popLSB(&defendersCopy);
        uint64_t legal = ~defenders;
        switch (board->mailbox[sq] - myOff) {
            case PAWN: legal &= getPawnAttacks(board->sideToMove, sq) & attackers & target & (1ull << board->epTarget); break;
            case KNIGHT: legal &= getKnightAttacks(sq) & target; break;
            case BISHOP: legal &= getBishopAttacks(sq, board->occupancy) & target; break;
            case ROOK: legal &= getRookAttacks(sq, board->occupancy) & target; break;
            case QUEEN: legal &= getQueenAttacks(sq, board->occupancy) & target; break;
            case KING: legal &= getKingAttacks(sq) & ~attacked; break;
        }
        *size = unpackAttacksBB(board, sq, legal, moves, *size);
    }

    // Generate pawn pushes
    uint64_t singlePush = getPawnPushes(board->sideToMove, board->pieces[myOff], board->occupancy) & target;
    uint64_t doublePush = getPawnPushes(board->sideToMove, singlePush & (board->sideToMove ? RANK_6 : RANK_3), board->occupancy) & target;
    int32_t offset = board->sideToMove ? 8 : -8;
    *size = unpackPawnPushesBB(singlePush, offset, MOVE_QUIET, moves, *size);
    *size = unpackPawnPushesBB(doublePush, offset * 2, MOVE_DOUBLE_PUSH, moves, *size);

    return moves;
}

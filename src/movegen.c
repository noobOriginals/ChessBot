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

uint32_t unpackAttacksBB(Board* board, uint32_t from, uint32_t pType, uint64_t movesbb, uint64_t attackers, Move* moves, uint32_t begin) {
    while (movesbb != 0ull) {
        uint32_t to = popLSB(&movesbb);
        uint32_t flag = MOVE_QUIET;

        uint32_t distance = (to < from) ? from - to : to - from; // Distance between squares (always positive)
        uint32_t capType = board->mailbox[to]; // Capture type

        if (capType != NO_PIECE) flag = MOVE_CAPTURE; // Standard piece captures

        // In case of pawn
        if (pType == PAWN) {
            if (to == board->epTarget) { // TO square is ep target -> en passant
                uint32_t sideOffset = board->sideToMove ? BLACK_OFFSET : 0, opOffset = board->sideToMove ? 0 : BLACK_OFFSET;
                uint64_t kingbb = board->pieces[KING + sideOffset];
                uint64_t rembb = (1ull << from) | (1ull << (to + epCaptureOffset[board->sideToMove]));
                uint64_t attacked = 0ull, boardOcc = (board->occupancy | (1ull << to)) & ~rembb, attCopy = attackers;
                while (attCopy != 0ull) {
                    uint32_t sq = popLSB(&attCopy);
                    switch (board->mailbox[sq] - opOffset) {
                        case BISHOP: attacked |= getBishopAttacks(sq, boardOcc); break;
                        case ROOK: attacked |= getRookAttacks(sq, boardOcc); break;
                        case QUEEN: attacked |= getQueenAttacks(sq, boardOcc); break;
                        default: break;
                    }
                }
                if (attacked & kingbb) continue;
                flag = MOVE_EP_CAPTURE;
            } else if ((to >> 3) == 7 || (to >> 3) == 0) { // Pawn reached last rank
                moves[begin] = (Move) from | ((Move) to << 6) | ((Move) MOVE_PROMO_CAPTURE_N << 12);
                moves[begin + 1] = (Move) from | ((Move) to << 6) | ((Move) MOVE_PROMO_CAPTURE_B << 12);
                moves[begin + 2] = (Move) from | ((Move) to << 6) | ((Move) MOVE_PROMO_CAPTURE_R << 12);
                moves[begin + 3] = (Move) from | ((Move) to << 6) | ((Move) MOVE_PROMO_CAPTURE_Q << 12);
                begin += 4;
                continue;
            }
        } else if (pType == KING && distance == 2) flag = (to < from) ? MOVE_CASTLE_Q : MOVE_CASTLE_K; // King and distance = 2 -> castle

        moves[begin] = (Move) from | ((Move) to << 6) | ((Move) flag << 12);
        begin += 1;
    }
    return begin;
}

uint32_t unpackPawnPushesBB(uint64_t movesbb, int32_t offset, uint32_t flag, uint64_t pinMask[64], Move* moves, uint32_t begin) {
    while (movesbb != 0ull) {
        uint32_t to = ctzll(movesbb);
        uint64_t tobb = (1ull << to);
        movesbb ^= tobb;
        uint32_t from = to + offset;
        if (pinMask[from] & tobb) continue;
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
    uint32_t myOff = (board->sideToMove ? BLACK_OFFSET : 0), opOff = (board->sideToMove ? 0 : BLACK_OFFSET);
    uint64_t checkers = 0ull, attacked = 0ull, kingbb = board->pieces[KING + myOff];

#if defined(USE_PER_PIECE_BITBOARDS)
    uint64_t defenders = board->sidePieces[board->sideToMove], attackers = board->sidePieces[board->sideToMove ^ 0b1];
#else
    uint64_t defenders = 0ull, attackers = 0ull;
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
    while (attackersCopy != 0ull) {
        uint32_t sq = popLSB(&attackersCopy);
        uint64_t attacks = 0ull;
        switch (board->mailbox[sq] - opOff) {
            case PAWN: attacks = getPawnAttacks(board->sideToMove ^ 0b1, sq); break;
            case KNIGHT: attacks = getKnightAttacks(sq); break;
            case BISHOP: attacks = getBishopAttacks(sq, board->occupancy & ~kingbb); break;
            case ROOK: attacks = getRookAttacks(sq, board->occupancy & ~kingbb); break;
            case QUEEN: attacks = getQueenAttacks(sq, board->occupancy & ~kingbb); break;
            case KING: attacks = getKingAttacks(sq); break;
            default: break;
        }
        attacked |= attacks;
        if (attacks & kingbb) checkers |= (1ull << sq);
    }

    // If double check
    if (popcountll(checkers) == 2) {
        uint32_t kingsq = ctzll(kingbb);
        uint64_t legal = getKingAttacks(kingsq) & ~defenders & ~attacked;
        *size = unpackAttacksBB(board, kingsq, KING, legal, attackers, moves, *size);
        return moves;
    }

    uint64_t pinMask[64] = {};
    attackersCopy = attackers;
    while (attackersCopy != 0ull) {
        uint32_t sq = popLSB(&attackersCopy);
        uint64_t attacks = 0ull;
        switch (board->mailbox[sq] - opOff) {
            case BISHOP: attacks = getBishopAttacks(sq, 0ull); break;
            case ROOK: attacks = getRookAttacks(sq, 0ull); break;
            case QUEEN: attacks = getQueenAttacks(sq, 0ull); break;
            default: break;
        }
        if (attacks & kingbb) {
            uint64_t attacker = (1ull << sq);
            uint64_t betweenbb = between(attacker, kingbb);
            uint64_t pinned = betweenbb & board->occupancy;
            if (popcountll(pinned) == 1 && (pinned & defenders)) pinMask[ctzll(pinned)] = ~(betweenbb | attacker);
        }
    }

    uint64_t castleMask = 0ull;
    switch (board->sideToMove) {
    case WHITE:
        if (!(WHITE_LEGAL_ATT_CASTLE_K & attacked) && !(WHITE_LEGAL_OCC_CASTLE_K & board->occupancy) && (board->castle & WHITE_KC)) castleMask |= (1ull << 6);
        if (!(WHITE_LEGAL_ATT_CASTLE_Q & attacked) && !(WHITE_LEGAL_OCC_CASTLE_Q & board->occupancy) && (board->castle & WHITE_QC)) castleMask |= (1ull << 2);
        break;

    case BLACK:
        if (!(BLACK_LEGAL_ATT_CASTLE_K & attacked) && !(BLACK_LEGAL_OCC_CASTLE_K & board->occupancy) && (board->castle & BLACK_KC)) castleMask |= (1ull << 62);
        if (!(BLACK_LEGAL_ATT_CASTLE_Q & attacked) && !(BLACK_LEGAL_OCC_CASTLE_Q & board->occupancy) && (board->castle & BLACK_QC)) castleMask |= (1ull << 58);
        break;

    default: break;
    }

    uint64_t target = between(checkers, kingbb) | checkers, epTarget = (board->epTarget < NO_EP_TARGET) ? (1ull << board->epTarget) : 0ull;
    uint64_t pawnTarget = target | ((checkers & (board->sideToMove ? epTarget << 8 : epTarget >> 8)) ? epTarget : 0ull);
    uint64_t defendersCopy = defenders;
    while (defendersCopy != 0ull) {
        uint32_t sq = popLSB(&defendersCopy), pType = board->mailbox[sq] - myOff;
        uint64_t legal = ~defenders & ~pinMask[sq];
        switch (pType) {
            case PAWN: legal &= getPawnAttacks(board->sideToMove, sq) & (attackers | epTarget) & pawnTarget; break;
            case KNIGHT: legal &= getKnightAttacks(sq) & target; break;
            case BISHOP: legal &= getBishopAttacks(sq, board->occupancy) & target; break;
            case ROOK: legal &= getRookAttacks(sq, board->occupancy) & target; break;
            case QUEEN: legal &= getQueenAttacks(sq, board->occupancy) & target; break;
            case KING: legal &= (getKingAttacks(sq) | castleMask) & ~attacked; break;
            default: break;
        }
        *size = unpackAttacksBB(board, sq, pType, legal, attackers, moves, *size);
    }

    // Generate pawn pushes
    uint64_t singlePush = getPawnPushes(board->sideToMove, board->pieces[myOff], board->occupancy);
    uint64_t doublePush = getPawnPushes(board->sideToMove, singlePush & (board->sideToMove ? RANK_6 : RANK_3), board->occupancy);
    int32_t offset = board->sideToMove ? 8 : -8;
    *size = unpackPawnPushesBB(singlePush & target, offset, MOVE_QUIET, pinMask, moves, *size);
    *size = unpackPawnPushesBB(doublePush & target, offset * 2, MOVE_DOUBLE_PUSH, pinMask, moves, *size);
    return moves;
}

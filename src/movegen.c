#include "movegen.h"

#include "bitboard.h"
#include "attacks.h"
#include "debug_utils.h"

void unpackStandardAttacks(u8 from, Bitboard attacks, Bitboard attackers, Move* moves, u32* count) {

    PUSH_STACK_TRACE("unpackStandardAttacks()");

    Bitboard lsb;
    u8 to, flag;
    u32 begin = *count;
    while (attacks) {
        to = popToLSB(&attacks, &lsb);
        flag = lsb & attackers ? MOVE_CAPTURE : MOVE_QUIET;
        moves[begin++] = (Move) from | ((Move) to << 6) | ((Move) flag << 12);
    }
    *count = begin;

    POP_STACK_TRACE();
}

void unpackPawnPushes(Bitboard pushes, i8 dist, Bitboard pinned, Bitboard pinMask[64], Move* moves, u32* count) {

    PUSH_STACK_TRACE("unpackStandardAttacks()");

    Bitboard lsb;
    u8 from, to, flag;
    u32 begin = *count;
    while (pushes) {
        to = popToLSB(&pushes, &lsb);
        from = to - dist;
        flag = abs(dist) > 8 ? MOVE_DOUBLE_PUSH : MOVE_QUIET;

        // Check for pins
        if (pinned & bbsq(from) && ~pinMask[from] & lsb) {
            continue;
        }

        moves[begin++] = (Move) from | ((Move) to << 6) | ((Move) flag << 12);
    }
    *count = begin;

    POP_STACK_TRACE();
}

// Main movegen method
Move* getLegalMoves(Board* board, Move* moves, u32* count) {

    PUSH_STACK_TRACE("getLegalMoves()");

    // Reset count
    *count = 0;

    // Read once from board
    u8 side = board->side, opp = board->side ^ 1u;
    Bitboard defenders = board->bb[side], notDefenders = ~defenders;
    Bitboard attackers = board->bb[opp], notAttackers = ~attackers;
    Bitboard king = board->bb[KING | side], notKing = ~king, allExceptKing = board->all & notKing;
    u8 kingsq = (u8) ctzll(king);

    // Declare useful variables
    Bitboard lsb = 0ull, attacks = 0ull, target = 0ull, pawnTarget = 0ull, bb2 = 0ull;
    u8 square = 0u, piece = 0u, i0 = 0u, i1 = 0u, i2 = 0u;

    // Compute
    Bitboard checkers = 0ull, oppSliders = 0ull, attacked = 0ull, pinned = 0ull, pinMask[64];

    while (attackers) {
        attacks = 0ull;
        target = 0ull;

        // Get attacks
        square = popToLSB(&attackers, &lsb);
        switch (ptype(board->mailbox[square])) {
            case PAWN: attacks = getPawnAttacks(square, opp); break;
            case KNIGHT: attacks = getKnightAttacks(square); break;
            case BISHOP: attacks = getBishopAttacks(square, allExceptKing); target = getBishopAttacks(square, 0ull); oppSliders |= lsb; break;
            case ROOK: attacks = getRookAttacks(square, allExceptKing); target = getRookAttacks(square, 0ull); oppSliders |= lsb; break;
            case QUEEN: attacks = getQueenAttacks(square, allExceptKing); target = getQueenAttacks(square, 0ull); oppSliders |= lsb; break;
            case KING: attacks = getKingAttacks(square); break;
            default: ASSERT_MSG(0, "getLegalMoves() failed: invalid piece type"); break;
        }
        attacked |= attacks;

        // Checkers
        if (attacks & king) {
            checkers |= lsb;
        }

        // Pinned pieces
        if (target & king) {
            pawnTarget = betweenSQ(square, kingsq);
            bb2 = pawnTarget & board->all;
            if (popcountll(bb2) == 1ull && (bb2 & defenders)) {
                pinned |= bb2;
                pinMask[ctzll(bb2)] = pawnTarget | lsb;
            }
        }
    }
    attackers = board->bb[opp];

    // Double check only allows king moves
    if (popcountll(checkers) > 1) {
        attacks = getKingAttacks(kingsq) & ~attacked & notDefenders;
        unpackStandardAttacks(kingsq, attacks, attackers, moves, count);

        POP_STACK_TRACE();

        return moves;
    }

    target = checkers ? betweenSQ((u8) ctzll(checkers), kingsq) | checkers : notDefenders;
    pawnTarget = target & attackers;
    while (defenders) {
        attacks = 0ull;

        // Get attacks
        square = popToLSB(&defenders, &lsb);
        switch (ptype(board->mailbox[square])) {
            case PAWN: attacks = getPawnAttacks(square, side) & pawnTarget; break;
            case KNIGHT: attacks = getKnightAttacks(square) & target; break;
            case BISHOP: attacks = getBishopAttacks(square, board->all) & target; break;
            case ROOK: attacks = getRookAttacks(square, board->all) & target; break;
            case QUEEN: attacks = getQueenAttacks(square, board->all) & target; break;
            case KING: attacks = getKingAttacks(square) & ~attacked & notDefenders; break;
            default: ASSERT_MSG(0, "getLegalMoves() failed: invalid piece type"); break;
        }

        // Update pin mask if piece is not pinned
        if (~pinned & lsb) {
            pinMask[square] = ~0ull;
        }

        // Unpack into move array
        unpackStandardAttacks(square, attacks & pinMask[square], attackers, moves, count);
    }

    Bitboard singlePush = getPawnPushes(board->bb[PAWN | side], board->all, side);
    Bitboard doublePush = getPawnPushes(singlePush & (side ? RANK_6 : RANK_3), board->all, side);
    unpackPawnPushes(singlePush & target, side ? -8 : 8, pinned, pinMask, moves, count);
    unpackPawnPushes(doublePush & target, side ? -16 : 16, pinned, pinMask, moves, count);

    POP_STACK_TRACE();

    return moves;
}

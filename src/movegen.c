#include "movegen.h"

#include "bitboard.h"
#include "attacks.h"
#include "debug_utils.h"

// From attack bitboards to moves
static void unpackStandardAttacks(u8 from, Bitboard attacks, Bitboard attackers, Move* moves, u64* begin) {

    PUSH_STACK_TRACE("unpackStandardAttacks()");

    Bitboard tobb;
    u8 to, flag;
    u64 index = *begin;
    while (attacks) {
        to = popToLSB(&attacks, &tobb);
        flag = tobb & attackers ? MOVE_CAPTURE : MOVE_QUIET;
        moves[index++] = (Move) from | ((Move) to << 6) | ((Move) flag << 12);
    }
    *begin = index;

    POP_STACK_TRACE();
}

static void unpackPawnAttacks(u8 from, Bitboard attacks, u8 mailbox[64], Bitboard epTarget, Move* moves, u64* begin) {

    PUSH_STACK_TRACE("unpackPawnAttacks()");

    Bitboard tobb;
    u8 to, flag;
    u64 index = *begin;
    while (attacks) {
        to = popToLSB(&attacks, &tobb);
        flag = tobb & (RANK_1 | RANK_8) ? (MOVE_PROMO_CAPTURE_N - (KNIGHT >> 1)) + (mailbox[to] >> 1) : (tobb & epTarget ? MOVE_EP_CAPTURE : MOVE_CAPTURE);
        moves[index++] = (Move) from | ((Move) to << 6) | ((Move) flag << 12);
    }
    *begin = index;

    POP_STACK_TRACE();
}

static void unpackPawnPushes(Bitboard pushes, i8 dist, Move* moves, u64* begin) {

    PUSH_STACK_TRACE("unpackPawnPushes()");

    Bitboard tobb;
    u8 from, to;
    u64 index = *begin;
    while (pushes) {
        to = popToLSB(&pushes, &tobb);
        from = to - dist;
        if (tobb & (RANK_1 | RANK_8)) {
            moves[index++] = (Move) from | ((Move) to << 6) | ((Move) MOVE_PROMO_N << 12);
            moves[index++] = (Move) from | ((Move) to << 6) | ((Move) MOVE_PROMO_B << 12);
            moves[index++] = (Move) from | ((Move) to << 6) | ((Move) MOVE_PROMO_R << 12);
            moves[index++] = (Move) from | ((Move) to << 6) | ((Move) MOVE_PROMO_Q << 12);
            continue;
        }
        moves[index++] = (Move) from | ((Move) to << 6) | ((Move) MOVE_QUIET << 12);
    }
    *begin = index;

    POP_STACK_TRACE();
}

static void unpackPawnDoublePushes(Bitboard pushes, i8 dist, Move* moves, u64* begin) {

    PUSH_STACK_TRACE("unpackPawnDoublePushes()");

    u8 to;
    u64 index = *begin;
    while (pushes) {
        to = popLSB(&pushes);
        moves[index++] = (Move) (to - dist) | ((Move) to << 6) | ((Move) MOVE_DOUBLE_PUSH << 12);
    }
    *begin = index;

    POP_STACK_TRACE();
}

// Main movegen method
Move* generateLegalMoves(Board* board, Move* movesBuffer, u64* bufferSize) {

    PUSH_STACK_TRACE("generateLegalMoves()");

    Bitboard defenders = board->bb[board->side];
    Bitboard attackers = board->bb[board->side ^ 1u];

    Bitboard target = ~defenders;
    u8 sq;
    *bufferSize = 0;
    while (defenders) {
        sq = popLSB(&defenders);
        switch (ptype(board->mailbox[sq])) {
            case PAWN: unpackPawnAttacks(sq, getPawnAttacks(sq, board->side) & (attackers | board->epTarget), board->mailbox, board->epTarget, movesBuffer, bufferSize); break;
            case KNIGHT: unpackStandardAttacks(sq, getKnightAttacks(sq) & target, attackers, movesBuffer, bufferSize); break;
            case BISHOP: unpackStandardAttacks(sq, getBishopAttacks(sq, board->all) & target, attackers, movesBuffer, bufferSize); break;
            case ROOK: unpackStandardAttacks(sq, getRookAttacks(sq, board->all) & target, attackers, movesBuffer, bufferSize); break;
            case QUEEN: unpackStandardAttacks(sq, getQueenAttacks(sq, board->all) & target, attackers, movesBuffer, bufferSize); break;
            case KING: unpackStandardAttacks(sq, getKingAttacks(sq) & target, attackers, movesBuffer, bufferSize); break;
            default: ASSERT_MSG(0, "generateLegalMoves() failed: invalid piece type"); break;
        }
    }

    Bitboard singlePushes = getPawnPushes(board->bb[PAWN | board->side], board->all, board->side);
    Bitboard doublePushes = getPawnPushes(singlePushes & (board->side ? RANK_6 : RANK_3), board->all, board->side);
    unpackPawnPushes(singlePushes, board->side ? -8 : 8, movesBuffer, bufferSize);
    unpackPawnDoublePushes(doublePushes, board->side ? -16 : 16, movesBuffer, bufferSize);

    POP_STACK_TRACE();

    return movesBuffer;
}

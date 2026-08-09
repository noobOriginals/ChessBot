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

static void unpackPawnAttacks(u8 from, Bitboard attacks, Board* board, Bitboard slidingAttackers, Bitboard kingbb, Bitboard epPawn, Move* moves, u64* begin) {

    PUSH_STACK_TRACE("unpackPawnAttacks()");

    Bitboard tobb, attacked;
    u8 sq, to, flag;
    u64 index = *begin;
    while (attacks) {
        to = popToLSB(&attacks, &tobb);

        // Check for en passant, which if present needs and additional pin check
        if (tobb & board->epTarget) {
            flag = MOVE_EP_CAPTURE;
            tobb |= bbsq(from) | epPawn;
            tobb = board->all ^ tobb;
            attacked = 0ull;
            while (slidingAttackers) {
                sq = popLSB(&slidingAttackers);

                // Some assertions for catching bugs
                ASSERT_MSG(board->mailbox[sq] > 1 && board->mailbox[sq] < 14, "unpackPawnAttacks() failed: piece type out of bounds");
                ASSERT_MSG(sq < 64, "unpackPawnAttacks() failed: square index out of bounds");

                switch (ptype(board->mailbox[sq])) {
                    case BISHOP: attacked |= getBishopAttacks(sq, tobb); break;
                    case ROOK: attacked |= getRookAttacks(sq, tobb); break;
                    case QUEEN: attacked |= getQueenAttacks(sq, tobb); break;
                    default: ASSERT_MSG(0, "unpackPawnAttacks() failed: invalid piece type"); break;
                }
            }
            if (attacked & kingbb) {
                continue;
            }
        } else {
            flag = tobb & (RANK_1 | RANK_8) ? (MOVE_PROMO_CAPTURE_N - (KNIGHT >> 1)) + (board->mailbox[to] >> 1) : MOVE_CAPTURE;
        }

        moves[index++] = (Move) from | ((Move) to << 6) | ((Move) flag << 12);
    }
    *begin = index;

    POP_STACK_TRACE();
}

static void unpackPawnPushes(Bitboard pushes, i8 dist, Bitboard pinned, Bitboard pinMask[64], Move* moves, u64* begin) {

    PUSH_STACK_TRACE("unpackPawnPushes()");

    Bitboard tobb;
    u8 from, to;
    u64 index = *begin;
    while (pushes) {
        to = popToLSB(&pushes, &tobb);
        from = to - dist;

        // Check for pins
        if (pinned & bbsq(from) && !(pinMask[from] & tobb)) {
            continue;
        }

        // Promotion
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

static void unpackPawnDoublePushes(Bitboard pushes, i8 dist, Bitboard pinned, Bitboard pinMask[64], Move* moves, u64* begin) {

    PUSH_STACK_TRACE("unpackPawnDoublePushes()");

    Bitboard tobb;
    u8 from, to;
    u64 index = *begin;
    while (pushes) {
        to = popToLSB(&pushes, &tobb);
        from = to - dist;

        // Check for pins
        if (pinned & bbsq(from) && !(pinMask[from] & tobb)) {
            continue;
        }

        moves[index++] = (Move) from | ((Move) to << 6) | ((Move) MOVE_DOUBLE_PUSH << 12);
    }
    *begin = index;

    POP_STACK_TRACE();
}

// Main movegen method
Move* generateLegalMoves(Board* board, Move* movesBuffer, u64* bufferSize) {

    PUSH_STACK_TRACE("generateLegalMoves()");

    // Reset buffer size
    *bufferSize = 0;

    // Declare universal variables
    Bitboard lsb, attacks, target, pawnTarget, kingTarget;
    u8 square;

    // Read once from board
    u8 side = board->side, opp = board->side ^ 1u;
    Bitboard kingbb = board->bb[KING | side], notKingbb = ~kingbb;
    Bitboard defenders = board->bb[side], notDefenders = ~defenders;
    Bitboard attackers = board->bb[opp];
    Bitboard epPawn = epPawnMask[((ctzll(board->epTarget) & 7) << 1) | side];
    u8 king = ctzll(kingbb);

    // Compute the following
    Bitboard attacked = 0ull, checkers = 0ull, pinned = 0ull, pinMask[64], slidingAttackers = 0ull;
    while (attackers) {
        // Read square and bitboard
        square = popToLSB(&attackers, &lsb);

        // Some assertions for catching bugs
        ASSERT_MSG(board->mailbox[square] > 1 && board->mailbox[square] < 14, "generateLegalMoves() failed: opp piece type out of bounds");
        ASSERT_MSG(square < 64, "generateLegalMoves() failed: opp square index out of bounds");
        ASSERT_MSG(opp < 2, "generateLegalMoves() failed: opp side can only have the value 0 (zero) or 1 (one)");

        // Get piece attakcs
        switch (ptype(board->mailbox[square])) {
        case PAWN:
            attacks = getPawnAttacks(square, opp);
            target = 0ull;
            break;

        case KNIGHT:
            attacks = getKnightAttacks(square);
            target = 0ull;
            break;

        case BISHOP:
            slidingAttackers |= lsb;
            attacks = getBishopAttacks(square, board->all & notKingbb);
            target = getBishopAttacks(square, 0ull);
            break;

        case ROOK:
            slidingAttackers |= lsb;
            attacks = getRookAttacks(square, board->all & notKingbb);
            target = getRookAttacks(square, 0ull);
            break;

        case QUEEN:
            slidingAttackers |= lsb;
            attacks = getQueenAttacks(square, board->all & notKingbb);
            target = getQueenAttacks(square, 0ull);
            break;

        case KING:
            attacks = getKingAttacks(square);
            target = 0ull;
            break;

        default: break;
        }

        // Register attacks
        attacked |= attacks;

        // Register checkers
        if (attacks & kingbb) {
            checkers |= lsb;
        }

        // If piece is slider, target (in this case, piece attacks with empty board) is computed for registering pinned pieces
        if (target & kingbb) {
            attacks = betweenSQ(square, king);
            target = attacks & board->all;
            if (popcountll(target) == 1 && (target & defenders)) {
                pinned |= target;
                pinMask[ctzll(target)] = attacks | lsb;
            }
        }
    }
    attackers = board->bb[opp]; // Reset attackers bb

    // Compute king target (king can move anywhere where it is not attacked)
    kingTarget = ~attacked & notDefenders;

    // Check if multiple checkers, then generate only king moves
    if (popcountll(checkers) > 1) {
        unpackStandardAttacks(king, getKingAttacks(king) & kingTarget, attackers, movesBuffer, bufferSize);

        POP_STACK_TRACE();

        return movesBuffer;
    }

    // Compute target bitboards
    target = checkers ? betweenSQ(ctzll(checkers), king) | checkers : notDefenders;
    pawnTarget = (attackers & target) | (target & epPawn ? board->epTarget : 0ull);

    while (defenders) {
        // Read square and bitboard
        square = popToLSB(&defenders, &lsb);

        // Some assertions for catching bugs
        ASSERT_MSG(board->mailbox[square] > 1 && board->mailbox[square] < 14, "generateLegalMoves() failed: piece type out of bounds");
        ASSERT_MSG(square < 64, "generateLegalMoves() failed: square index out of bounds");
        ASSERT_MSG(opp < 2, "generateLegalMoves() failed: side can only have the value 0 (zero) or 1 (one)");

        // If piece is pinned
        if (pinned & lsb) {
            attacks = pinMask[square];
        } else {
            attacks = ~0ull;
        }

        // Get leagl attacks
        switch (ptype(board->mailbox[square])) {
        case PAWN:
            // For pawns use a separate method as the possible flags for pawn moves are different
            attacks &= getPawnAttacks(square, side) & pawnTarget;
            unpackPawnAttacks(square, attacks, board, slidingAttackers, kingbb, epPawn, movesBuffer, bufferSize);
            continue;

        case KNIGHT:
            attacks &= getKnightAttacks(square) & target;
            break;

        case BISHOP:
            attacks &= getBishopAttacks(square, board->all) & target;
            break;

        case ROOK:
            attacks &= getRookAttacks(square, board->all) & target;
            break;

        case QUEEN:
            attacks &= getQueenAttacks(square, board->all) & target;
            break;

        case KING:
            attacks &= getKingAttacks(square) & kingTarget;
            break;

        default: break;
        }

        unpackStandardAttacks(square, attacks, attackers, movesBuffer, bufferSize);
    }

    Bitboard singlePushes = getPawnPushes(board->bb[PAWN | board->side], board->all, board->side);
    Bitboard doublePushes = getPawnPushes(singlePushes & (board->side ? RANK_6 : RANK_3), board->all, board->side);
    unpackPawnPushes(singlePushes & target, board->side ? -8 : 8, pinned, pinMask, movesBuffer, bufferSize);
    unpackPawnDoublePushes(doublePushes & target, board->side ? -16 : 16, pinned, pinMask, movesBuffer, bufferSize);

    POP_STACK_TRACE();

    return movesBuffer;
}

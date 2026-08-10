#include "movegen.h"

#include "bitboard.h"
#include "attacks.h"
#include "debug_utils.h"

void unpackQuietMoves(u8 from, Bitboard attacks, Move* moves, u32* count) {

    PUSH_STACK_TRACE("unpackQuietMoves()");

    u32 begin = *count;
    while (attacks) {
        moves[begin++] = (Move) from | ((Move) popLSB(&attacks) << 6) | ((Move) MOVE_QUIET << 12);
    }
    *count = begin;

    POP_STACK_TRACE();
}

void unpackCaptures(u8 from, Bitboard attacks, Move* moves, u32* count) {

    PUSH_STACK_TRACE("unpackCaptures()");

    u32 begin = *count;
    while (attacks) {
        moves[begin++] = (Move) from | ((Move) popLSB(&attacks) << 6) | ((Move) MOVE_CAPTURE << 12);
    }
    *count = begin;

    POP_STACK_TRACE();
}

void unpackPromoCaptures(u8 from, Bitboard attacks, Move* moves, u32* count) {

    PUSH_STACK_TRACE("unpackPromoCaptures()");

    Move base;
    u32 begin = *count;
    while (attacks) {
        base = (Move) from | ((Move) popLSB(&attacks) << 6);
        moves[begin++] = base | ((Move) MOVE_PROMO_CAPTURE_N << 12);
        moves[begin++] = base | ((Move) MOVE_PROMO_CAPTURE_B << 12);
        moves[begin++] = base | ((Move) MOVE_PROMO_CAPTURE_R << 12);
        moves[begin++] = base | ((Move) MOVE_PROMO_CAPTURE_Q << 12);
    }
    *count = begin;

    POP_STACK_TRACE();
}

void unpackPushes(Bitboard pushes, i8 dist, Bitboard pinned, Bitboard pinMask[64], Move* moves, u32* count) {

    PUSH_STACK_TRACE("unpackPushes()");

    Bitboard lsb;
    u8 from, to;
    u32 begin = *count;
    while (pushes) {
        to = popToLSB(&pushes, &lsb);
        from = to - dist;

        // Check for pins
        if (pinned & bbsq(from) && ~pinMask[from] & lsb) {
            continue;
        }

        moves[begin++] = (Move) from | ((Move) to << 6) | ((Move) MOVE_QUIET << 12);
    }
    *count = begin;

    POP_STACK_TRACE();
}

void unpackPromoPushes(Bitboard pushes, i8 dist, Bitboard pinned, Bitboard pinMask[64], Move* moves, u32* count) {

    PUSH_STACK_TRACE("unpackPromoPushes()");

    Bitboard lsb;
    u8 from, to;
    Move base;
    u32 begin = *count;
    while (pushes) {
        to = popToLSB(&pushes, &lsb);
        from = to - dist;

        // Check for pins
        if (pinned & bbsq(from) && ~pinMask[from] & lsb) {
            continue;
        }

        base = (Move) from | ((Move) to << 6);
        moves[begin++] = base | ((Move) MOVE_PROMO_N << 12);
        moves[begin++] = base | ((Move) MOVE_PROMO_B << 12);
        moves[begin++] = base | ((Move) MOVE_PROMO_R << 12);
        moves[begin++] = base | ((Move) MOVE_PROMO_Q << 12);
    }
    *count = begin;

    POP_STACK_TRACE();
}

void unpackDoublePushes(Bitboard pushes, i8 dist, Bitboard pinned, Bitboard pinMask[64], Move* moves, u32* count) {

    PUSH_STACK_TRACE("unpackDoublePushes()");

    Bitboard lsb;
    u8 from, to;
    u32 begin = *count;
    while (pushes) {
        to = popToLSB(&pushes, &lsb);
        from = to - dist;

        // Check for pins
        if (pinned & bbsq(from) && ~pinMask[from] & lsb) {
            continue;
        }

        moves[begin++] = (Move) from | ((Move) to << 6) | ((Move) MOVE_DOUBLE_PUSH << 12);
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
    Bitboard epPawn = side ? board->epTarget << 8 : board->epTarget >> 8;
    u8 kingsq = (u8) ctzll(king);
    i8 pushDist = side ? -8 : 8;

    // Declare useful variables
    Bitboard lsb = 0ull, attacks = 0ull, target = 0ull, bb1 = 0ull, bb2 = 0ull;
    u8 square = 0u, piece = 0u, sq2 = 0u;

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
            bb1 = betweenSQ(square, kingsq);
            bb2 = bb1 & board->all;
            if (popcountll(bb2) == 1ull && (bb2 & defenders)) {
                pinned |= bb2;
                pinMask[ctzll(bb2)] = bb1 | lsb;
            }
        }
    }
    attackers = board->bb[opp];

    // Double check only allows king moves
    if (popcountll(checkers) > 1) {
        attacks = getKingAttacks(kingsq) & ~attacked & notDefenders;
        unpackQuietMoves(kingsq, attacks & notAttackers, moves, count);
        unpackCaptures(kingsq, attacks & attackers, moves, count);

        POP_STACK_TRACE();

        return moves;
    }

    target = checkers ? betweenSQ((u8) ctzll(checkers), kingsq) | checkers : notDefenders;
    while (defenders) {
        attacks = 0ull;

        // Get attacks
        square = popToLSB(&defenders, &lsb);
        piece = ptype(board->mailbox[square]);
        switch (piece) {
            case PAWN: attacks = getPawnAttacks(square, side) & target & attackers; break;
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

        // Special unpack for pawns
        // Needs extra promotion unpack (last rank reached)
        // But pawn attacks must always be captures, so the quiet unpack is no longer needed
        if (piece == PAWN) {
            unpackCaptures(square, attacks & pinMask[square] & ~(RANK_1 | RANK_8), moves, count);
            unpackPromoCaptures(square, attacks & pinMask[square] & (RANK_1 | RANK_8), moves, count);
            continue;
        }

        // Unpack into move array
        unpackQuietMoves(square, attacks & pinMask[square] & notAttackers, moves, count);
        unpackCaptures(square, attacks & pinMask[square] & attackers, moves, count);
    }

    // Compute pushes
    bb1 = getPawnPushes(board->bb[PAWN | side], board->all, side);
    bb2 = getPawnPushes(bb1 & (side ? RANK_6 : RANK_3), board->all, side);
    bb1 &= target;
    unpackPushes(bb1 & ~(RANK_1 | RANK_8), pushDist, pinned, pinMask, moves, count);
    unpackPromoPushes(bb1 & (RANK_1 | RANK_8), pushDist, pinned, pinMask, moves, count);
    unpackDoublePushes(bb2 & target, pushDist * 2, pinned, pinMask, moves, count);

    // Castle magic
    bb1 = side ? BLACK_KC | BLACK_QC : WHITE_KC | WHITE_QC;

    // En passant magic
    if (target & epPawn) {
        // epTarget's opposite side attacks are exactly the places where friendly pawns should be
        piece = (u8) ctzll(board->epTarget);
        attacks = getPawnAttacks(piece, opp) & board->bb[PAWN | side];
        while (attacks) {
            // Get square of friendly pawn
            square = popToLSB(&attacks, &lsb);

            // XOR mask for simulating the ep move. Flips the friendly pawn, the opponent pawn and the epTarget (friendly pawn after being moved)
            bb1 = lsb | epPawn | board->epTarget;
            target = board->all ^ bb1; // Reuse variable, the new board occupancy
            attacked = 0ull; // Reset attacked squares

            // Loop through each opponent slider's attacks (computed earlier)
            bb1 = oppSliders;
            while (bb1) {
                sq2 = popLSB(&bb1);
                switch (ptype(board->mailbox[sq2])) {
                    case BISHOP: attacked |= getBishopAttacks(sq2, target); break;
                    case ROOK: attacked |= getRookAttacks(sq2, target); break;
                    case QUEEN: attacked |= getQueenAttacks(sq2, target); break;
                }
            }

            // If king is attacked, move is not legal
            if (attacked & king) {
                continue;
            }

            // Store move
            moves[*count] = (Move) square | ((Move) piece << 6) | ((Move) MOVE_EP_CAPTURE << 12);
            *count += 1;
        }
    }

    POP_STACK_TRACE();

    return moves;
}

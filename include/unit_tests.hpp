#ifndef UNIT_TESTS_HPP
#define UNIT_TESTS_HPP

#include <iostream>

#include "types.h"
#include "bitboard.h"
#include "attacks.h"
#include "board.h"
#include "movegen.h"

#include "debug_utils.h"

#include "cppvisuals.hpp"

#define MAX_DEPTH 128

namespace unit_test {

inline bool assertBitboards(Bitboard actual, Bitboard expected) {
    if (actual != expected) {
        std::cerr << "assertBitboards() failed!\nExpected:\n" << toString(expected) << "\nGot:\n" << toString(actual) << "\n";
        return false;
    }
    return true;
}

inline bool magicBitboards() {
    std::cout << "Testing bishop bitboards:\n";
    for (u32 sq = 0; sq < 64; sq++) {
        Bitboard mask = getBishopAttacksSlow((u8) sq, 0ull);
        Bitboard subset = 0ull;
        do {
            if (!assertBitboards(getBishopAttacks((u8) sq, mask), getBishopAttacksSlow((u8) sq, mask))) {
                std::cerr << "Test failed at square " << sq << "\n";
                return false;
            }
            subset = (subset - mask) & mask;
        } while (subset != 0ull);
        std::cout << "Square " << sq << " passed\n";
    }
    std::cout << "\nTesting rook bitboards:\n";
    for (u32 sq = 0; sq < 64; sq++) {
        Bitboard mask = getRookAttacksSlow((u8) sq, 0ull);
        Bitboard subset = 0ull;
        do {
            if (!assertBitboards(getRookAttacks((u8) sq, mask), getRookAttacksSlow((u8) sq, mask))) {
                std::cerr << "Test failed at square " << sq << "\n";
                return false;
            }
            subset = (subset - mask) & mask;
        } while (subset != 0ull);
        std::cout << "Square " << sq << " passed\n";
    }
    return true;
}

inline u64 perft(Board* board, u32 depth) {
    if (depth == 0) return 1ull;

    PUSH_STACK_TRACE("perft()");

    u32 count;
    Move moves[MAX_LEGAL_MOVES];
    getLegalMoves(board, moves, &count);

    if (depth == 1) {

        POP_STACK_TRACE();

        return count;
    }

    u64 nodes = 0;
    UndoState state;
    for (u32 i = 0; i < count; i++) {
        makeMove(board, moves[i], &state);
        nodes += perft(board, depth - 1);
        unmakeMove(board, moves[i], &state);
    }

    POP_STACK_TRACE();

    return nodes;
}

// bench() is the real legay perft. No bulk counting not anything other than PURE COMPUTE BABY!
Move benchMoves[MAX_DEPTH][MAX_LEGAL_MOVES];
inline u64 bench(Board* board, u32 depth) {
    if (depth == 0) {
        return 1;
    }

    PUSH_STACK_TRACE("bench()");

    u32 count;
    getLegalMoves(board, benchMoves[depth], &count);
    u64 nodes = 0;
    UndoState state;
    for (u32 i = 0; i < count; i++) {
        makeMove(board, benchMoves[depth][i], &state);
        nodes += bench(board, depth - 1);
        unmakeMove(board, benchMoves[depth][i], &state);
    }

    POP_STACK_TRACE();

    return nodes;
}

inline Bitboard getAttackersTo(Board* board, Bitboard bb) {
    u8 square = ctzll(bb);
    u8 piece = board->mailbox[square];
    if (!piece) return 0ull;
    u8 opp = pside(piece) ^ 1u;
    Bitboard attackers = board->bb[opp], fin = 0ull;
    while (attackers) {
        Bitboard lsb;
        u8 sq = popToLSB(&attackers, &lsb);
        Bitboard attacks = getPieceAttakcs(board->mailbox[sq], sq, opp, board->all);
        if (attacks & bb) fin |= lsb;
    }
    return fin;
}

struct ExtendedPerftResults {
    u64 nodes = 0, captures = 0, ep = 0, castles = 0, promotions = 0, checks = 0, discoveryChecks = 0, doubleChecks = 0, checkmates = 0;
    ExtendedPerftResults& operator+=(const ExtendedPerftResults& other) {
        nodes += other.nodes;
        captures += other.captures;
        ep += other.ep;
        castles += other.castles;
        promotions += other.promotions;
        checks += other.checks;
        discoveryChecks += other.discoveryChecks;
        doubleChecks += other.doubleChecks;
        checkmates += other.checkmates;
        return *this;
    }
};

inline ExtendedPerftResults extendedPerft(Board* board, u32 depth) {

    PUSH_STACK_TRACE("extendedPerft()");

    u32 count;
    Move moves[MAX_LEGAL_MOVES];
    getLegalMoves(board, moves, &count);

    UndoState state;
    ExtendedPerftResults res;

    if (depth <= 1) {
        res.nodes += count;
        for (u32 i = 0; i < count; i++) {
            switch (moveFlag(moves[i])) {
                case MOVE_CAPTURE: res.captures++; break;

                case MOVE_PROMO_N:
                case MOVE_PROMO_B:
                case MOVE_PROMO_R:
                case MOVE_PROMO_Q: res.promotions++; break;

                case MOVE_PROMO_CAPTURE_N:
                case MOVE_PROMO_CAPTURE_B:
                case MOVE_PROMO_CAPTURE_R:
                case MOVE_PROMO_CAPTURE_Q: res.captures++; res.promotions++; break;

                case MOVE_EP_CAPTURE: res.captures++; res.ep++; break;

                case MOVE_CASTLE_K:
                case MOVE_CASTLE_Q: res.castles++; break;

                default: break;
            }

            makeMove(board, moves[i], &state);
            Bitboard checkers = getAttackersTo(board, board->bb[KING | board->side]);
            if (popcountll(checkers) == 2) {
                res.checks++;
                res.doubleChecks++;
            } else if (checkers & bbsq(moveTo(moves[i]))) {
                res.checks++;
            } else if (checkers) {
                res.checks++;
                res.discoveryChecks++;
            }
            u32 dummyCount = 0;
            Move dummy[MAX_LEGAL_MOVES];
            getLegalMoves(board, dummy, &dummyCount);
            if (!dummyCount) {
                res.checkmates++;
            }
            unmakeMove(board, moves[i], &state);
        }

        POP_STACK_TRACE();

        return res;
    }

    for (u32 i = 0; i < count; i++) {
        makeMove(board, moves[i], &state);
        res += extendedPerft(board, depth - 1);
        unmakeMove(board, moves[i], &state);
    }

    POP_STACK_TRACE();

    return res;
}

}

#endif // UNIT_TESTS_HPP

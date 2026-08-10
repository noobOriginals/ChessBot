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

inline void extendedPerft(Board* board, u32 depth, u64& nodes, u64& capt, u64& ep, u64& castle, u64& promo) {
    if (depth == 0) {
        nodes++;
        return;
    }

    PUSH_STACK_TRACE("extendedPerft()");

    u32 count;
    Move moves[MAX_LEGAL_MOVES];
    getLegalMoves(board, moves, &count);

    if (depth == 1) {
        nodes += count;
        for (u32 i = 0; i < count; i++) {
            switch (moveFlag(moves[i])) {
                case MOVE_CAPTURE: capt++; break;

                case MOVE_PROMO_N:
                case MOVE_PROMO_B:
                case MOVE_PROMO_R:
                case MOVE_PROMO_Q: promo++; break;

                case MOVE_PROMO_CAPTURE_N:
                case MOVE_PROMO_CAPTURE_B:
                case MOVE_PROMO_CAPTURE_R:
                case MOVE_PROMO_CAPTURE_Q: capt++; promo++; break;

                case MOVE_EP_CAPTURE: capt++; ep++; break;

                case MOVE_CASTLE_K:
                case MOVE_CASTLE_Q: castle++; break;

                default: break;
            }
        }

        POP_STACK_TRACE();

        return;
    }

    UndoState state;
    for (u32 i = 0; i < count; i++) {
        makeMove(board, moves[i], &state);
        extendedPerft(board, depth - 1, nodes, capt, ep, castle, promo);
        unmakeMove(board, moves[i], &state);
    }

    POP_STACK_TRACE();
}

}

#endif // UNIT_TESTS_HPP

#include <iostream>
#include <chrono>
#include <string>

#include "types.h"
#include "bitboard.h"
#include "board.h"
#include "attacks.h"
#include "movegen.h"

#include "debug_utils.h"

#include "cppvisuals.hpp"

#define TEST_FEN_1 "r1bqkbnr/pppp1ppp/2n5/8/2PpP3/5N2/PP3PPP/RNBQKB1R b KQkq c3 0 4"
#define TEST_FEN_2 "r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 2 3"

bool assertBitboards(Bitboard actual, Bitboard expected) {
    if (actual != expected) {
        std::cerr << "assertBitboards() failed!\nExpected:\n" << toString(expected) << "\nGot:\n" << toString(actual) << "\n";
        return false;
    }
    return true;
}

bool testMagicBitboards() {
    std::cout << "Testing bishop bitboards:\n";
    for (u32 sq = 0; sq < 64; sq++) {
        Bitboard mask = getBishopAttacksSlow(sq, 0ull);
        Bitboard subset = 0ull;
        do {
            if (!assertBitboards(getBishopAttacks(sq, mask), getBishopAttacksSlow(sq, mask))) {
                std::cerr << "Test failed at square " << sq << "\n";
                return false;
            }
            subset = (subset - mask) & mask;
        } while (subset != 0ull);
        std::cout << "Square " << sq << " passed\n";
    }

    std::cout << "\nTesting rook bitboards:\n";
    for (u32 sq = 0; sq < 64; sq++) {
        Bitboard mask = getRookAttacksSlow(sq, 0ull);
        Bitboard subset = 0ull;
        do {
            if (!assertBitboards(getRookAttacks(sq, mask), getRookAttacksSlow(sq, mask))) {
                std::cerr << "Test failed at square " << sq << "\n";
                return false;
            }
            subset = (subset - mask) & mask;
        } while (subset != 0ull);
        std::cout << "Square " << sq << " passed\n";
    }

    return true;
}

u64 perft(Board* board, u32 depth) {
    if (depth == 0) return 1;

    PUSH_STACK_TRACE("perft()");

    u64 legalCount;
    Move legal[MAX_LEGAL_MOVES];
    generateLegalMoves(board, legal, &legalCount);

    if (depth == 1) {

        POP_STACK_TRACE();

        return legalCount;
    }

    UndoState state;
    u64 nodes = 0;
    for (u64 i = 0; i < legalCount; i++) {
        makeMove(board, legal[i], &state);
        nodes += perft(board, depth - 1);
        unmakeMove(board, legal[i], &state);
    }

    POP_STACK_TRACE();

    return nodes;
}

i32 main() {
    initBitboard();
    initAttacks();

    std::string input;
    Move moves[1024];
    UndoState undo[1024];
    u16 moveIdx = 0;
    Board* board = createBoard();
    setFEN(board, STARTPOS_FEN);

    std::cout << "Enter initial position: ";

    // Parse FEN (if present)
    std::cin >> input;
    if (input == "quit" || input == "q") {
        return 0;
    }

    if (input == "fen") {
        std::cin >> input;
        std::string fen = "";
        while (input != "moves" && input != "go") {
            fen += input + " ";
            std::cin >> input;
        }
        setFEN(board, fen.c_str());
    } else if (input == "startpos") {
        std::cin >> input;
    }

    // Parse moves (if given)
    if (input == "moves") {
        std::cin >> input;
        while (input != "go") {
            Move m = stringToMove(board, input.c_str());
            if (!m) return 1;
            moves[moveIdx] = m;
            makeMove(board, m, &undo[moveIdx]);
            moveIdx++;
            std::cin >> input;
        }
    }

    // Loop
    u64 legalCount;
    Move legalMoves[MAX_LEGAL_MOVES];
    while (true) {
        std::cout << board << "\n";
        std::cout << "Board: FEN: " << getSTDStringFEN(board) << "\n";
        generateLegalMoves(board, legalMoves, &legalCount);
        std::cout << "Board: " << legalCount << " pseudo-legal moves: ";
        for (u64 i = 0; i < legalCount; i++) std::cout << getSTDStringMove(legalMoves[i]) << " ";
        std::cout << "\n";
        std::cout << "Enter move: ";
        std::cin >> input;
        if (input == "quit" || input == "q") {
            return 0;
        } else if (input == "perft") {
            u32 depth;
            std::cin >> depth;
            auto start = std::chrono::high_resolution_clock::now();
            u64 nodes = perft(board, depth);
            u64 elapsed = std::chrono::duration<u64, std::nano>(std::chrono::high_resolution_clock::now() - start).count();
            std::cout << "Perft depth " << depth << ", position: FEN: " << getSTDStringFEN(board) << ", nodes: " << nodes << ", NPS: " << nodes * 1000000000 / elapsed << "\n";
            continue;
        } else if (input == "undo") {
            if (moveIdx == 0) continue;
            moveIdx--;
            unmakeMove(board, moves[moveIdx], &undo[moveIdx]);
            continue;
        } else if (input == "redo") {
            makeMove(board, moves[moveIdx], &undo[moveIdx]);
            moveIdx++;
        } else {
            Move m = stringToMove(board, input.c_str());
            if (!m) {
                std::cout << "Invalid move! Try again.\n";
                continue;
            }
            moves[moveIdx] = m;
            makeMove(board, moves[moveIdx], &undo[moveIdx]);
            moveIdx++;
        }
        std::cout << "Made move: " << getSTDStringMove(moves[moveIdx - 1]) << "\n";
        std::cout << "Move flag: " << moveFlag(moves[moveIdx - 1]) << "\n";
    }

    deleteBoard(board);
    return 0;
}

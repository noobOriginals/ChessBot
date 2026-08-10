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
#include "unit_tests.hpp"

#define TEST_FEN_1 "r1bqkbnr/pppp1ppp/2n5/8/2PpP3/5N2/PP3PPP/RNBQKB1R b KQkq c3 0 4"
#define TEST_FEN_2 "r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 2 3"

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
    u32 legalCount;
    Move legalMoves[MAX_LEGAL_MOVES];
    while (true) {
        std::cout << board << "\n";
        std::cout << "Board: FEN: " << getSTDStringFEN(board) << "\n";
        getLegalMoves(board, legalMoves, &legalCount);
        std::cout << "Board: " << legalCount << " pseudo-legal moves: ";
        for (u64 i = 0; i < legalCount; i++) std::cout << getSTDStringMove(legalMoves[i]) << " ";
        std::cout << "\n";
        std::cout << "Enter move: ";
        std::cin >> input;
        if (input == "quit" || input == "q") {
            return 0;
        } else if (input == "bench") {
            u32 depth;
            std::cin >> depth;
            auto start = std::chrono::high_resolution_clock::now();
            u64 nodes = unit_test::bench(board, depth);
            u64 elapsed = std::chrono::duration<u64, std::nano>(std::chrono::high_resolution_clock::now() - start).count() / 1000;
            std::cout <<
                "\nBench results =>" <<
                "\nFEN              : " << getSTDStringFEN(board) <<
                "\nDepth            : " << depth <<
                "\nTime             : " << elapsed / 1000000.0 << "s" <<
                "\nNPS              : " << nodes * 1000000 / elapsed <<
                "\nNodes            : " << nodes << "\n\n";
            continue;
        } else if (input == "perft") {
            u32 depth;
            std::cin >> depth;
            auto start = std::chrono::high_resolution_clock::now();
            u64 nodes = unit_test::perft(board, depth);
            u64 elapsed = std::chrono::duration<u64, std::nano>(std::chrono::high_resolution_clock::now() - start).count() / 1000;
            std::cout <<
                "\nLegacy Perft results =>" <<
                "\nFEN              : " << getSTDStringFEN(board) <<
                "\nDepth            : " << depth <<
                "\nNPS              : " << nodes * 1000000 / elapsed <<
                "\nNodes            : " << nodes << "\n\n";
            continue;
        } else if (input == "experft") {
            u32 depth;
            std::cin >> depth;
            auto start = std::chrono::high_resolution_clock::now();
            unit_test::ExtendedPerftResults perftRes = unit_test::extendedPerft(board, depth);
            u64 elapsed = std::chrono::duration<u64, std::nano>(std::chrono::high_resolution_clock::now() - start).count() / 1000;
            std::cout <<
                "\nExtended Perft results =>" <<
                "\nFEN              : " << getSTDStringFEN(board) <<
                "\nDepth            : " << depth <<
                "\nNPS              : " << perftRes.nodes * 1000000 / elapsed <<
                "\nNodes            : " << perftRes.nodes <<
                "\nCaptures         : " << perftRes.captures <<
                "\nEp               : " << perftRes.ep <<
                "\nCastles          : " << perftRes.castles <<
                "\nPromotions       : " << perftRes.promotions <<
                "\nChecks           : " << perftRes.checks <<
                "\nDiscovery Checks : " << perftRes.discoveryChecks <<
                "\nDouble Checks    : " << perftRes.doubleChecks <<
                "\nCheckmates       : " << perftRes.checkmates << "\n\n";
            continue;
        } else if (input == "undo") {
            if (moveIdx == 0) continue;
            moveIdx--;
            unmakeMove(board, moves[moveIdx], &undo[moveIdx]);
            continue;
        } else if (input == "print") {
            Bitboard def = board->bb[board->side];
            std::cout << toString(def) << "\n";
            while (def) {
                Bitboard lsb;
                u8 sq = popToLSB(&def, &lsb);
                std::cout << "Square " << (u32) sq << "\n" << toString(getPieceAttakcs(board->mailbox[sq], sq, board->side, board->all)) << "\n";
            }
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

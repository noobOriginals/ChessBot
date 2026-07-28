#include <iostream>
#include <chrono>
#include <string>

#include "board.h"
#include "attacks.h"

#define TEST_FEN_1 "r1bqk2r/pppp1ppp/2n2n2/1Bb1p3/4P3/3P1N2/PPP2PPP/RNBQ1RK1 b kq - 0 5"

void displayBoard(Board* board) {
    char buffer[1024] = {};
    getVisualBoardString(board, buffer, 1024);
    std::cout << buffer;
}

void displayBoardFen(Board* board) {
    char buffer[1024] = {};
    getFen(board, buffer, 1024);
    std::cout << buffer << "\n";
}

void displayBitboard(uint64_t bitboard) {
    char buffer[1024] = {};
    getVisualBitboardString(bitboard, buffer, 1024);
    std::cout << buffer;
}

int32_t assertPositions(uint64_t actual, uint64_t expected) {
    if (actual != expected) {
        char expectedBuffer[1024] = {}, actualBuffer[1024] = {};
        getVisualBitboardString(expected, expectedBuffer, 1024);
        getVisualBitboardString(actual, actualBuffer, 1024);
        printf("Position assert failed!\n.Expected: %s\nActual: %s\n", expectedBuffer, actualBuffer);
        return 0;
    }
    return 1;
}

void testMagicBitboards() {
    for (uint32_t sq = 0; sq < 64; sq++) {
        printf("Testing bishop attacks, square %u\n", sq);
        uint64_t mask = bishopMasks[sq];
        uint64_t subset = 0;
        do {
            if (!assertPositions(getBishopAttacks(sq, subset), getBishopAttacksSlow(sq, subset))) return;
            subset = (subset - mask) & mask;
        } while (subset != 0);
    }
    for (uint32_t sq = 0; sq < 64; sq++) {
        printf("Testing rook attacks, square %u\n", sq);
        uint64_t mask = rookMasks[sq];
        uint64_t subset = 0;
        do {
            if (!assertPositions(getRookAttacks(sq, subset), getRookAttacksSlow(sq, subset))) return;
            subset = (subset - mask) & mask;
        } while (subset != 0);
    }
}

void testMagicBitboardsVsSlowVersion() {
    uint32_t iterations = 100000;

    std::cout << "Timing magic bitboards, " << iterations << " iterations\n";
    auto startTime = std::chrono::high_resolution_clock::now();
    for (uint32_t i = 0; i < iterations; i++) {
        for (uint32_t sq = 0; sq < 64; sq++) {
            uint64_t mask = bishopMasks[sq];
            uint64_t subset = 0;
            do {
                getBishopAttacks(sq, subset);
                subset = (subset - mask) & mask;
            } while (subset != 0);
        }
        for (uint32_t sq = 0; sq < 64; sq++) {
            uint64_t mask = rookMasks[sq];
            uint64_t subset = 0;
            do {
                getRookAttacks(sq, subset);
                subset = (subset - mask) & mask;
            } while (subset != 0);
        }
    }
    std::chrono::duration<double, std::milli> elapsed = std::chrono::high_resolution_clock::now() - startTime;
    std::cout << "Took " << elapsed.count() / 1000.0f << " seconds\n";

    std::cout << "Timing magic bitboards, " << iterations << " iterations\n";
    startTime = std::chrono::high_resolution_clock::now();
    for (uint32_t i = 0; i < iterations; i++) {
        for (uint32_t sq = 0; sq < 64; sq++) {
            uint64_t mask = bishopMasks[sq];
            uint64_t subset = 0;
            do {
                getBishopAttacksSlow(sq, subset);
                subset = (subset - mask) & mask;
            } while (subset != 0);
        }
        for (uint32_t sq = 0; sq < 64; sq++) {
            uint64_t mask = rookMasks[sq];
            uint64_t subset = 0;
            do {
                getRookAttacksSlow(sq, subset);
                subset = (subset - mask) & mask;
            } while (subset != 0);
        }
    }
    elapsed = std::chrono::high_resolution_clock::now() - startTime;
    std::cout << "Took " << elapsed.count() / 1000.0f << " seconds\n";
}

int main() {
    initAttackTables();
    Board* board = createBoard();
    Move moves[1024];
    PrevState prevState[1024];
    uint32_t prevIdx = 0;
    std::string input;
    std::cout << "Enter initial board state: ";
    std::cin >> input;
    if (input == "startpos") setFen(board, STARTPOS_FEN);
    else setFen(board, input.c_str());
    std::cin >> input;
    while (input != "go") {
        moves[prevIdx] = getMoveFromAlgebraic(board, input.c_str());
        makeMove(board, moves[prevIdx], &prevState[prevIdx]);
        prevIdx++;
        std::cin >> input;
    }
    while (true) {
        displayBoard(board);
        displayBoardFen(board);
        std::cout << "\nEnter move: ";
        std::cin >> input;
        if (input == "quit") break;
        if (input == "print") {
            std::cin >> input;
            switch (input[0]) {
            case 'P': displayBitboard(board->pieces[0]); break;
            case 'N': displayBitboard(board->pieces[1]); break;
            case 'B': displayBitboard(board->pieces[2]); break;
            case 'R': displayBitboard(board->pieces[3]); break;
            case 'Q': displayBitboard(board->pieces[4]); break;
            case 'K': displayBitboard(board->pieces[5]); break;
            case 'p': displayBitboard(board->pieces[6]); break;
            case 'n': displayBitboard(board->pieces[7]); break;
            case 'b': displayBitboard(board->pieces[8]); break;
            case 'r': displayBitboard(board->pieces[9]); break;
            case 'q': displayBitboard(board->pieces[10]); break;
            case 'k': displayBitboard(board->pieces[11]); break;
            default: break;
            }
            continue;
        }
        if (input == "undo") {
            if (prevIdx < 1) continue;
            prevIdx--;
            unmakeMove(board, moves[prevIdx], &prevState[prevIdx]);
        } else if (input == "redo") {
            if (moves[prevIdx] == 0) continue;
            makeMove(board, moves[prevIdx], &prevState[prevIdx]);
            prevIdx++;
        } else {
            moves[prevIdx] = getMoveFromAlgebraic(board, input.c_str());
            makeMove(board, moves[prevIdx], &prevState[prevIdx]);
            prevIdx++;
        }
    }
    destroyBoard(board);
    return 0;
}

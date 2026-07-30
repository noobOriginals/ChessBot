#include <iostream>
#include <chrono>
#include <string>

#include "board.h"
#include "attacks.h"
#include "movegen.h"

#define TEST_FEN_1 "r1bqk2r/pppp1ppp/2n2n2/1Bb1p3/4P3/3P1N2/PPP2PPP/RNBQ1RK1 b kq - 0 5"

void displayBoard(Board* board) {
    char buffer[1024] = {};
    std::cout << getVisualBoardString(board, buffer, 1024);
}

void displayBoardFen(Board* board) {
    char buffer[1024] = {};
    std::cout << getFen(board, buffer, 1024) << "\n";
}

void displayBitboard(uint64_t bitboard) {
    char buffer[1024] = {};
    std::cout << getVisualBitboardString(bitboard, buffer, 1024);
}

int32_t assertPositions(uint64_t actual, uint64_t expected) {
    if (actual != expected) {
        char buffer[1024] = {};
        printf("Position assert failed!\n.Expected: %s\nActual: %s\n", getVisualBitboardString(expected, buffer, 1024), getVisualBitboardString(actual, buffer, 1024));
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

void minigame() {
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
            for (char c : input) {
                switch (c) {
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
                    case 'W': displayBitboard(getPawnPushes(WHITE, board->pieces[0], board->occupancy)); break;
                    case 'w': displayBitboard(getPawnPushes(BLACK, board->pieces[6], board->occupancy)); break;
                    case 'X': displayBitboard(getPawnDoublePushes(WHITE, board->pieces[0], board->occupancy)); break;
                    case 'x': displayBitboard(getPawnDoublePushes(BLACK, board->pieces[6], board->occupancy)); break;
                    // case 'S': displayBitboard(board->sidePieces[WHITE]); break;
                    // case 's': displayBitboard(board->sidePieces[BLACK]); break;
                    default: break;
                }
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
}

void printMoves(Move* moves, uint32_t size) {
    if (!moves) return;
    char buffer[64] = {};
    printf("All legal moves: ");
    for (uint32_t i = 0; i < size - 1; i++) {
        printf("%s, ", getAlgebraicFromMove(moves[i], buffer, 64));
    }
    printf("%s\n", getAlgebraicFromMove(moves[size - 1], buffer, 64));
}

int main() {
    initAttackTables();
    initRayTable();

    Board* board = createBoard();
    setFen(board, "r1b1kbnr/pp1p1ppp/2p5/q3p1P1/4P3/3n1N2/PPP2P1P/RNBQKB1R w KQkq - 0 7");
    displayBoard(board);

    uint32_t size;
    Move* moves = generateLegalMoves(board, &size);
    printMoves(moves, size);

    destroyBoard(board);
    return 0;
}

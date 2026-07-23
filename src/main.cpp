#include <iostream>
#include <chrono>

#include "board.h"
#include "attacks.h"

#include "magic_search.h"

#define TEST_FEN_1 "r1bqk2r/pppp1ppp/2n2n2/1Bb1p3/4P3/3P1N2/PPP2PPP/RNBQ1RK1 b kq - 0 5"

Board* board;

void testBoardVisual() {
    char buffer[1024] = {};
    getVisualBoardString(board, buffer, 1024);
    std::cout << buffer << "\n";
}

void testBoardFen() {
    char buffer[1024] = {};
    getFen(board, buffer, 1024);
    std::cout << buffer << "\n";
}

void testBitboardVisual(uint64_t bitboard) {
    char buffer[1024] = {};
    getVisualBitboardString(bitboard, buffer, 1024);
    std::cout << buffer << "\n";
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
    board = createBoard();

    setFen(board, TEST_FEN_1);
    computeBitboards(board);
    testBoardFen();
    testBoardVisual();

    testBitboardVisual(bishopMasks[27]);
    testBitboardVisual(bishopMasks[0]);

    uint32_t maxBits = 0;
    for (uint32_t i = 0; i < 64; i++) {
        std::cout << bishopRelevantBits[i] << " ";
        if (bishopRelevantBits[i] > maxBits) maxBits = bishopRelevantBits[i];
    }
    std::cout << "\nMax bits: " << maxBits << "\n";

    testBitboardVisual(rookMasks[27]);
    testBitboardVisual(rookMasks[0]);

    maxBits = 0;
    for (uint32_t i = 0; i < 64; i++) {
        std::cout << rookRelevantBits[i] << " ";
        if (rookRelevantBits[i] > maxBits) maxBits = rookRelevantBits[i];
    }
    std::cout << "\nMax bits: " << maxBits << "\n";

    // testBitboardVisual(pawnAttacks[WHITE][27]);
    // testBitboardVisual(pawnAttacks[BLACK][27]);
    // testBitboardVisual(knightAttacks[27]);
    // testBitboardVisual(kingAttacks[27]);

    // testBitboardVisual(getBishopAttacks(27, board->allPieces));
    // testBitboardVisual(getRookAttacks(27, board->allPieces));
    // testBitboardVisual(getQueenAttacks(27, board->allPieces));

    // setFen(board, STARTPOS_FEN);
    // testBoardVisual();
    // testBoardFen();

    // testBishopMagicSearch(27);
    // testRookMagicSearch(27);

    testMagicBitboardsVsSlowVersion();

    destroyBoard(board);
    return 0;
}

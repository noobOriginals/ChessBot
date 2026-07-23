#include <iostream>

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

    printf("Bishop magic search:\n{");
    for (uint32_t i = 0; i < 64; i++) {
        printf("0x%llxull, ", findBishopMagic(i));
    }
    printf("};\n");

    printf("Rook magic search:\n");
    for (uint32_t i = 0; i < 64; i++) {
        printf("0x%llxull, ", findBishopMagic(i));
    }
    printf("};\n");

    destroyBoard(board);
    return 0;
}

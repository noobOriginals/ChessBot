#include <iostream>

#include "board.h"

void resetBuffer(char* str, uint64_t size) {
    for (uint64_t i = 0; i < size; i++) {
        str[i] = 0;
    }
}

int main() {
    std::cout << "Hello, world!\n";

    Board* board = createBoard();

    char buffer[1024] = {};

    initAttackTables();

    resetBuffer(buffer, 1024);
    getVisualBitboardString(pawnAttacks[WHITE][27], buffer, 1024);
    std::cout << buffer;
    resetBuffer(buffer, 1024);
    getVisualBitboardString(pawnAttacks[BLACK][27], buffer, 1024);
    std::cout << buffer;
    resetBuffer(buffer, 1024);
    getVisualBitboardString(knightAttacks[27], buffer, 1024);
    std::cout << buffer;
    resetBuffer(buffer, 1024);
    getVisualBitboardString(kingAttacks[27], buffer, 1024);
    std::cout << buffer;

    setFen(board, "r1bqk2r/pppp1ppp/2n2n2/1Bb1p3/4P3/3P1N2/PPP2PPP/RNBQ1RK1 b kq - 0 5");
    resetBuffer(buffer, 1024);
    getVisualBoardString(board, buffer, 1024);
    std::cout << buffer;
    resetBuffer(buffer, 1024);
    getFen(board, buffer, 1024);
    std::cout << buffer << "\n";

    setFen(board, STARTPOS_FEN);
    resetBuffer(buffer, 1024);
    getVisualBoardString(board, buffer, 1024);
    std::cout << buffer;
    resetBuffer(buffer, 1024);
    getFen(board, buffer, 1024);
    std::cout << buffer << "\n";



    destroyBoard(board);
    return 0;
}

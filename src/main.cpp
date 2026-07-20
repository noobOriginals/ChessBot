#include <iostream>

#include "board.h"

int main() {
    std::cout << "Hello, world!\n";

    Board* board = createBoard();

    char buffer[1024] = {};

    setFen(board, "r1bqk2r/pppp1ppp/2n2n2/1Bb1p3/4P3/3P1N2/PPP2PPP/RNBQ1RK1 b kq - 0 5");
    getVisualString(board, buffer, 1024);
    std::cout << buffer;

    setFen(board, STARTPOS_FEN);
    getVisualString(board, buffer, 1024);
    std::cout << buffer;

    destroyBoard(board);
    return 0;
}

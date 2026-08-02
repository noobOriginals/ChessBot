#include <iostream>

#include "types.h"
#include "bitboard.h"
#include "board.h"
#include "cppvisuals.hpp"

#define TEST_FEN_1 "r1bqkbnr/pppp1ppp/2n5/8/2PpP3/5N2/PP3PPP/RNBQKB1R b KQkq c3 0 4"
#define TEST_FEN_2 "r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 2 3"

i32 main() {
    Board* board = createBoard();
    setFEN(board, TEST_FEN_1);
    std::cout << board << "\n";
    std::cout << getSTDStringFEN(board) << "\n";
    deleteBoard(board);
    return 0;
}

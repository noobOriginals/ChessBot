#include <iostream>
#include <string>

#include "chess.hpp"

int main() {
    chess::Board board;
    std::cout << board << "\n";
    board.move(chess::Move("e2e3"));
    std::cout << board << "\n";
    return 0;
}

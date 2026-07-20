#include <iostream>
#include <string>

#include "chess.hpp"

int main() {
    std::vector<chess::Move> legal;
    chess::Board board;

    std::cout << board;

    legal = board.genLegalMoves();
    for (auto& m : legal) {
        std::cout << m.toString() << " ";
    }
    std::cout << "\n\n";

    board.move(chess::Move("e2e5"));
    board.move(chess::Move("d7d5"));

    std::cout << board;

    legal = board.genLegalMoves();
    for (auto& m : legal) {
        std::cout << m.toString() << " ";
    }
    std::cout << "\n\n";
    return 0;
}

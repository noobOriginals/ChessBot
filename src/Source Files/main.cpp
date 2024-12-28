#include <iostream>
#include <chess.hpp>

int main() {
    chess::Board board;
    board.reset("rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6 0 2");
    board.print();
    for (int i = 0; i < 4; i++) {
        std::cout << board.castleRights(i) << "\n";
    }
    int r, c;
    board.enPassantSquare(r, c);
    std::cout << r << " " << c << "\n";
    std::cout << board.halfMoves() << "\n";
    std::cout << board.fullMoves() << "\n";
    return 0;
}
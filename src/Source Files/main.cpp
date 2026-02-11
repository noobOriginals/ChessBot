#include <iostream>
#include <chrono>

#include <engine/chess.hpp>

int main() {
    chess::board::reset("rnbqkbnr/pp4p1/2pP3p/3Ppp2/8/8/PPP2PPP/RNBQKBNR w KQkq e6 0 6");
    chess::board::printInfo();
    return 0;
}
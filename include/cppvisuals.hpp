#ifndef CPPVISUALS_HPP
#define CPPVISUALS_HPP

#include <iostream>
#include <string>

#include "types.h"
#include "bitboard.h"
#include "board.h"
#include "visuals.h"

inline std::string getSTDStringMove(Move move) {
    std::string str(32, 0);
    moveToString(move, str.data(), str.size());
    str.shrink_to_fit();
    str.resize(strlen(str.data()));
    return str;
}

inline std::string getSTDStringFEN(const Board* board) {
    std::string str(128, 0);
    getFEN(board, str.data(), str.size());
    str.shrink_to_fit();
    str.resize(strlen(str.data()));
    return str;
}

inline std::string toString(Bitboard bb) {
    std::string str(VISUAL_BUFFER_SIZE, 0);
    getVisualBitboard(bb, str.data(), str.size());
    return str;
}

inline std::string toString(const Board* board) {
    std::string str(VISUAL_BUFFER_SIZE, 0);
    getVisualBoard(board, str.data(), str.size());
    return str;
}

inline std::ostream& operator<<(std::ostream& out, const Board* board) {
    out << toString(board);
    return out;
}

#endif // CPPVISUALS_HPP

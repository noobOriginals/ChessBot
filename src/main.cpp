#include <iostream>

#include "types.h"
#include "bitboard.h"
#include "board.h"

std::ostream& stream(std::ostream& out, Bitboard bb) {
    out << "     a   b   c   d   e   f   g   h     \n";
    out << "   +---+---+---+---+---+---+---+---+   \n";
    for (i32 r = 8; r > 0; r--) {
        out << " " << r << " |";
        for (i32 f = 0; f < 8; f++) {
            if (bb & bbsq(((r - 1) << 3) + f)) {
                out << " 1 |";
            } else {
                out << "   |";
            }
        }
        out << " " << r << " \n";
        out << "   +---+---+---+---+---+---+---+---+   \n";
    }
    out << "     a   b   c   d   e   f   g   h     ";
    return out;
}

i32 main() {
    return 0;
}

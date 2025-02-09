#include <chess.hpp>

using namespace std;

namespace chess {

constexpr char* alphabet = "abcdefgh";

long nanoTime() {
    auto now = std::chrono::time_point_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now());
    return std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
}

namespace piece {
    Piece getPieceColor(Piece value) {
        value = (Piece) (value >> 3);
        return (Piece) (value << 3);
    }
    Piece getPieceType(Piece value) {
        value = (Piece) (value | 24);
        return (Piece) (value ^ 24);
    }
    bool isColor(Piece value, Piece color) {
        return getPieceColor(value) == color;
    }
    bool isType(Piece value, Piece type) {
        return getPieceType(value) == type;
    }
    char toChar(Piece value) {
        bool uppercase = isColor(value, White);
        value = getPieceType(value);
        switch (value) {
            case None: return ' ';
            case Pawn: return uppercase ? 'P' : 'p';
            case Knight: return uppercase ? 'N' : 'n';
            case Bishop: return uppercase ? 'B' : 'b';
            case Rook: return uppercase ? 'R' : 'r';
            case Queen: return uppercase ? 'Q' : 'q';
            case King: return uppercase ? 'K' : 'k';
            default: return ' ';
        }
    }
    Piece toPiece(char value) {
        bool uppercase = isupper(value);
        switch (toupper(value)) {
            case ' ': return None;
            case 'P': return (Piece) ((uppercase ? White : Black) | Pawn);
            case 'N': return (Piece) ((uppercase ? White : Black) | Knight);
            case 'B': return (Piece) ((uppercase ? White : Black) | Bishop);
            case 'R': return (Piece) ((uppercase ? White : Black) | Rook);
            case 'Q': return (Piece) ((uppercase ? White : Black) | Queen);
            case 'K': return (Piece) ((uppercase ? White : Black) | King);
            default: return None;
        }
    }
}

Move::Move() {
    startSquare = 0;
    endSquare = 0;
}
Move::Move(Move& move) {
    this->startSquare = move.startSquare;
    this->endSquare = move.endSquare;
}
void Move::operator=(Move& move) {
    this->startSquare = move.startSquare;
    this->endSquare = move.endSquare;
}
Move::Move(int start, int end) {
    startSquare = start;
    endSquare = end;
}
Move::Move(string move) {
    int rank, file;
    rank = 8 - stoi(string(1, move[1]));
    file = string(alphabet).find_first_of(move[0]);
    startSquare = rank * 8 + file;
    rank = 8 - stoi(string(1, move[3]));
    file = string(alphabet).find_first_of(move[2]);
    endSquare = rank * 8 + file;
}
Move::Move(string start, string end) {
    string move = start + end;
    int rank, file;
    rank = 8 - stoi(string(1, move[1]));
    file = string(alphabet).find_first_of(move[0]);
    startSquare = rank * 8 + file;
    rank = 8 - stoi(string(1, move[3]));
    file = string(alphabet).find_first_of(move[2]);
    endSquare = rank * 8 + file;
}
std::string Move::toString() {
    string move = "";
    string alph = "abcdefgh";
    int rank, file;
    file = startSquare % 8;
    rank = 8 - (startSquare - file) / 8;
    move += "" + alph[file];
    move += "" + rank;
    file = endSquare % 8;
    rank = 8 - (endSquare - file) / 8;
    move += "" + alph[file];
    move += "" + rank;
    return move;
}
Move Move::fromEnPassantTargetSquare(std::string targetSquare) {
    string file = string(1, targetSquare[0]);
    Move move;
    if (targetSquare[1] == '3') {
        move = Move(file + "2" + file + "4");
    } else {
        move = Move(file + "7" + file + "5");
    }
    return move;
}

}
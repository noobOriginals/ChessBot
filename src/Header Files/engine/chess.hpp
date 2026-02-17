#ifndef CHESS_HPP
#define CHESS_HPP

#include <iostream>
#include <string>

#include <util/types.h>

#define WHITE 8
#define BLACK 16

#define NONE 0
#define PAWN 1
#define KNIGHT 2
#define BISHOP 3
#define ROOK 4
#define QUEEN 5
#define KING 6

namespace chess {

typedef uint8 Piece;
struct Move {
    int32 startSquare, endSquare;
    Move();
    Move(int32 startSquare, int32 endSquare);
    Move(std::string move);
    Move(std::string startSquare, std::string endSquare);
    std::string toString();
};
typedef Move* Moves;

namespace piece {
    Piece getPieceColor(Piece value);
    Piece getPieceType(Piece value);
    bool isColor(Piece value, Piece color) ;
    bool isType(Piece value, Piece type);
    char toChar(Piece value);
    Piece toPiece(char value);
}
namespace move {
    Move fromEnPassantTargetSquare(std::string targetSquare);
}
namespace board {
    void reset(std::string fen);
    void reset();
    Piece** to2DArray();
    Moves generateLegalMoves();
    void makeMove(Move move);
    void print();
    void printInfo();
}

}

#endif
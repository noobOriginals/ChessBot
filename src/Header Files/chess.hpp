#ifndef CHESS_H
#define CHESS_H

#include <iostream>
#include <string>
#include <chrono>

#define White 8
#define Black 16

#define None 0
#define Pawn 1
#define Knight 2
#define Bishop 3
#define Rook 4
#define Queen 5
#define King 6

namespace chess {

typedef uint8_t Piece;

long nanoTime();

namespace piece {
    Piece getPieceColor(Piece value);
    Piece getPieceType(Piece value);
    bool isColor(Piece value, Piece color) ;
    bool isType(Piece value, Piece type);
    char toChar(Piece value);
    Piece toPiece(char value);
}
struct Move {
    int startSquare, endSquare;
    Move();
    Move(Move& m);
    void operator=(Move& move);
    Move(int startSquare, int endSquare);
    Move(std::string move);
    Move(std::string startSquare, std::string endSquare);
    std::string toString();
    static Move fromEnPassantTargetSquare(std::string targetSquare);
};
typedef Move* Moves;
namespace board {
    void reset(std::string fen);
    void reset();
    Piece** to2DArray();
    Moves generateLegalMoves();
    void print();
    void printInfo();
}

}

#endif

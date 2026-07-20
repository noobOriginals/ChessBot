#ifndef CHESS_HPP
#define CHESS_HPP

#include <ostream>
#include <string>
#include <vector>

#define NONE   0b00000
#define BAD    0b11111
#define COLOR  0b11000
#define TYPE   0b00111
#define WHITE  0b01000
#define BLACK  0b10000
#define PAWN   0b00001
#define KNIGHT 0b00010
#define BISHOP 0b00011
#define ROOK   0b00100
#define QUEEN  0b00101
#define KING   0b00110

namespace chess {

typedef signed char schar;
typedef long long llong;
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;
typedef unsigned long long ullong;

typedef uchar Piece;

int clampToBoardEdges(int idx);
int strToSquareIdx(const std::string& square);
std::string sqIdxToString(int idx);
Piece getPieceColor(Piece p);
Piece getPieceType(Piece p);
char pieceToChar(Piece p);
Piece charToPiece(char c);

class Move {
public:
    Move() = default;
    Move(int start, int end);
    Move(const std::string& move);

    int getStart() const;
    int getEnd() const;
    bool isValid() const;
    std::string toString() const;

private:
    int start = -1, end = -1;
    bool valid = false;
};

class Board {
public:
    Board();
    Board(const std::string& fen);

    void reset();
    void reset(const std::string& fen);

    void move(const Move& move);

    std::vector<Move> genLegalMoves() const;

    Piece& at(int file, int rank);
    const Piece& at(int file, int rank) const;

    Piece& operator[](int idx);
    const Piece& operator[](int idx) const;

    Piece getNextSide() const;
    uchar getCastleRights() const;

private:
    void checkPawnMoves(int idx, std::vector<Move>& moves) const;

    Piece board[64] = {};
    bool valid = false;
    Piece nextSide = NONE;
    uchar castleRights = 0b1111;
    int enPassantSquare = -1;
};

std::ostream& operator<<(std::ostream& out, const Board& b);

} // namespace chess

#endif // CHESS_HPP

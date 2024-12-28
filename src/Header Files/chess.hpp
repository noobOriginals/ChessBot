#pragma once
#ifndef CHESS_H

#include <iostream>
#include <string>

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
    class Board {
    public:
        Board();
        Board(std::string fen);
        void reset();
        void reset(std::string fen);
        void getSquare(std::string sqr, int& r, int& c);
        unsigned char getPiece(char value);
        char getValue(unsigned char piece);
        void print();
        bool castleRights(int idx) { return castle[idx]; }
        void enPassantSquare(int& r, int& c) { r = enr; c = enc; }
        bool whiteToMove() { return whiteMove; }
        int halfMoves() { return halfm; }
        int fullMoves() { return fullm; }
        unsigned char* operator[](int i) {
            return board[i];
        }
    private:
        unsigned char board[8][8];
        bool castle[4] = { true };
        bool whiteMove;
        int enr, enc;
        int halfm;
        int fullm;
        void printRow();
    };
}

#endif
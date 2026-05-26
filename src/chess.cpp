#include "chess.hpp"

#include <iostream>
#include <string>

namespace chess {

int clampToBoardEdges(int idx) {
    if (idx < 0) {
        return 0;
    }
    if (idx > 7) {
        return 7;
    }
    return idx;
}

int strToSquareIdx(const std::string& square) {
    if (square.size() != 2) {
        return -1;
    }
    int file = std::tolower(square[0]) - 97;
    int rank = square[1] - 49;
    if (file < 0 || file > 7 || rank < 0 || rank > 7) {
        std::cerr << "Invalid square format.\n";
        return -1;
    }
    return rank * 8 + file;
}

std::string sqIdxToString(int idx) {
    if (idx < 0 || idx > 63) {
        std::cerr << "Invalid square idx\n";
        return std::string(1, 0);
    }
    std::string str(2, 0);
    str[0] = (char) idx % 8 + 97;
    str[1] = (char) idx / 8 + 49;
    return str;
}

Piece getPieceColor(Piece p) {
    return COLOR & p;
}

Piece getPieceType(Piece p) {
    return TYPE & p;
}

char pieceToChar(Piece p) {
    char c = (WHITE & p) ? 0 : 32;
    switch (TYPE & p) {
        case PAWN: return c + 'P';
        case KNIGHT: return c + 'N';
        case BISHOP: return c + 'B';
        case ROOK: return c + 'R';
        case QUEEN: return c + 'Q';
        case KING: return c + 'K';
        default: return ' ';
    }
}

Piece charToPiece(char c) {
    Piece p = WHITE;
    if (c > 96 && c < 123) {
        p = BLACK;
    }
    c = std::tolower(c);
    switch (c) {
        case 'P': return p | PAWN;
        case 'N': return p | KNIGHT;
        case 'B': return p | BISHOP;
        case 'R': return p | ROOK;
        case 'Q': return p | QUEEN;
        case 'K': return p | KING;
        default: return NONE;
    }
}

// Move

Move::Move(int start, int end) : start(start), end(end) {}

Move::Move(const std::string& move) {
    if (move.size() != 4) {
        return;
    }
    start = strToSquareIdx(move.substr(0, 2));
    end = strToSquareIdx(move.substr(2, 2));
    valid = true;
}

int Move::getStart() {
    return start;
}

int Move::getEnd() {
    return end;
}

bool Move::isValid() {
    return valid;
}

std::string Move::toString() {
    return sqIdxToString(start) + sqIdxToString(end);
}

// Board

Board::Board() {
    reset();
}

Board::Board(const std::string& fen) {
    reset(fen);
}

void Board::reset() {
    reset("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

void Board::reset(const std::string& fen) {
    int file = 0;
    int rank = 0;
    ulong i = 0;
    while (!valid && i < fen.size()) {
        // Board is not loaded yet
        if (fen[i] == '/') {
            rank++;
            file = 0;
            i++; continue;
        }
        if (fen[i] > 48 && fen[i] < 57) {
            file += fen[i] - 48;
            i++; continue;
        }
        at(file, rank) = charToPiece(fen[i]);
        file++;
        if (rank * 8 + file >= 64) {
            valid = true;
        }
        i++;
    }
}

Piece& Board::at(int file, int rank) {
    file = clampToBoardEdges(file);
    rank = clampToBoardEdges(rank);
    return board[rank * 8 + file];
}

const Piece& Board::at(int file, int rank) const {
    clampToBoardEdges(file);
    clampToBoardEdges(rank);
    return board[rank * 8 + file];
}

Piece& Board::operator[](int idx) {
    if (idx < 0) {
        idx = 0;
    }
    if (idx > 63) {
        idx = 63;
    }
    return board[idx];
}

const Piece& Board::operator[](int idx) const {
    if (idx < 0) {
        idx = 0;
    }
    if (idx > 63) {
        idx = 63;
    }
    return board[idx];
}


} // namespace chess

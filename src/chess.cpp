#include "chess.hpp"

#include <iostream>
#include <string>
#include <vector>

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
        case 'p': return p | PAWN;
        case 'n': return p | KNIGHT;
        case 'b': return p | BISHOP;
        case 'r': return p | ROOK;
        case 'q': return p | QUEEN;
        case 'k': return p | KING;
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

int Move::getStart() const {
    return start;
}

int Move::getEnd() const {
    return end;
}

bool Move::isValid() const {
    return valid;
}

std::string Move::toString() const {
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
    int rank = 7;
    ulong i = 0;
    while (i < fen.size()) {
        if (rank <= 0 && file >= 8) {
            valid = true;
            break;
        }

        // Board is not loaded yet
        if (fen[i] == '/') {
            rank--;
            file = 0;
            i++; continue;
        }
        if (fen[i] > 48 && fen[i] < 57) {
            file += fen[i] - 48;
            i++; continue;
        }
        at(file, rank) = charToPiece(fen[i]);
        file++;
        i++;
    }
    nextMove = (fen[++i] == 'w') ? WHITE : BLACK;
    i++;
    castleRights = 0b0000;
    while (fen[++i] != ' ') {
        switch (fen[i]) {
            case 'K': castleRights |= 0b0001; break;
            case 'Q': castleRights |= 0b0010; break;
            case 'k': castleRights |= 0b0100; break;
            case 'q': castleRights |= 0b1000; break;
        }
    }
}

void Board::move(const Move& move) {

}

std::vector<Move> Board::genLegalMoves() const {
    std::vector<Move> moves;
    for (int idx = 0; idx < 64; idx++) {
        if (board[idx] & nextMove) {
            switch (board[idx] & TYPE) {
                case PAWN:
            }
        }
    }
    return moves;
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

Piece Board::getNextMove() const {
    return nextMove;
}

uchar Board::getCastleRights() const {
    return castleRights;
}

// Private Board

void Board::checkPawnMoves(int idx, std::vector<Move>& moves) {
    int rankOffset = (nextMove & WHITE) ? 8 : -8;
    if (!board[idx + rankOffset]) {
        moves.push_back(Move(idx, idx + rankOffset));
    }
    int startRank = (nextMove & WHITE) ? 1 : 6;
    if (idx / 8 == startRank && !board[idx + 2 * rankOffset]) {
        moves.push_back(Move(idx, idx + 2 * rankOffset));
    }
}

// Print utility

std::ostream& operator<<(std::ostream& out, const Board& b) {
    out << "Chess Board:\n\n";
    out << "     a   b   c   d   e   f   g   h     \n";
    out << "   +---+---+---+---+---+---+---+---+   \n";
    for (int rank = 7; rank >= 0; rank--) {
        out << " " << rank + 1 << " ";
        for (int file = 0; file < 8; file++) {
            out << "| " << pieceToChar(b.at(file, rank)) << " ";
        }
        out << "| " << rank + 1 << " \n";
        out << "   +---+---+---+---+---+---+---+---+   \n";
    }
    out << "     a   b   c   d   e   f   g   h     \n\n";
    out << "Next move: " << ((b.getNextMove() == WHITE) ? "White" : "Black") << "\n";
    uchar castle = b.getCastleRights();
    out << "Castle rights: " << (castle & 0b0001 ? "K" : "") << (castle & 0b0010 ? "Q" : "") << (castle & 0b0100 ? "k" : "") << (castle & 0b1000 ? "q" : "") << "\n\n";
    return out;
}

} // namespace chess

#include <engine/chess.hpp>

#include <util/arr_ops.h>

namespace chess {

const char* ALPHABET = "abcdefgh";

Move::Move() {
    startSquare = 0;
    endSquare = 0;
}
Move::Move(int start, int end) {
    startSquare = start;
    endSquare = end;
}
Move::Move(std::string move) {
    int rank, file;
    rank = 8 - stoi(std::string(1, move[1]));
    file = std::string(ALPHABET).find_first_of(move[0]);
    startSquare = rank * 8 + file;
    rank = 8 - stoi(std::string(1, move[3]));
    file = std::string(ALPHABET).find_first_of(move[2]);
    endSquare = rank * 8 + file;
}
Move::Move(std::string start, std::string end) {
    std::string move = start + end;
    int rank, file;
    rank = 8 - stoi(std::string(1, move[1]));
    file = std::string(ALPHABET).find_first_of(move[0]);
    startSquare = rank * 8 + file;
    rank = 8 - stoi(std::string(1, move[3]));
    file = std::string(ALPHABET).find_first_of(move[2]);
    endSquare = rank * 8 + file;
}
std::string Move::toString() {
    std::string move = "";
    int rank, file;
    file = startSquare % 8;
    rank = 8 - (startSquare - file) / 8;
    move += std::string(1, ALPHABET[file]);
    move += std::to_string(rank);
    file = endSquare % 8;
    rank = 8 - (endSquare - file) / 8;
    move += std::string(1, ALPHABET[file]);
    move += std::to_string(rank);
    return move;
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
        bool uppercase = isColor(value, WHITE);
        value = getPieceType(value);
        switch (value) {
            case NONE: return ' ';
            case PAWN: return uppercase ? 'P' : 'p';
            case KNIGHT: return uppercase ? 'N' : 'n';
            case BISHOP: return uppercase ? 'B' : 'b';
            case ROOK: return uppercase ? 'R' : 'r';
            case QUEEN: return uppercase ? 'Q' : 'q';
            case KING: return uppercase ? 'K' : 'k';
            default: return ' ';
        }
    }
    Piece toPiece(char value) {
        bool uppercase = isupper(value);
        switch (toupper(value)) {
            case ' ': return NONE;
            case 'P': return (Piece) ((uppercase ? WHITE : BLACK) | PAWN);
            case 'N': return (Piece) ((uppercase ? WHITE : BLACK) | KNIGHT);
            case 'B': return (Piece) ((uppercase ? WHITE : BLACK) | BISHOP);
            case 'R': return (Piece) ((uppercase ? WHITE : BLACK) | ROOK);
            case 'Q': return (Piece) ((uppercase ? WHITE : BLACK) | QUEEN);
            case 'K': return (Piece) ((uppercase ? WHITE : BLACK) | KING);
            default: return NONE;
        }
    }
}

namespace move {
    Move fromEnPassantTargetSquare(std::string targetSquare) {
        std::string file = std::string(1, targetSquare[0]);
        Move move;
        if (targetSquare[1] == '3') {
            move = Move(file + "2" + file + "4");
        } else {
            move = Move(file + "7" + file + "5");
        }
        return move;
    }
}

namespace board {
    Piece square[64] = {};
    bool castleRights[4] = {};
    bool whiteToMove = true;
    Move lastMove;
    uint32_t halfMoves = 0, fullMoves = 0;
    uint32_t whitePieces[16], blackPieces[16];

    void reset(std::string fen) {
        for (int i = 0; i < 64; i++) square[i] = 0;
        for (int i = 0; i < 4; i++) castleRights[i] = 0;
        for (int i = 0; i < 16; i++) whitePieces[i] = 0;
        for (int i = 0; i < 16; i++) blackPieces[i] = 0;
        int wpi = 0, bpi = 0;
        char c;
        int s = 0;
        int last = 0;
        for (int i = 0; i < fen.length(); i++) {
            c = fen[i];
            if (c < 58 && c > 47) {
                s += stoi(std::string(1, c));
            } else if (c == '/') {
                s += (s % 8 == 0) ? 0 : 8 - (s % 8);
            } else {
                Piece piece = piece::toPiece(c);
                square[s] = piece;
                if (piece::isColor(piece, WHITE)) {
                    whitePieces[wpi] = s;
                    wpi++;
                } else {
                    blackPieces[bpi] = s;
                    bpi++;
                }
                s++;
            }
            if (s > 63) {
                last = i;
                break;
            }
        }
        last += 2;
        c = fen[last];
        whiteToMove = (c == 'w');
        last += 2;
        std::string castle = fen.substr(last, fen.find_first_of(' ', last));
        if (castle.length() > 0) {
            last = fen.find_first_of(' ', last);
            for (int i = 0; i < castle.length(); i++) {
                c = castle[i];
                switch (c) {
                    case 'K':
                        castleRights[0] = true;
                        break;
                    case 'Q':
                        castleRights[1] = true;
                        break;
                    case 'k':
                        castleRights[2] = true;
                        break;
                    case 'q':
                        castleRights[3] = true;
                        break;
                }
            }
        }
        last++;
        c = fen[last];
        if (c != '-') {
            std::string en = fen.substr(last, fen.find_first_of(' ', last));
            lastMove = move::fromEnPassantTargetSquare(en);
        } else {
            lastMove = Move();
        }
        last = fen.find_first_of(' ', last) + 1;
        std::string hm = fen.substr(last, fen.find_first_of(' ', last));
        halfMoves = stoi(hm);
        last = fen.find_first_of(' ', last) + 1;
        std::string fm = fen.substr(last);
        fullMoves = stoi(fm);
    }

    void reset() {
        reset("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    }

    Piece** to2DArray() {
        Piece** square = (Piece**)malloc(sizeof(Piece*) * 8);
        for (int i = 0; i < 8; i++) square[i] = (Piece*)malloc(sizeof(Piece) * 8);
        for (int i = 0; i < 64; i++) {
            int file, rank;
            file = i % 8;
            rank = (i - file) / 8;
            square[rank][file] = board::square[i];
        }
        return square;
    }

    void makeMove(Move move) {
        if (piece::getPieceColor(square[move.startSquare]) == BLACK) {
            fullMoves++;
            whiteToMove = true;
        } else {
            whiteToMove = false;
        }
        if (square[move.endSquare] != NONE) {
            if (piece::getPieceColor(square[move.endSquare]) == WHITE) {
                whitePieces[find(move.endSquare, whitePieces, 0, 15)] = -1;
            } else {
                blackPieces[find(move.endSquare, blackPieces, 0, 15)] = -1;
            }
        }
        square[move.endSquare] = square[move.startSquare];
        square[move.startSquare] = NONE;
    }

    void print() {
        std::cout << "+---+---+---+---+---+---+---+---+\n";
        std::cout << "|";
        for (int i = 0; i < 64; i++) {
            std::cout << " " + std::string(1, piece::toChar(square[i])) << " |";
            if ((i + 1) % 8 == 0) {
                std::cout << " " << (9 - (i + 1) / 8) << "\n";
                std::cout << "+---+---+---+---+---+---+---+---+\n";
                if (i == 63) break;
                std::cout << "|";
            }
        }
        std::cout << "  a   b   c   d   e   f   g   h  \n";
    }

    void printInfo() {
        std::cout << "Castle Rights:\n";
        std::cout << "  White King-Side: " << castleRights[0] << "\n";
        std::cout << "  White Queen-Side: " << castleRights[1] << "\n";
        std::cout << "  Black King-Side: " << castleRights[2] << "\n";
        std::cout << "  Black Queen-Side: " << castleRights[3] << "\n";
        std::cout << (whiteToMove ? "White to move" : "Black to move") << "\n";
        std::cout << "Last move: " << ((lastMove.startSquare == 0 && lastMove.endSquare == 0) ? "-" : lastMove.toString()) << "\n";
        std::cout << "Half moves: " << halfMoves << "  Full moves: " << fullMoves << "\n";
        std::cout << "White pieces indices: \n";
        for (int i = 0; i < 16; i++) {
            std::string add = whitePieces[i] < 10 ? " " : "";
            std::cout << "  " << whitePieces[i] << add;
        }
        std::cout << "\n";
        std::cout << "Black pieces indices: \n";
        for (int i = 0; i < 16; i++) {
            std::string add = blackPieces[i] < 10 ? " " : "";
            std::cout << "  " << blackPieces[i] << add;
        }
        std::cout << "\n\n";
        print();
    }
}

}
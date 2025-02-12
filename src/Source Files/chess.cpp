#include <chess.hpp>

using namespace std;

namespace chess {

constexpr char* alphabet = "abcdefgh";

long nanoTime() {
    auto now = chrono::time_point_cast<chrono::nanoseconds>(chrono::system_clock::now());
    return chrono::duration_cast<chrono::nanoseconds>(now.time_since_epoch()).count();
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
string Move::toString() {
    string move = "";
    int rank, file;
    file = startSquare % 8;
    rank = 8 - (startSquare - file) / 8;
    move += string(1, alphabet[file]);
    move += to_string(rank);
    file = endSquare % 8;
    rank = 8 - (endSquare - file) / 8;
    move += string(1, alphabet[file]);
    move += to_string(rank);
    return move;
}
Move Move::fromEnPassantTargetSquare(string targetSquare) {
    string file = string(1, targetSquare[0]);
    Move move;
    if (targetSquare[1] == '3') {
        move = Move(file + "2" + file + "4");
    } else {
        move = Move(file + "7" + file + "5");
    }
    return move;
}

namespace board {
    Piece square[64] = {};
    bool castleRights[4] = {};
    bool whiteToMove = true;
    Move lastMove;
    uint32_t halfMoves = 0, fullMoves = 0;
    uint32_t whitePieces[16], blackPieces[16];

    void reset(string fen) {
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
                s += stoi(string(1, c));
            } else if (c == '/') {
                s += (s % 8 == 0) ? 0 : 8 - (s % 8);
            } else {
                Piece piece = piece::toPiece(c);
                square[s] = piece;
                if (piece::isColor(piece, White)) {
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
        string castle = fen.substr(last, fen.find_first_of(' ', last));
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
            string en = fen.substr(last, fen.find_first_of(' ', last));
            lastMove = Move::fromEnPassantTargetSquare(en);
        } else {
            lastMove = Move();
        }
        last = fen.find_first_of(' ', last) + 1;
        string hm = fen.substr(last, fen.find_first_of(' ', last));
        halfMoves = stoi(hm);
        last = fen.find_first_of(' ', last) + 1;
        string fm = fen.substr(last);
        fullMoves = stoi(fm);
    }
    void reset() { reset("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"); }
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
    void print() {
        cout << "+---+---+---+---+---+---+---+---+\n";
        cout << "|";
        for (int i = 0; i < 64; i++) {
            cout << " " + string(1, piece::toChar(square[i])) << " |";
            if ((i + 1) % 8 == 0) {
                cout << " " << (9 - (i + 1) / 8) << "\n";
                cout << "+---+---+---+---+---+---+---+---+\n";
                if (i == 63) break;
                cout << "|";
            }
        }
        cout << "  a   b   c   d   e   f   g   h  \n";
    }
    void printInfo() {
        cout << "Castle Rights:\n";
        cout << "  White King-Side: " << castleRights[0] << "\n";
        cout << "  White Queen-Side: " << castleRights[1] << "\n";
        cout << "  Black King-Side: " << castleRights[2] << "\n";
        cout << "  Black Queen-Side: " << castleRights[3] << "\n";
        cout << (whiteToMove ? "White to move" : "Black to move") << "\n";
        cout << "Last move: " << ((lastMove.startSquare == 0 && lastMove.endSquare == 0) ? "-" : lastMove.toString()) << "\n";
        cout << "Half moves: " << halfMoves << "  Full moves: " << fullMoves << "\n";
        cout << "White pieces indices: \n";
        for (int i = 0; i < 16; i++) {
            string add = whitePieces[i] < 10 ? " " : "";
            cout << "  " << whitePieces[i] << add;
        }
        cout << "\n";
        cout << "Black pieces indices: \n";
        for (int i = 0; i < 16; i++) {
            string add = blackPieces[i] < 10 ? " " : "";
            cout << "  " << blackPieces[i] << add;
        }
        cout << "\n\n";
        print();
    }
}

}
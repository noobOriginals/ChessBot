#include <chess.hpp>
using namespace std;

bool pieceIsColor(unsigned char piece, unsigned char color) {
    return (piece >> 3 == color >> 3);
}

namespace chess {
    Board::Board() {
        reset();
    }
    Board::Board(string fen) {
        reset(fen);
    }
    void Board::reset() {
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                board[i][j] = 0;
            }
        }
        whiteMove = true;
        board[0][0] = BLACK | ROOK;
        board[0][1] = BLACK | KNIGHT;
        board[0][2] = BLACK | BISHOP;
        board[0][3] = BLACK | QUEEN;
        board[0][4] = BLACK | KING;
        board[0][5] = BLACK | BISHOP;
        board[0][6] = BLACK | KNIGHT;
        board[0][7] = BLACK | ROOK;
        for (int i = 0; i < 8; i++) {
            board[1][i] = BLACK | PAWN;
            board[6][i] = WHITE | PAWN;
        }
        board[7][0] = WHITE | ROOK;
        board[7][1] = WHITE | KNIGHT;
        board[7][2] = WHITE | BISHOP;
        board[7][3] = WHITE | QUEEN;
        board[7][4] = WHITE | KING;
        board[7][5] = WHITE | BISHOP;
        board[7][6] = WHITE | KNIGHT;
        board[7][7] = WHITE | ROOK;
    }
    void Board::reset(string fen) {
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                board[i][j] = 0;
            }
        }
        for (int i = 0; i < 4; i++) {
            castle[i] = false;
        }
        char c;
        int s = 0;
        int last = 0;
        for (int i = 0; i < fen.length(); i++) {
            c = fen[i];
            if (c < 58 && c > 47) {
                string st(1, c);
                s += stoi(st);
            } else if (c == '/') {
                s += (s % 8 == 0) ? 0 : 8 - (s % 8);
            } else {
                board[(int)(s / 8)][s % 8] = getPiece(c);
                s++;
            }
            if (s > 63) {
                last = i;
                break;
            }
        }
        last += 2;
        c = fen[last];
        whiteMove = (c == 'w');
        last += 2;
        c = fen[last];
        string cas = fen.substr(last, fen.find_first_of(' ', last));
        if (cas.length() > 0) {
            last = fen.find_first_of(' ', last);
            for (int i = 0; i < cas.length(); i++) {
                c = cas[i];
                switch (c) {
                    case 'K':
                        castle[0] = true;
                        break;
                    case 'Q':
                        castle[1] = true;
                        break;
                    case 'k':
                        castle[2] = true;
                        break;
                    case 'q':
                        castle[3] = true;
                        break;
                }
            }
        }
        last++;
        c = fen[last];
        if (c != '-') {
            string en = fen.substr(last, fen.find_first_of(' ', last));
            last = fen.find_first_of(' ', last);
            getSquare(en, enr, enc);
        } else {
            last = fen.find_first_of(' ', last);
            enr = 0;
            enc = 0;
        }
        last++;
        string hm = fen.substr(last, fen.find_first_of(' ', last));
        halfm = stoi(hm);
        last = fen.find_first_of(' ', last) + 1;
        string fm = fen.substr(last);
        fullm = stoi(fm);
    }
    void Board::getSquare(string sqr, int& r, int& c) {
        string alph = "abcdefgh";
        c = alph.find_first_of(sqr[0]);
        r = 8 - stoi(string(1, sqr[1]));
    }
    unsigned char Board::getPiece(char value) {
        bool uppercase = (value < 91);
        value = toupper(value);
        switch (value) {
            case ' ':
                return (unsigned char) (((uppercase) ? WHITE : BLACK) | NONE);
            case 'P':
                return (unsigned char) (((uppercase) ? WHITE : BLACK) | PAWN);
            case 'N':
                return (unsigned char) (((uppercase) ? WHITE : BLACK) | KNIGHT);
            case 'B':
                return (unsigned char) (((uppercase) ? WHITE : BLACK) | BISHOP);
            case 'R':
                return (unsigned char) (((uppercase) ? WHITE : BLACK) | ROOK);
            case 'Q':
                return (unsigned char) (((uppercase) ? WHITE : BLACK) | QUEEN);
            case 'K':
                return (unsigned char) (((uppercase) ? WHITE : BLACK) | KING);
            default:
                return (unsigned char) (((uppercase) ? WHITE : BLACK) | NONE);
        }
    }
    char Board::getValue(unsigned char piece) {
        bool uppercase = pieceIsColor(piece, WHITE);
        short add = 3 << 3;
        piece = (short) (piece | add);
        piece = (short) (piece ^ add);
        switch (piece) {
            case NONE:
                return (uppercase) ? ' ' : ' ';
            case PAWN:
                return (uppercase) ? 'P' : 'p';
            case KNIGHT:
                return (uppercase) ? 'N' : 'n';
            case BISHOP:
                return (uppercase) ? 'B' : 'b';
            case ROOK:
                return (uppercase) ? 'R' : 'r';
            case QUEEN:
                return (uppercase) ? 'Q' : 'q';
            case KING:
                return (uppercase) ? 'K' : 'k';
            default:
                return ' ';
        }
    }
    void Board::printRow() {
        cout << "+---+---+---+---+---+---+---+---+\n";
    }
    void Board::print() {
        printRow();
        cout << "|";
        for (int i = 0; i < 64; i++) {
            cout << " " << getValue(board[(int)(i / 8)][i % 8]) << " |";
            if ((i + 1) % 8 == 0) {
                cout << " " << (9 - (int) ((i + 1) / 8)) << "\n";
                printRow();
                if (i == 63) {
                    break;
                }
                cout << "|";
            }
        }
        cout << "  a   b   c   d   e   f   g   h  \n";
    }
}
#include "board.h"

// Std includes
#include <stdlib.h>
#include <string.h>

// Create the Board struct (allocate memory)
Board* createBoard() {
    return (Board*) calloc(1, sizeof(Board));
}

// Create the Board struct's object (allocate memory) and set the board in the starting position via setFen(...)
Board* createDefaultBoard() {
    Board* board = createBoard();
    setFen(board, STARTPOS_FEN);
    return board;
}

// Free board-occupied memory (just free())
void destroyBoard(Board* board) {
    free(board);
}

// Compute merged bitboards
void computeBitboards(Board* board) {
    board->whitePieces = 0;
    board->blackPieces = 0;
    for (uint bb = 0; bb < TOTAL_PIECE_TYPES; bb += 1) {
        if (bb < BLACK_OFFSET) {
            board->whitePieces |= board->pieces[bb];
        } else {
            board->blackPieces |= board->pieces[bb];
        }
    }
    board->allPieces = board->whitePieces | board->blackPieces;
}

// Sets the board to the specified FEN
void setFen(Board* board, const char* fen) {
    memset(board, 0, sizeof(Board));
    uint len = strlen(fen);
    uint i = 0;
    uint rank = 7, file = 0;
    while (i < len && fen[i] != ' ') {
        if (fen[i] == '/') {
            rank -= 1;
            file = 0;
            i += 1;
            continue;
        }
        if (fen[i] < 58) {
            file += fen[i] - 48;
            i += 1;
            continue;
        }
        uint square = rank * 8 + file;
        uint offset = 0;
        char piece = fen[i];
        if (fen[i] > 90) {
            offset = BLACK_OFFSET;
            piece -= 32; // To upper case
        }
        switch (piece) {
            case 'P': board->pieces[PAWN + offset] |= (1ull << square); break;
            case 'N': board->pieces[KNIGHT + offset] |= (1ull << square); break;
            case 'B': board->pieces[BISHOP + offset] |= (1ull << square); break;
            case 'R': board->pieces[ROOK + offset] |= (1ull << square); break;
            case 'Q': board->pieces[QUEEN + offset] |= (1ull << square); break;
            case 'K': board->pieces[KING + offset] |= (1ull << square); break;
            default: break;
        }
        file += 1;
        i += 1;
    }
    i += 1; if (i >= len) return;
    board->sideToMove = (fen[i] == 'w') ? WHITE : BLACK;
    i += 2; if (i >= len) return;
    if (fen[i] != '-') {
        while (i < len && fen[i] != ' ') {
            switch (fen[i]) {
                case 'K': board->castle |= 0b1000; break;
                case 'Q': board->castle |= 0b0100; break;
                case 'k': board->castle |= 0b0010; break;
                case 'q': board->castle |= 0b0001; break;
                default: break;
            }
            i += 1;
        }
        i += 1;
    } else {
        i += 2;
    }
    if (i >= len) return;
    if (fen[i] != '-') {
        file = fen[i] - 'a';
        i += 1; if (i >= len) return;
        rank = fen[i] - '1';
        board->epTarget = rank * 8 + file;
    } else {
        board->epTarget = NO_EP_TARGET;
    }
    i += 2; if (i >= len) return;
    char buffer[64];
    uint bufferIdx = 0;
    while (i < len && bufferIdx < 63 && fen[i] != ' ') {
        buffer[bufferIdx] = fen[i];
        bufferIdx += 1;
        i += 1;
    }
    buffer[63] = '\0';
    board->halfMoves = atoi(buffer);
    i += 1; if (i >= len) return;
    for (uint j = 0; j < 64; j += 1) buffer[j] = 0;
    bufferIdx = 0;
    while (i < len && bufferIdx < 63) {
        buffer[bufferIdx] = fen[i];
        bufferIdx += 1;
        i += 1;
    }
    buffer[63] = '\0';
    board->fullMoves = atoi(buffer);
}

// Sets the board to the specified FEN and then executes the given move array
void setFenAndMoves(Board* board, const char* fen, const char** moves, uint moveCount) {
    setFen(board, fen);
}

// Returns the FEN string representation of the board
const char* getFen(Board* board) {
    return "";
}

// Returns the visual string representation of the board, ready to be printed to the console, or file or anything else
void getVisualString(Board* board, char* str, ullong len) {
    const char* template =
    "     a   b   c   d   e   f   g   h     \n"
    "   +---+---+---+---+---+---+---+---+   \n"
    " 8 | x | y |   |   |   |   |   | z | 8 \n"
    "   +---+---+---+---+---+---+---+---+   \n"
    " 7 | w |   |   |   |   |   |   |   | 7 \n"
    "   +---+---+---+---+---+---+---+---+   \n"
    " 6 |   |   |   |   |   |   |   |   | 6 \n"
    "   +---+---+---+---+---+---+---+---+   \n"
    " 5 |   |   |   |   |   |   |   |   | 5 \n"
    "   +---+---+---+---+---+---+---+---+   \n"
    " 4 |   |   |   |   |   |   |   |   | 4 \n"
    "   +---+---+---+---+---+---+---+---+   \n"
    " 3 |   |   |   |   |   |   |   |   | 3 \n"
    "   +---+---+---+---+---+---+---+---+   \n"
    " 2 |   |   |   |   |   |   |   |   | 2 \n"
    "   +---+---+---+---+---+---+---+---+   \n"
    " 1 |   |   |   |   |   |   |   |   | 1 \n"
    "   +---+---+---+---+---+---+---+---+   \n"
    "     a   b   c   d   e   f   g   h     \n";
    if (len < strlen(template)) return;
    memcpy(str, template, strlen(template));
    computeBitboards(board);
    uint vidx = 85;
    for (uint rank = 8; rank > 0; rank--) {
        for (uint file = 0; file < 8; file += 1) {
            uint square = (rank - 1) * 8 + file;
            char piece = ' ';
            if (board->allPieces & (1ull << square)) {
                for (uint bb = 0; bb < TOTAL_PIECE_TYPES; bb += 1) {
                    if (board->pieces[bb] & (1ull << square)) {
                        if (bb >= BLACK_OFFSET) {
                            bb -= BLACK_OFFSET;
                            piece = 32; // To lower case
                        } else {
                            piece = 0;
                        }
                        switch (bb) {
                            case PAWN: piece += 'P'; break;
                            case KNIGHT: piece += 'N'; break;
                            case BISHOP: piece += 'B'; break;
                            case ROOK: piece += 'R'; break;
                            case QUEEN: piece += 'Q'; break;
                            case KING: piece += 'K'; break;
                            default: piece = ' '; break;
                        }
                        break;
                    }
                }
            }
            str[vidx] = piece;
            vidx += 4; // File offset in template visual
        }
        vidx += 48; // Rank offset from last file to first file in template visual
    }
}

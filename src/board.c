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
    for (uint32_t bb = 0; bb < TOTAL_PIECE_TYPES; bb += 1) {
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
    // Zero the entire board, ready for new FEN configuration
    memset(board, 0, sizeof(Board));
    board->epTarget = NO_EP_TARGET;

    // Initiate function-scope values
    uint32_t fenSize = strlen(fen);
    uint32_t rank = 7, file = 0;
    uint32_t i = 0;

    while (i < fenSize && fen[i] != ' ') {
        // Next rank
        if (fen[i] == '/') {
            rank -= 1;
            file = 0;
            i += 1;
            continue;
        }

        // Skip over empty squares
        if (fen[i] < 'A') {
            file += fen[i] - '0';
            i += 1;
            continue;
        }

        // Calculate necessary values
        uint32_t square = rank * 8 + file;
        uint32_t offset = 0;

        char piece = fen[i];
        if (fen[i] > 'Z') { // Char is lower case
            offset = BLACK_OFFSET; // Change to black bitboards
            piece = fen[i] - 'a' + 'A'; // To upper case
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

        // Next file
        file += 1;
        i += 1;
    }
    i += 1; if (i >= fenSize) return; // Increment and check for out of bounds pattern (avoids reading uninitialized memory)

    board->sideToMove = (fen[i] == 'w') ? WHITE : BLACK;
    i += 2; if (i >= fenSize) return;

    if (fen[i] != '-') {
        while (i < fenSize && fen[i] != ' ') {
            switch (fen[i]) {
                case 'K': board->castle |= WHITE_KC; break;
                case 'Q': board->castle |= WHITE_QC; break;
                case 'k': board->castle |= BLACK_KC; break;
                case 'q': board->castle |= BLACK_QC; break;
                default: break;
            }
            i += 1;
        }
        i += 1;
    } else {
        i += 2; // Skip over two chars (the '-' and the following whitespace)
    }
    if (i >= fenSize) return;

    if (fen[i] != '-') {
        file = fen[i] - 'a';
        i += 1; if (i >= fenSize) return;
        rank = fen[i] - '1';
        board->epTarget = rank * 8 + file;
    }
    i += 2; if (i >= fenSize) return;

    // Construct the half moves number into a char buffer then pass it to atoi() to convert to an integer
    char buffer[64];
    uint32_t bufferIdx = 0;
    while (i < fenSize && bufferIdx < 63 && fen[i] != ' ') {
        buffer[bufferIdx] = fen[i];
        bufferIdx += 1;
        i += 1;
    }
    buffer[63] = '\0'; // Make sure last char is null
    board->halfMoves = atoi(buffer);
    i += 1; if (i >= fenSize) return;

    // Same pattern as before, for full moves
    for (uint32_t j = 0; j < 64; j += 1) buffer[j] = 0;
    bufferIdx = 0;
    while (i < fenSize && bufferIdx < 63) {
        buffer[bufferIdx] = fen[i];
        bufferIdx += 1;
        i += 1;
    }
    buffer[63] = '\0';
    board->fullMoves = atoi(buffer);
}

// Sets the board to the specified FEN and then executes the given move array
void setFenAndMoves(Board* board, const char* fen, const char** moves, uint32_t moveCount) {
    setFen(board, fen);
    // TODO: implement move making and the rest of the function
}

// Returns the FEN string representation of the board
const char* getFen(Board* board) {
    // TODO: reverse FEN parsing into FEN building
    return "";
}

// Returns the visual string representation of the board, ready to be printed to the console, or file or anything else
int getVisualString(Board* board, char* str, uint64_t size) {
    // Visual template that gets copied to the given buffer
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
    if (size < strlen(template)) return -1;
    memcpy(str, template, strlen(template));

    computeBitboards(board); // Compute all necessary bitboards
    uint32_t vidx = 85; // First index of a square in the visual template
    for (uint32_t rank = 8; rank > 0; rank--) {
        for (uint32_t file = 0; file < 8; file += 1) {
            uint32_t square = (rank - 1) * 8 + file;
            char piece = ' ';
            if (board->allPieces & (1ull << square)) { // Only if a piece is present
                for (uint32_t bb = 0; bb < TOTAL_PIECE_TYPES; bb += 1) { // Check until finding the right piece type
                    if (board->pieces[bb] & (1ull << square)) {
                        if (bb >= BLACK_OFFSET) {
                            bb -= BLACK_OFFSET; // Switch to constant indicees
                            piece = 'a' - 'A'; // To lower case
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
    return 0;
}

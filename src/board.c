#include "board.h"

// Std includes
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Utility
static const char* boardVisualTemplate =
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
"     a   b   c   d   e   f   g   h     \n"; // Visual template that gets copied to the given buffer

// Castle rights masks
const uint8_t castleRightsMask[64] = {
    0b1011, 0b1111, 0b1111, 0b1111, 0b0011, 0b1111, 0b1111, 0b0111,
    0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111,
    0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111,
    0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111,
    0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111,
    0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111,
    0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111,
    0b1110, 0b1111, 0b1111, 0b1111, 0b1100, 0b1111, 0b1111, 0b1101
};

// En passant capture offset
const int32_t epCaptureOffset[2] = {-8, 8};

// Utility
void placePiece(Board* board, uint32_t pType, uint32_t square) {
    uint64_t mask = (1ull << square);
    board->pieces[pType] |= mask;
    board->mailbox[square] = pType;
    board->occupancy |= mask;
#if defined(USE_PER_PIECE_BITBOARDS)
    board->sidePieces[pType >= BLACK_OFFSET] |= mask;
#endif
}

void removePiece(Board* board, uint32_t pType, uint32_t square) {
    uint64_t mask = (1ull << square);
    board->pieces[pType] &= ~mask;
    board->mailbox[square] = NO_PIECE;
    board->occupancy &= ~mask;
#if defined(USE_PER_PIECE_BITBOARDS)
    board->sidePieces[pType >= BLACK_OFFSET] &= ~mask;
#endif
}

void movePiece(Board* board, uint32_t pType, uint32_t from, uint32_t to) {
    uint64_t mask = (1ull << from) | (1ull << to);
    board->pieces[pType] ^= mask;
    board->mailbox[from] = NO_PIECE;
    board->mailbox[to] = pType;
    board->occupancy ^= mask;
#if defined(USE_PER_PIECE_BITBOARDS)
    board->sidePieces[pType >= BLACK_OFFSET] ^= mask;
#endif
}

// Create the Board struct (allocate memory and clear board fields)
Board* createBoard() {
    Board* board = (Board*) calloc(1, sizeof(Board));
    for (uint32_t i = 0; i < 64; i += 1) board->mailbox[i] = NO_PIECE;
    board->epTarget = NO_EP_TARGET;
    return board;
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

// Make moves on the boards
void makeMove(Board* board, Move move, PrevState* state) {
    uint32_t from = moveFrom(move), to = moveTo(move), flag = moveFlag(move), capture = to; // Unpack move data
    if (flag == MOVE_EP_CAPTURE) capture += epCaptureOffset[board->sideToMove]; // Necessary for en passant
    uint32_t pType = board->mailbox[from], capType = board->mailbox[capture]; // Get piece types

    // Store prev state
    state->captured = capType;
    state->castle = board->castle;
    state->epTarget = board->epTarget;
    state->halfMoves = board->halfMoves;

    // Default board update
    board->halfMoves = (pType == PAWN || pType == BLACK_OFFSET || capType != NO_PIECE) ? 0 : board->halfMoves + 1;
    board->epTarget = NO_EP_TARGET;

    switch (flag) {
    case MOVE_QUIET:
        movePiece(board, pType, from, to); // Just move the piece
        break;

    case MOVE_DOUBLE_PUSH:
        movePiece(board, pType, from, to);
        board->epTarget = to + epCaptureOffset[board->sideToMove]; // Also set ep target
        break;

    case MOVE_CASTLE_K:
        movePiece(board, pType, from, to);
        movePiece(board, pType + ROOK - KING, from + 3, to - 1); // Move the rook of the same color (pType always is KING, but color differs)
        break;

    case MOVE_CASTLE_Q:
        movePiece(board, pType, from, to);
        movePiece(board, pType + ROOK - KING, from - 4, to + 1); // Move the rook of the same color (pType always is KING, but color differs)
        break;

    case MOVE_CAPTURE:
        removePiece(board, capType, capture); // First remove the captured piece for movePiece() to work properly
        movePiece(board, pType, from, to);
        break;

    case MOVE_EP_CAPTURE:
        removePiece(board, capType, capture); // First remove the captured piece for movePiece() to work properly. EP capture handled earlier
        movePiece(board, pType, from, to);
        break;

    case MOVE_PROMO_N:
        removePiece(board, pType, from); // Remove the pawn
        placePiece(board, pType + KNIGHT, to); // pType always is KNIGHT, color differs
        break;

    case MOVE_PROMO_B:
        removePiece(board, pType, from); // Remove the pawn
        placePiece(board, pType + BISHOP, to); // pType always is BISHOP, color differs
        break;

    case MOVE_PROMO_R:
        removePiece(board, pType, from); // Remove the pawn
        placePiece(board, pType + ROOK, to); // pType always is ROOK, color differs
        break;

    case MOVE_PROMO_Q:
        removePiece(board, pType, from); // Remove the pawn
        placePiece(board, pType + QUEEN, to); // pType always is QUEEN, color differs
        break;

    case MOVE_PROMO_CAPTURE_N:
        removePiece(board, capType, capture);
        removePiece(board, pType, from);
        placePiece(board, pType + KNIGHT, to);
        break;

    case MOVE_PROMO_CAPTURE_B:
        removePiece(board, capType, capture);
        removePiece(board, pType, from);
        placePiece(board, pType + BISHOP, to);
        break;

    case MOVE_PROMO_CAPTURE_R:
        removePiece(board, capType, capture);
        removePiece(board, pType, from);
        placePiece(board, pType + ROOK, to);
        break;

    case MOVE_PROMO_CAPTURE_Q:
        removePiece(board, capType, capture);
        removePiece(board, pType, from);
        placePiece(board, pType + QUEEN, to);
        break;

    default:
        fprintf(stderr, "makeMove() failed: Invalid Flag!\n");
        exit(1);
    }

    board->castle &= castleRightsMask[from] & castleRightsMask[to]; // Update castle rights
    board->fullMoves += board->sideToMove; // Increment full move count (black increments, white does nothing)
    board->sideToMove ^= 0b1; // Flip side to move
}

void unmakeMove(Board* board, Move move, PrevState* state) {
    board->sideToMove ^= 0b1; // Flip first for correct ep capture offset
    uint32_t from = moveFrom(move), to = moveTo(move), flag = moveFlag(move), capture = to; // Unpack move data
    if (flag == MOVE_EP_CAPTURE) capture += epCaptureOffset[board->sideToMove]; // When undoing en passant move
    uint32_t pType = board->mailbox[to], capType = state->captured; // Get piece types

    switch (flag) {
    case MOVE_QUIET:
        movePiece(board, pType, to, from); // Move in reverse (to -> from)
        break;

    case MOVE_DOUBLE_PUSH:
        movePiece(board, pType, to, from); // Move in reverse (to -> from)
        break;

    case MOVE_CASTLE_K:
        movePiece(board, pType + ROOK - KING, to - 1, from + 3); // Move rook of same color (like makeMove() does above)
        movePiece(board, pType, to, from);
        break;

    case MOVE_CASTLE_Q:
        movePiece(board, pType + ROOK - KING, to + 1, from - 4); // Move rook of same color (like makeMove() does above)
        movePiece(board, pType, to, from);
        break;

    case MOVE_CAPTURE:
        movePiece(board, pType, to, from); // First move the piece
        placePiece(board, capType, capture); // Then place the captured one
        break;

    case MOVE_EP_CAPTURE:
        movePiece(board, pType, to, from); // First move the piece
        placePiece(board, capType, capture); // Then place the captured one (en passant handled above)
        break;

    case MOVE_PROMO_N:
        removePiece(board, pType, to); // Remove the current piece
        placePiece(board, pType - KNIGHT, from); // Place the pawn back (pType is always KNIGHT, color differs)
        break;

    case MOVE_PROMO_B:
        removePiece(board, pType, to); // Remove the current piece
        placePiece(board, pType - BISHOP, from); // Place the pawn back (pType is always BISHOP, color differs)
        break;

    case MOVE_PROMO_R:
        removePiece(board, pType, to); // Remove the current piece
        placePiece(board, pType - ROOK, from); // Place the pawn back (pType is always ROOK, color differs)
        break;

    case MOVE_PROMO_Q:
        removePiece(board, pType, to); // Remove the current piece
        placePiece(board, pType - QUEEN, from); // Place the pawn back (pType is always QUEEN, color differs)
        break;

    case MOVE_PROMO_CAPTURE_N:
        removePiece(board, pType, to);
        placePiece(board, pType - KNIGHT, from);
        placePiece(board, capType, capture);
        break;

    case MOVE_PROMO_CAPTURE_B:
        removePiece(board, pType, to);
        placePiece(board, pType - BISHOP, from);
        placePiece(board, capType, capture);
        break;

    case MOVE_PROMO_CAPTURE_R:
        removePiece(board, pType, to);
        placePiece(board, pType - ROOK, from);
        placePiece(board, capType, capture);
        break;

    case MOVE_PROMO_CAPTURE_Q:
        removePiece(board, pType, to);
        placePiece(board, pType - QUEEN, from);
        placePiece(board, capType, capture);
        break;

    default:
        fprintf(stderr, "unmakeMove() failed: Invalid Flag!\n");
        exit(1);
    }

    // Restore prev state
    board->castle = state->castle;
    board->epTarget = state->epTarget;
    board->halfMoves = state->halfMoves;
    board->fullMoves -= board->sideToMove; // Decrement full move count (black decrements, white does nothing)
}

// Convert algebraic notation to Move type
Move getMoveFromAlgebraic(Board* board, const char* agbMove) {
    if (strlen(agbMove) < 4) {
        fprintf(stderr, "getMoveFromAlgebraic() failed: Invalid move length: %s\n", agbMove);
        exit(1);
    }

    // Check move
    int32_t valid = 1;
    if (agbMove[0] < 'a' || agbMove[0] > 'h') valid = 0;
    if (agbMove[1] < '1' || agbMove[1] > '8') valid = 0;
    if (agbMove[2] < 'a' || agbMove[2] > 'h') valid = 0;
    if (agbMove[3] < '1' || agbMove[3] > '8') valid = 0;
    if (!valid) {
        fprintf(stderr, "getMoveFromAlgebraic() failed: Invalid move format: %s\n", agbMove);
        exit(1);
    }

    // Extract move data
    uint32_t from = ((agbMove[1] - '1') << 3) + agbMove[0] - 'a';
    uint32_t to = ((agbMove[3] - '1') << 3) + agbMove[2] - 'a';
    uint32_t flag = MOVE_QUIET;

    uint32_t distance = (to < from) ? from - to : to - from; // Distance between squares (always positive)
    uint32_t sideOffset = BLACK_OFFSET * board->sideToMove; // Offset for handling different sides
    uint32_t pType = board->mailbox[from]; // Piece type
    uint32_t capType = board->mailbox[to]; // Capture type

    if (capType != NO_PIECE) flag = MOVE_CAPTURE; // Standard piece captures

    // In case of pawn
    if (pType - sideOffset == PAWN) {
        if (distance == 16) flag = MOVE_DOUBLE_PUSH; // Distance = 2 ranks -> double push
        if (to == board->epTarget) flag = MOVE_EP_CAPTURE; // TO square is ep target -> en passant
        if ((to >> 3) == 7 || (to >> 3) == 0) { // Pawn reached last rank
            if (strlen(agbMove) < 5) {
                fprintf(stderr, "getMoveFromAlgebraic() failed: Invalid promotion move length: %s\n", agbMove);
                exit(1);
            }
            switch (agbMove[4]) {
                case 'n': flag = MOVE_PROMO_N; break;
                case 'b': flag = MOVE_PROMO_B; break;
                case 'r': flag = MOVE_PROMO_R; break;
                case 'q': flag = MOVE_PROMO_Q; break;

                default:
                    fprintf(stderr, "getMoveFromAlgebraic() failed: Invalid promotion move format: %s\n", agbMove);
                    exit(1);
            }
            if (capType != NO_PIECE) flag += MOVE_PROMO_CAPTURE_OFF; // Capture promotion
        }
    }

    if (pType - sideOffset == KING && distance == 2) flag = (to < from) ? MOVE_CASTLE_Q : MOVE_CASTLE_K; // King and distance = 2 -> castle

    return (Move) from | ((Move) to << 6) | ((Move) flag << 12);
}

const char* getAlgebraicFromMove(Move move, char* buffer, uint64_t size) {
    if (size < 6) {
        fprintf(stderr, "getAlgebraicFromMove() failed: Invalid move length!\n");
        exit(1);
    }
    uint32_t from = moveFrom(move), to = moveTo(move), flag = moveFlag(move); // Unpack move data
    buffer[0] = (char) (from & 7) + 'a';
    buffer[1] = (char) (from >> 3) + '1';
    buffer[2] = (char) (to & 7) + 'a';
    buffer[3] = (char) (to >> 3) + '1';
    if (flag >= MOVE_PROMO_CAPTURE_N) flag -= MOVE_PROMO_CAPTURE_OFF;
    switch (flag) {
        case MOVE_PROMO_N: buffer[4] = 'n'; break;
        case MOVE_PROMO_B: buffer[4] = 'b'; break;
        case MOVE_PROMO_R: buffer[4] = 'r'; break;
        case MOVE_PROMO_Q: buffer[4] = 'q'; break;
        default: break;
    }
    buffer[(flag >= MOVE_PROMO_N) ? 5 : 4] = '\0';
    return (const char*) buffer;
}

// Compute merged bitboards (deprecated)
void computeBitboards(Board* board) {
#if defined(USE_PER_PIECE_BITBOARDS)
    board->sidePieces[WHITE] = 0;
    board->sidePieces[BLACK] = 0;
    for (uint32_t bb = 0; bb < TOTAL_PIECE_TYPES; bb += 1) {
        board->sidePieces[bb >= BLACK_OFFSET] |= board->pieces[bb];
    }
    board->occupancy = board->sidePieces[WHITE] | board->sidePieces[BLACK];
#else
    for (uint32_t bb = 0; bb < TOTAL_PIECE_TYPES; bb += 1) {
        board->occupancy |= board->pieces[bb];
    }
#endif
}

// Sets the board to the specified FEN
void setFen(Board* board, const char* fen) {
    // Zero the entire board, ready for new FEN configuration
    memset(board, 0, sizeof(Board));
    for (uint32_t i = 0; i < 64; i += 1) board->mailbox[i] = NO_PIECE;
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
        uint32_t square = (rank << 3) + file;
        uint32_t offset = 0;

        char piece = fen[i];
        if (fen[i] > 'Z') { // Char is lower case
            offset = BLACK_OFFSET; // Change to black bitboards
            piece = fen[i] - 'a' + 'A'; // To upper case
        }

        uint32_t pType = NO_PIECE;
        switch (piece) {
            case 'P': pType = PAWN + offset; break;
            case 'N': pType = KNIGHT + offset; break;
            case 'B': pType = BISHOP + offset; break;
            case 'R': pType = ROOK + offset; break;
            case 'Q': pType = QUEEN + offset; break;
            case 'K': pType = KING + offset; break;
            default: break;
        }
        if (pType < NO_PIECE) placePiece(board, pType, square); // Using the same placePiece() as makeMove()

        // Next file
        file += 1;
        i += 1;
    }
    i += 1; if (i >= fenSize) return; // Increment and check for out of bounds pattern (avoids reading uninitialized memory)

    board->sideToMove = (fen[i] == 'w') ? WHITE : BLACK;
    i += 2; if (i >= fenSize) return; // Increment and check for out of bounds pattern (avoids reading uninitialized memory)

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
        i += 1; if (i >= fenSize) return; // Increment and check for out of bounds pattern (avoids reading uninitialized memory)
        rank = fen[i] - '1';
        board->epTarget = (rank << 3) + file;
    }
    i += 2; if (i >= fenSize) return; // Increment and check for out of bounds pattern (avoids reading uninitialized memory)

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
    i += 1; if (i >= fenSize) return; // Increment and check for out of bounds pattern (avoids reading uninitialized memory)

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
void setFenAndMoves(Board* board, const char* fen, const char** moves, uint64_t moveCount) {
    setFen(board, fen);
    PrevState placeholder;
    for (uint64_t i = 0; i < moveCount; i += 1) makeMove(board, getMoveFromAlgebraic(board, moves[i]), &placeholder);
}

// Returns the FEN string representation of the board
const char* getFen(Board* board, char* fen, uint64_t fenSize) {
    uint32_t i = 0; if (i >= fenSize) return NULL;
    for (uint32_t rank = 8; rank > 0; rank--) {
        uint32_t empty = 0;
        for (uint32_t file = 0; file < 8; file += 1) {
            uint32_t square = ((rank - 1) << 3) + file;
            char piece = ' ';
            if (board->mailbox[square] < NO_PIECE) { // Only if a piece is present
                uint32_t pType = board->mailbox[square];
                if (pType >= BLACK_OFFSET) {
                    pType -= BLACK_OFFSET; // Switch to constant indicees
                    piece = 'a' - 'A'; // To lower case after adding the piece character
                } else {
                    piece = 0;
                }
                switch (pType) {
                    case PAWN: piece += 'P'; break;
                    case KNIGHT: piece += 'N'; break;
                    case BISHOP: piece += 'B'; break;
                    case ROOK: piece += 'R'; break;
                    case QUEEN: piece += 'Q'; break;
                    case KING: piece += 'K'; break;
                    default: piece = ' '; break;
                }
            } else {
                empty += 1; // Count empty square
                continue;
            }

            // Write the empty square count
            if (empty > 0) {
                fen[i] = empty + '0';
                empty = 0;
                i += 1; if (i >= fenSize) return NULL; // Check out of bounds pattern
            }

            // Write the piece char
            fen[i] = piece;
            i += 1; if (i >= fenSize) return NULL; // Check out of bounds pattern
        }

        // Write the empty square count
        if (empty > 0) {
            fen[i] = empty + '0';
            i += 1; if (i >= fenSize) return NULL; // Check out of bounds pattern
        }

        // End of rank
        fen[i] = (rank == 1) ? ' ' : '/';
        i += 1; if (i >= fenSize) return NULL; // Check out of bounds pattern
    }

    // White = 0, Black = 1
    fen[i] = (board->sideToMove) ? 'b' : 'w';
    i += 1; if (i >= fenSize) return NULL; fen[i] = ' '; i += 1; if (i >= fenSize) return NULL; // Go to next position in FEN with whitespace in between

    // Castle rights
    if (!board->castle) {
        fen[i] = '-';
        i += 1; if (i >= fenSize) return NULL; fen[i] = ' '; i += 1; if (i >= fenSize) return NULL; // Go to next position in FEN with whitespace in between
    } else {
        if (board->castle & WHITE_KC) {
            fen[i] = 'K';
            i += 1; if (i >= fenSize) return NULL; // Check out of bounds pattern
        }
        if (board->castle & WHITE_QC) {
            fen[i] = 'Q';
            i += 1; if (i >= fenSize) return NULL; // Check out of bounds pattern
        }
        if (board->castle & BLACK_KC) {
            fen[i] = 'k';
            i += 1; if (i >= fenSize) return NULL; // Check out of bounds pattern
        }
        if (board->castle & BLACK_QC) {
            fen[i] = 'q';
            i += 1; if (i >= fenSize) return NULL; // Check out of bounds pattern
        }
        fen[i] = ' '; i += 1; if (i >= fenSize) return NULL; // Go to next position in FEN with whitespace in between
    }

    if (board->epTarget < NO_EP_TARGET) {
        uint32_t rank = board->epTarget >> 3, file = board->epTarget & 7;
        fen[i] = file + 'a';
        i += 1; if (i >= fenSize) return NULL; // Check out of bounds pattern
        fen[i] = rank + '1';
    } else {
        fen[i] = '-';
    }
    i += 1; if (i >= fenSize) return NULL; fen[i] = ' '; i += 1; if (i >= fenSize) return NULL; // Go to next position in FEN with whitespace in between

    // Write the number of half moves into the FEN
    char buffer[64] = {};
    uint32_t x = board->halfMoves, bufferIdx = 0;
    if (x == 0) { // Edge case if x = 0, write directly to FEN
        fen[i] = '0';
        i += 1; if (i >= fenSize) return NULL; // Check out of bounds pattern
    } else {
        while (bufferIdx < 64 && x > 0) {
            buffer[bufferIdx] = x % 10 + '0';
            x /= 10;
            bufferIdx += 1;
        }
        while (bufferIdx > 0) {
            bufferIdx -= 1;
            fen[i] = buffer[bufferIdx];
            i += 1; if (i >= fenSize) return NULL; // Check out of bounds pattern
        }
    }
    fen[i] = ' '; i += 1; if (i >= fenSize) return NULL; // Go to next position in FEN with whitespace in between

    // Write the number of full moves into the FEN
    for (uint32_t b = 0; b < 64; b += 1) {
        buffer[b] = 0;
    }
    x = board->fullMoves;
    bufferIdx = 0;
    if (x == 0) { // Edge case if x = 0, write directly to FEN
        fen[i] = '0';
        i += 1; if (i >= fenSize) return NULL; // Check out of bounds pattern
    } else {
        while (bufferIdx < 64 && x > 0) {
            buffer[bufferIdx] = x % 10 + '0';
            x /= 10;
            bufferIdx += 1;
        }
        while (bufferIdx > 0) {
            bufferIdx -= 1;
            if (i >= fenSize) return NULL; // Check out of bounds pattern
            fen[i] = buffer[bufferIdx];
            i += 1;
        }
    }
    return (const char*) fen;
}

// Returns the visual string representation of the given bitboard, ready to be printed to the console, or file or anything else
const char* getVisualBitboardString(uint64_t bitboard, char* str, uint64_t size) {
    // Copy the visual template
    if (size < strlen(boardVisualTemplate)) return NULL;
    memcpy(str, boardVisualTemplate, strlen(boardVisualTemplate));

    uint32_t vidx = 85; // First index of a square in the visual template
    for (uint32_t rank = 8; rank > 0; rank--) {
        for (uint32_t file = 0; file < 8; file += 1) {
            uint32_t square = ((rank - 1) << 3) + file;
            char piece = ' ';
            if (bitboard & (1ull << square)) {
                piece = '1';
            }
            str[vidx] = piece;
            vidx += 4; // File offset in template visual
        }
        vidx += 48; // Rank offset from last file to first file in template visual
    }
    return (const char*) str;
}

// Returns the visual string representation of the board, ready to be printed to the console, or file or anything else
const char* getVisualBoardString(Board* board, char* str, uint64_t size) {
    // Copy the visual template
    if (size < strlen(boardVisualTemplate)) return NULL;
    memcpy(str, boardVisualTemplate, strlen(boardVisualTemplate));

    uint32_t vidx = 85; // First index of a square in the visual template
    for (uint32_t rank = 8; rank > 0; rank--) {
        for (uint32_t file = 0; file < 8; file += 1) {
            uint32_t square = ((rank - 1) << 3) + file;
            char piece = ' ';
            uint32_t pType = board->mailbox[square];
            if (pType >= BLACK_OFFSET) {
                pType -= BLACK_OFFSET; // Switch to constant indicees
                piece = 'a' - 'A'; // To lower case after adding the piece character
            } else {
                piece = 0;
            }
            switch (pType) {
                case PAWN: piece += 'P'; break;
                case KNIGHT: piece += 'N'; break;
                case BISHOP: piece += 'B'; break;
                case ROOK: piece += 'R'; break;
                case QUEEN: piece += 'Q'; break;
                case KING: piece += 'K'; break;
                default: piece = ' '; break;
            }
            str[vidx] = piece;
            vidx += 4; // File offset in template visual
        }
        vidx += 48; // Rank offset from last file to first file in template visual
    }
    return (const char*) str;
}

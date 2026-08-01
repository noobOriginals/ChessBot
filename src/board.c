#include "board.h"

#include <stdlib.h>

#include "debug_utils.h"

// Precomputed masks
const u8 castleRightsMask[64] = {11u, 15u, 15u, 15u, 3u, 15u, 15u, 7u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 14u, 15u, 15u, 15u, 12u, 15u, 15u, 13u};
const Bitboard epTargetMask[16] = {0x10000ull, 0x10000000000ull, 0x20000ull, 0x20000000000ull, 0x40000ull, 0x40000000000ull, 0x80000ull, 0x80000000000ull, 0x100000ull, 0x100000000000ull, 0x200000ull, 0x200000000000ull, 0x400000ull, 0x400000000000ull, 0x800000ull, 0x800000000000ull};
const Bitboard epPawnMask[16] = {0x100000000ull, 0x1000000ull, 0x200000000ull, 0x2000000ull, 0x400000000ull, 0x4000000ull, 0x800000000ull, 0x8000000ull, 0x1000000000ull, 0x10000000ull, 0x2000000000ull, 0x20000000ull, 0x4000000000ull, 0x40000000ull, 0x8000000000ull, 0x80000000ull};

// Basic board struct handles
Board* createBoard() {
    return (Board*) calloc(1, sizeof(Board));
}

void deleteBoard(Board* board) {

    ASSERT_MSG(board, "deleteBoard() failed: board pointer cannot be NULL");

    free(board);
}

// Basic board ops utility
void placePiece(Board* board, u8 piece, u8 square) {

    ASSERT_MSG(board, "placePiece() failed: board pointer cannot be NULL");
    ASSERT_MSG(piece > 1 && piece < 14, "placePiece() failed: piece type out of bounds");
    ASSERT_MSG(square < 64, "placePiece() failed: square index out of bounds");
    ASSERT_MSG(board->mailbox[square] == NO_PIECE, "placePiece() failed: cannot place a piece onto an occupied square");

    Bitboard mask = bbsq(square);
    board->all |= mask;
    board->bb[pside(piece)] |= mask;
    board->bb[piece] |= mask;
    board->mailbox[square] = piece;
}

void removePiece(Board* board, u8 piece, u8 square) {

    ASSERT_MSG(board, "removePiece() failed: board pointer cannot be NULL");
    ASSERT_MSG(piece > 1 && piece < 14, "removePiece() failed: piece type out of bounds");
    ASSERT_MSG(square < 64, "removePiece() failed: square index out of bounds");

    Bitboard mask = ~bbsq(square);
    board->all &= mask;
    board->bb[pside(piece)] &= mask;
    board->bb[piece] &= mask;
    board->mailbox[square] = NO_PIECE;
}

void movePiece(Board* board, u8 piece, u8 from, u8 to) {

    ASSERT_MSG(board, "movePiece() failed: board pointer cannot be NULL");
    ASSERT_MSG(piece > 1 && piece < 14, "movePiece() failed: piece type out of bounds");
    ASSERT_MSG(from < 64, "movePiece() failed: from index out of bounds");
    ASSERT_MSG(to < 64, "movePiece() failed: to index out of bounds");
    ASSERT_MSG(board->mailbox[from] == piece, "movePiece() failed: given piece is not equal to mailbox piece on from square");
    ASSERT_MSG(board->mailbox[to] == NO_PIECE, "movePiece() failed: cannot move a piece onto an occupied square");

    Bitboard mask = bbsq(from) | bbsq(to);
    board->all ^= mask;
    board->bb[pside(piece)] ^= mask;
    board->bb[piece] ^= mask;
    board->mailbox[from] = NO_PIECE;
    board->mailbox[to] = piece;
}

// Most important of all
void makeMove(Board* board, Move move, UndoState* state) {

    ASSERT_MSG(board, "makeMove() failed: board pointer cannot be NULL");
    ASSERT_MSG(state, "makeMove() failed: undo state pointer cannot be NULL");

    // Unpack move data
    u8 from = (u8) moveFrom(move), to = (u8) moveTo(move), flag = (u8) moveFlag(move);

    ASSERT_MSG(from != to, "makeMove() failed: from index cannot equal to index");

    u8 piece = board->mailbox[from], capture = board->mailbox[to];

    // Store previous state
    state->capture = capture;
    state->epTarget = board->epTarget;
    state->castle = board->castle;
    state->halfMoves = board->halfMoves;

    // Update ep target
    board->epTarget = NO_EP_TARGET;

    switch (flag) {
    case MOVE_QUIET:
        movePiece(board, piece, from, to);
        break;

    case MOVE_DOUBLE_PUSH:
        movePiece(board, piece, from, to);
        board->epTarget = epTargetMask[((to & 7) << 1) | board->side];
        break;

    case MOVE_CASTLE_K:
        movePiece(board, piece, from, to);
        movePiece(board, ROOK | board->side, from + 3, to - 1);
        break;

    case MOVE_CASTLE_Q:
        movePiece(board, piece, from, to);
        movePiece(board, ROOK | board->side, from - 4, to + 1);
        break;

    case MOVE_CAPTURE:
        removePiece(board, capture, to);
        movePiece(board, piece, from, to);
        break;

    case MOVE_EP_CAPTURE:

        ASSERT_MSG(to == ctzll(board->epTarget), "makeMove() failed: MOVE_EP_CAPTURE flag passed but to square is not ep target");

        removePiece(board, PAWN | (board->side ^ 1u), ctzll(epPawnMask[((to & 7) << 1) | board->side]));
        movePiece(board, piece, from, to);
        break;

    case MOVE_PROMO_N:
        removePiece(board, piece, from);
        placePiece(board, KNIGHT | board->side, to);
        break;

    case MOVE_PROMO_B:
        removePiece(board, piece, from);
        placePiece(board, BISHOP | board->side, to);
        break;

    case MOVE_PROMO_R:
        removePiece(board, piece, from);
        placePiece(board, ROOK | board->side, to);
        break;

    case MOVE_PROMO_Q:
        removePiece(board, piece, from);
        placePiece(board, QUEEN | board->side, to);
        break;

    case MOVE_PROMO_CAPTURE_N:
        removePiece(board, capture, to);
        removePiece(board, piece, from);
        placePiece(board, KNIGHT | board->side, to);
        break;

    case MOVE_PROMO_CAPTURE_B:
        removePiece(board, capture, to);
        removePiece(board, piece, from);
        placePiece(board, BISHOP | board->side, to);
        break;

    case MOVE_PROMO_CAPTURE_R:
        removePiece(board, capture, to);
        removePiece(board, piece, from);
        placePiece(board, ROOK | board->side, to);
        break;

    case MOVE_PROMO_CAPTURE_Q:
        removePiece(board, capture, to);
        removePiece(board, piece, from);
        placePiece(board, QUEEN | board->side, to);
        break;

    default:

        ASSERT_MSG(0, "makeMove() failed: invalid flag");

        break;
    }

    // Update board state
    board->castle &= castleRightsMask[from] & castleRightsMask[to];
    board->halfMoves = (ptype(piece) == PAWN || capture != NO_PIECE || flag == MOVE_EP_CAPTURE) ? 0 : board->halfMoves + 1;
    board->fullMoves += board->side;
    board->side ^= 1u;
}

void unmakeMove(Board* board, Move move, UndoState* state) {

    ASSERT_MSG(board, "unmakeMove() failed: board pointer cannot be NULL");
    ASSERT_MSG(state, "unmakeMove() failed: undo state pointer cannot be NULL");

    // Unpack move data
    u8 from = (u8) moveFrom(move), to = (u8) moveTo(move), flag = (u8) moveFlag(move);

    ASSERT_MSG(from != to, "unmakeMove() failed: from index cannot equal to index");

    u8 piece = board->mailbox[to], capture = state->capture;

    // Restore board state
    board->side ^= 1u;
    board->fullMoves -= board->side;
    board->halfMoves = state->halfMoves;
    board->castle = state->castle;
    board->epTarget = state->epTarget;

    switch (flag) {
    case MOVE_QUIET:
        movePiece(board, piece, to, from);
        break;

    case MOVE_DOUBLE_PUSH:
        movePiece(board, piece, to, from);
        break;

    case MOVE_CASTLE_K:
        movePiece(board, piece, to, from);
        movePiece(board, ROOK | board->side, to - 1, from + 3);
        break;

    case MOVE_CASTLE_Q:
        movePiece(board, piece, to, from);
        movePiece(board, ROOK | board->side, to + 1, from - 4);
        break;

    case MOVE_CAPTURE:
        movePiece(board, piece, to, from);
        placePiece(board, capture, to);
        break;

    case MOVE_EP_CAPTURE:

        ASSERT_MSG(to == ctzll(board->epTarget), "unmakeMove() failed: MOVE_EP_CAPTURE flag passed but to square is not ep target");

        movePiece(board, piece, to, from);
        placePiece(board, PAWN | (board->side ^ 1u), ctzll(epPawnMask[((to & 7) << 1) | board->side]));
        break;

    case MOVE_PROMO_N:
        removePiece(board, KNIGHT | board->side, to);
        placePiece(board, piece, from);
        break;

    case MOVE_PROMO_B:
        removePiece(board, BISHOP | board->side, to);
        placePiece(board, piece, from);
        break;

    case MOVE_PROMO_R:
        removePiece(board, ROOK | board->side, to);
        placePiece(board, piece, from);
        break;

    case MOVE_PROMO_Q:
        removePiece(board, QUEEN | board->side, to);
        placePiece(board, piece, from);
        break;

    case MOVE_PROMO_CAPTURE_N:
        removePiece(board, KNIGHT | board->side, to);
        placePiece(board, piece, from);
        placePiece(board, capture, to);
        break;

    case MOVE_PROMO_CAPTURE_B:
        removePiece(board, BISHOP | board->side, to);
        placePiece(board, piece, from);
        placePiece(board, capture, to);
        break;

    case MOVE_PROMO_CAPTURE_R:
        removePiece(board, ROOK | board->side, to);
        placePiece(board, piece, from);
        placePiece(board, capture, to);
        break;

    case MOVE_PROMO_CAPTURE_Q:
        removePiece(board, QUEEN | board->side, to);
        placePiece(board, piece, from);
        placePiece(board, capture, to);
        break;

    default:

        ASSERT_MSG(0, "unmakeMove() failed: invalid flag");

        break;
    }
}

// Loading positions to the board
Move stringToMove(Board* board, const char* str) {
    return 0;
}

const char* moveToString(Move move, char* buffer, u64 size) {
    return buffer;
}

void setFEN(Board* board, const char* fen) {

}

void setPosition(Board* board, const char* fen, const char** moves, u64 moveCount) {

}

const char* getFEN(Board* board, char* buffer, u64 size) {
    return buffer;
}

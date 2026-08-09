#include "board.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "debug_utils.h"

// Precomputed masks
const u8 castleRightsMask[64] = {11u, 15u, 15u, 15u, 3u, 15u, 15u, 7u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 15u, 14u, 15u, 15u, 15u, 12u, 15u, 15u, 13u};
const Bitboard epTargetMask[16] = {0x10000ull, 0x10000000000ull, 0x20000ull, 0x20000000000ull, 0x40000ull, 0x40000000000ull, 0x80000ull, 0x80000000000ull, 0x100000ull, 0x100000000000ull, 0x200000ull, 0x200000000000ull, 0x400000ull, 0x400000000000ull, 0x800000ull, 0x800000000000ull};
const Bitboard epPawnMask[16] = {0x100000000ull, 0x1000000ull, 0x200000000ull, 0x2000000ull, 0x400000000ull, 0x4000000ull, 0x800000000ull, 0x8000000ull, 0x1000000000ull, 0x10000000ull, 0x2000000000ull, 0x20000000ull, 0x4000000000ull, 0x40000000ull, 0x8000000000ull, 0x80000000ull};

// Basic board struct handles
Board* createBoard(void) {
    return (Board*) calloc(1, sizeof(Board));
}

void deleteBoard(Board* board) {

    PUSH_STACK_TRACE("deleteBoard()");
    ASSERT_MSG(board, "deleteBoard() failed: board pointer cannot be NULL");

    free(board);

    POP_STACK_TRACE();
}

// Basic board ops utility
void placePiece(Board* board, u8 piece, u8 square) {

    PUSH_STACK_TRACE("placePiece()");
    ASSERT_MSG(board, "placePiece() failed: board pointer cannot be NULL");
    ASSERT_MSG(piece > 1 && piece < 14, "placePiece() failed: piece type out of bounds");
    ASSERT_MSG(square < 64, "placePiece() failed: square index out of bounds");
    ASSERT_MSG(board->mailbox[square] == NO_PIECE, "placePiece() failed: cannot place a piece onto an occupied square");

    Bitboard mask = bbsq(square);
    board->all |= mask;
    board->bb[pside(piece)] |= mask;
    board->bb[piece] |= mask;
    board->mailbox[square] = piece;

    POP_STACK_TRACE();
}

void removePiece(Board* board, u8 piece, u8 square) {

    PUSH_STACK_TRACE("removePiece()");
    ASSERT_MSG(board, "removePiece() failed: board pointer cannot be NULL");
    ASSERT_MSG(piece > 1 && piece < 14, "removePiece() failed: piece type out of bounds");
    ASSERT_MSG(square < 64, "removePiece() failed: square index out of bounds");

    Bitboard mask = ~bbsq(square);
    board->all &= mask;
    board->bb[pside(piece)] &= mask;
    board->bb[piece] &= mask;
    board->mailbox[square] = NO_PIECE;

    POP_STACK_TRACE();
}

void movePiece(Board* board, u8 piece, u8 from, u8 to) {

    PUSH_STACK_TRACE("movePiece()");
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

    POP_STACK_TRACE();
}

// Most important of all
void makeMove(Board* board, Move move, UndoState* state) {

    PUSH_STACK_TRACE("makeMove()");
    ASSERT_MSG(board, "makeMove() failed: board pointer cannot be NULL");
    ASSERT_MSG(state, "makeMove() failed: undo state pointer cannot be NULL");

    // Unpack move type
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

        ASSERT_MSG(bbsq(to) == state->epTarget, "makeMove() failed: MOVE_EP_CAPTURE flag passed but to square is not ep target");

        removePiece(board, PAWN | (board->side ^ 1u), (u8) ctzll(epPawnMask[((to & 7) << 1) | board->side]));
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

    POP_STACK_TRACE();
}

void unmakeMove(Board* board, Move move, UndoState* state) {

    PUSH_STACK_TRACE("unmakeMove()");
    ASSERT_MSG(board, "unmakeMove() failed: board pointer cannot be NULL");
    ASSERT_MSG(state, "unmakeMove() failed: undo state pointer cannot be NULL");

    // Unpack move type
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

        ASSERT_MSG(bbsq(to) == state->epTarget, "unmakeMove() failed: MOVE_EP_CAPTURE flag passed but to square is not ep target");

        movePiece(board, piece, to, from);
        placePiece(board, PAWN | (board->side ^ 1u), (u8) ctzll(epPawnMask[((to & 7) << 1) | board->side]));
        break;

    case MOVE_PROMO_N:
        removePiece(board, piece, to);
        placePiece(board, PAWN | board->side, from);
        break;

    case MOVE_PROMO_B:
        removePiece(board, piece, to);
        placePiece(board, PAWN | board->side, from);
        break;

    case MOVE_PROMO_R:
        removePiece(board, piece, to);
        placePiece(board, PAWN | board->side, from);
        break;

    case MOVE_PROMO_Q:
        removePiece(board, piece, to);
        placePiece(board, PAWN | board->side, from);
        break;

    case MOVE_PROMO_CAPTURE_N:
        removePiece(board, piece, to);
        placePiece(board, PAWN | board->side, from);
        placePiece(board, capture, to);
        break;

    case MOVE_PROMO_CAPTURE_B:
        removePiece(board, piece, to);
        placePiece(board, PAWN | board->side, from);
        placePiece(board, capture, to);
        break;

    case MOVE_PROMO_CAPTURE_R:
        removePiece(board, piece, to);
        placePiece(board, PAWN | board->side, from);
        placePiece(board, capture, to);
        break;

    case MOVE_PROMO_CAPTURE_Q:
        removePiece(board, piece, to);
        placePiece(board, PAWN | board->side, from);
        placePiece(board, capture, to);
        break;

    default:

        ASSERT_MSG(0, "unmakeMove() failed: invalid flag");

        break;
    }

    POP_STACK_TRACE();
}

// Loading positions to the board
Move stringToMove(const Board* board, const char* str) {

    PUSH_STACK_TRACE("stringToMove()");
    ASSERT_MSG(board, "stringToMove() failed: board pointer cannot be NULL");

    if (strlen(str) < 4) goto fail;

    // Unpack string
    u8 from = ((str[1] - '1') << 3) + str[0] - 'a';
    u8 to = ((str[3] - '1') << 3) + str[2] - 'a';
    u8 flag = board->mailbox[to] ? MOVE_CAPTURE : MOVE_QUIET;

    // Handle special falgs
    u8 type = ptype(board->mailbox[from]);
    u8 dist = (u8) abs((i8) to - from);
    if (type == PAWN) {
        if (dist == 16) {
            flag = MOVE_DOUBLE_PUSH;
        } else if (bbsq(to) == board->epTarget) {
            flag = MOVE_EP_CAPTURE;
        } else if ((to >> 3) == 0 || (to >> 3) == 7) {
            if (strlen(str) < 5) goto fail;
            switch (str[4]) {
                case 'n': flag = flag ? MOVE_PROMO_CAPTURE_N : MOVE_PROMO_N; break;
                case 'b': flag = flag ? MOVE_PROMO_CAPTURE_B : MOVE_PROMO_B; break;
                case 'r': flag = flag ? MOVE_PROMO_CAPTURE_R : MOVE_PROMO_R; break;
                case 'q': flag = flag ? MOVE_PROMO_CAPTURE_Q : MOVE_PROMO_Q; break;
                default: goto fail;
            }
        }
    } else if (type == KING && dist == 2) {
        flag = ((i8) to - from) > 0 ? MOVE_CASTLE_K : MOVE_CASTLE_Q;
    }

    POP_STACK_TRACE();

    // Pack move type
    return (Move) from | ((Move) to << 6) | ((Move) flag << 12);

fail: // Fail label
    fprintf(stderr, "stringToMove() failed: invalid string format\n");

    POP_STACK_TRACE();

    return 0;
}

char* moveToString(Move move, char* buffer, u64 size) {

    PUSH_STACK_TRACE("moveToString()");
    ASSERT_MSG(move, "moveToString() failed: move cannot be 0 (zero)");

    if (size < 5) goto fail;

    // Unpack move type
    u8 from = (u8) moveFrom(move), to = (u8) moveTo(move), flag = (u8) moveFlag(move);

    // Write to buffer
    buffer[0] = (from & 7) + 'a';
    buffer[1] = (from >> 3) + '1';
    buffer[2] = (to & 7) + 'a';
    buffer[3] = (to >> 3) + '1';
    buffer[4] = 0;

    // Handle promotions
    flag = flag >= MOVE_PROMO_CAPTURE_N ? flag - MOVE_PROMO_CAPTURE_OFF : flag;
    if (flag >= MOVE_PROMO_N) {
        if (size < 6) goto fail;
        switch (flag) {
            case MOVE_PROMO_N: buffer[4] = 'n'; break;
            case MOVE_PROMO_B: buffer[4] = 'b'; break;
            case MOVE_PROMO_R: buffer[4] = 'r'; break;
            case MOVE_PROMO_Q: buffer[4] = 'q'; break;
            default: fprintf(stderr, "moveToString() failed: invalid move flag\n"); POP_STACK_TRACE(); return NULL;
        }
        buffer[5] = 0;
    }

    POP_STACK_TRACE();

    return buffer;

fail: // Fail label
    fprintf(stderr, "moveToString() failed: buffer size too small\n");

    POP_STACK_TRACE();

    return NULL;
}

i32 setFEN(Board* board, const char* fen) {

    PUSH_STACK_TRACE("setFEN()");
    ASSERT_MSG(board, "setFEN() failed: board pointer cannot be NULL");

    memset(board, 0, sizeof(Board)); // Zero the board
    u64 len = strlen(fen), i;
    i8 rank = 7, file = 0;
    for (i = 0; i < len && fen[i] != ' '; i++) {
        // Next rank
        if (fen[i] == '/') {
            file = 0;
            rank--;
            continue;
        }

        // Skip empty squares
        if (fen[i] < '9' && fen[i] > '0') {
            file += fen[i] - '0';
            continue;
        }

        // Decide piece type
        u8 side = (fen[i] > 'Z') ? BLACK : WHITE;
        char piece = side ? fen[i] - 'a' + 'A' : fen[i]; // - 'a' + 'A' -> switch to upper case if needed
        switch (piece) {
            case 'P': placePiece(board, PAWN | side, (rank << 3) + file); break;
            case 'N': placePiece(board, KNIGHT | side, (rank << 3) + file); break;
            case 'B': placePiece(board, BISHOP | side, (rank << 3) + file); break;
            case 'R': placePiece(board, ROOK | side, (rank << 3) + file); break;
            case 'Q': placePiece(board, QUEEN | side, (rank << 3) + file); break;
            case 'K': placePiece(board, KING | side, (rank << 3) + file); break;
            default: goto fail;
        }

        file++;
    }
    i++; if (i >= len) goto fail;

    // Read side to move
    board->side = (fen[i++] == 'w') ? WHITE : BLACK;
    i++; if (i >= len) goto fail;

    // Read castle rights
    while (i < len && fen[i] != ' ') {
        switch (fen[i++]) {
            case 'K': board->castle |= WHITE_KC; break;
            case 'Q': board->castle |= WHITE_QC; break;
            case 'k': board->castle |= BLACK_KC; break;
            case 'q': board->castle |= BLACK_QC; break;
            default: break;
        }
    }
    i++; if (i >= len) goto fail;

    // Read ep target
    if (fen[i] != '-') {
        file = fen[i++] - 'a'; if (i >= len) goto fail;
        rank = fen[i++] - '1'; if (i >= len) goto fail;
        board->epTarget = bbsq((rank << 3) + file);
    } else {
        i++;
    }
    i++; if (i >= len) goto fail;

    // Read half move count
    while (i < len && fen[i] != ' ') {
        board->halfMoves *= 10;
        board->halfMoves += fen[i] - '0';
        i++;
    }
    i++; if (i >= len) goto fail;

    // Read full move count
    while (i < len && fen[i] != ' ') {
        board->fullMoves *= 10;
        board->fullMoves += fen[i] - '0';
        i++;
    }

    POP_STACK_TRACE();

    return 0;

fail: // Fail label
    fprintf(stderr, "setFEN() failed: invalid FEN format\n");

    POP_STACK_TRACE();

    return 1;
}

i32 setPosition(Board* board, const char* fen, const char** moves, u64 moveCount) {

    PUSH_STACK_TRACE("setPosition()");
    ASSERT_MSG(board, "setPosition() failed: board pointer cannot be NULL");

    // Set fen
    if (setFEN(board, fen)) {

        POP_STACK_TRACE();

        return 1;
    }

    // Then make each move. TODO: check if move is legal or not.
    UndoState dummyState;
    for (u64 i = 0; i < moveCount; i++) {
        Move m = stringToMove(board, moves[i]);
        if (!m) return 1;
        makeMove(board, m, &dummyState);
    }

    POP_STACK_TRACE();

    return 0;
}

char* getFEN(const Board* board, char* buffer, u64 size) {

    PUSH_STACK_TRACE("getFEN()");
    ASSERT_MSG(board, "getFEN() failed: board pointer cannot be NULL");

    u64 i = 0;
    for (i8 rank = 7; rank > -1; rank--) {
        u8 empty = 0;
        for (i8 file = 0; file < 8; file++) {
            u8 sq = (rank << 3) + file;
            if (board->mailbox[sq]) {
                if (empty) {
                    buffer[i++] = empty + '0';
                    empty = 0;
                    if (i >= size) goto fail;
                }
                switch (ptype(board->mailbox[sq])) {
                    case PAWN: buffer[i] = 'P'; break;
                    case KNIGHT: buffer[i] = 'N'; break;
                    case BISHOP: buffer[i] = 'B'; break;
                    case ROOK: buffer[i] = 'R'; break;
                    case QUEEN: buffer[i] = 'Q'; break;
                    case KING: buffer[i] = 'K'; break;
                    default: fprintf(stderr, "getFEN() failed: invalid piece type\n"); return NULL;
                }
                buffer[i++] += (pside(board->mailbox[sq])) ? 'a' - 'A' : 0;
                if (i >= size) goto fail;
            } else {
                empty++;
            }
        }
        if (empty) {
            buffer[i++] = empty + '0';
            if (i >= size) goto fail;
        }
        buffer[i++] = (rank == 0) ? ' ' : '/';
        if (i >= size) goto fail;
    }

    // Write side to move
    buffer[i++] = board->side ? 'b' : 'w';
    buffer[i++] = ' '; if (i >= size) goto fail;

    // Write castle rights
    if (board->castle) {
        if (i + 4 >= size) goto fail;
        if (board->castle & WHITE_KC) buffer[i++] = 'K';
        if (board->castle & WHITE_QC) buffer[i++] = 'Q';
        if (board->castle & BLACK_KC) buffer[i++] = 'k';
        if (board->castle & BLACK_QC) buffer[i++] = 'q';
    } else {
        buffer[i++] = '-'; if (i >= size) goto fail;
    }
    buffer[i++] = ' '; if (i >= size) goto fail;

    // Write ep target
    if (board->epTarget) {
        if (i + 2 >= size) goto fail;
        u8 sq = (u8) ctzll(board->epTarget);
        buffer[i++] = (sq & 7) + 'a';
        buffer[i++] = (sq >> 3) + '1';
    } else {
        buffer[i++] = '-'; if (i >= size) goto fail;
    }
    buffer[i++] = ' '; if (i >= size) goto fail;

    u16 x, y;

    // Write half moves
    x = board->halfMoves;
    y = 0;
    while (x) {
        y *= 10;
        y += x % 10;
        x /= 10;
    }
    if (!y) buffer[i++] = '0';
    while (y) {
        buffer[i++] = (y % 10) + '0';
        if (i >= size) goto fail;
        y /= 10;
    }
    buffer[i++] = ' '; if (i >= size) goto fail;

    // Write full moves
    x = board->fullMoves;
    y = 0;
    while (x) {
        y *= 10;
        y += x % 10;
        x /= 10;
    }
    if (!y) buffer[i++] = '0';
    while (y) {
        buffer[i++] = (y % 10) + '0';
        if (i >= size) goto fail;
        y /= 10;
    }
    buffer[i] = 0;

    POP_STACK_TRACE();

    return buffer;

fail: // Fail label
    fprintf(stderr, "getFEN() failed: buffer size too small\n");

    POP_STACK_TRACE();

    return NULL;
}

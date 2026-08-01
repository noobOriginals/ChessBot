#ifndef BOARD_H
#define BOARD_H

#include "types.h"
#include "bitboard.h"

#if defined(__cplusplus)
extern "C" {
#endif

#define WHITE 0u
#define BLACK 1u

#define PAWN 2u
#define KNIGHT 4u
#define BISHOP 6u
#define ROOK 8u
#define QUEEN 10u
#define KING 12u
#define PIECE_MASK 14u
#define SIDE_MASK 1u
#define NO_PIECE 0u

#define ptype(x) (x & PIECE_MASK)
#define pside(x) (x & SIDE_MASK)

#define NO_EP_TARGET 0ull

#define WHITE_KC 0b1000u
#define WHITE_QC 0b0100u
#define BLACK_KC 0b0010u
#define BLACK_QC 0b0001u

#define STARTPOS_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

#define MOVE_FROM_MASK 0b0000000000111111u
#define MOVE_TO_MASK   0b0000111111000000u
#define MOVE_FLAG_MASK 0b1111000000000000u

#define MOVE_QUIET 0u
#define MOVE_DOUBLE_PUSH 1u
#define MOVE_CASTLE_K 2u
#define MOVE_CASTLE_Q 3u
#define MOVE_CAPTURE 4u
#define MOVE_EP_CAPTURE 5u
#define MOVE_PROMO_N 8u
#define MOVE_PROMO_B 9u
#define MOVE_PROMO_R 10u
#define MOVE_PROMO_Q 11u
#define MOVE_PROMO_CAPTURE_N 12u
#define MOVE_PROMO_CAPTURE_B 13u
#define MOVE_PROMO_CAPTURE_R 14u
#define MOVE_PROMO_CAPTURE_Q 15u

#define MOVE_PROMO_CAPTURE_OFF 4u

#define moveFrom(x) ((u16) x & MOVE_FROM_MASK)
#define moveTo(x) (((u16) x & MOVE_TO_MASK) >> 6)
#define moveFlag(x) (((u16) x & MOVE_FLAG_MASK) >> 12)

// Board struct
typedef struct {
    // Actual position
    Bitboard all;
    Bitboard bb[14];
    u8 mailbox[64];

    // Additional info required for movegen
    Bitboard epTarget;
    u8 castle, side;

    // Extra info
    u16 halfMoves, fullMoves;
} Board;

// Move type (all move data fits inside 16 bits)
typedef u16 Move;

// Store previous board state to restore with unmakeMove()
typedef struct {
    u8 capture;
    Bitboard epTarget;
    u8 castle;
    u16 halfMoves;
} UndoState;

// Precomputed masks
extern const u8 castleRightsMask[64];
extern const Bitboard epTargetMask[16];
extern const Bitboard epPawnMask[16];

// Basic board struct handles
Board* createBoard();
void deleteBoard(Board* board);

// Basic board ops utility
void placePiece(Board* board, u8 piece, u8 square);
void removePiece(Board* board, u8 piece, u8 square);
void movePiece(Board* board, u8 piece, u8 from, u8 to);

// Most important of all
void makeMove(Board* board, Move move, UndoState* state);
void unmakeMove(Board* board, Move move, UndoState* state);

// Loading positions to the board
Move stringToMove(Board* board, const char* str);
const char* moveToString(Move move, char* buffer, u64 size);
void setFEN(Board* board, const char* fen);
void setPosition(Board* board, const char* fen, const char** moves, u64 moveCount);
const char* getFEN(Board* board, char* buffer, u64 size);

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // BOARD_H

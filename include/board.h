#ifndef BOARD_H
#define BOARD_H

// Std includes
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Bitboard array constants
#define TOTAL_PIECE_TYPES 12
#define BLACK_OFFSET 6
#define PAWN 0
#define KNIGHT 1
#define BISHOP 2
#define ROOK 3
#define QUEEN 4
#define KING 5

// Side to move constants
#define WHITE 0
#define BLACK 1

// Castle rights constants
#define WHITE_KC 0b1000
#define WHITE_QC 0b0100
#define BLACK_KC 0b0010
#define BLACK_QC 0b0001

// Utility
#define NO_PIECE 12
#define NO_EP_TARGET 64
#define FILE_A 0b0000000100000001000000010000000100000001000000010000000100000001ull
#define FILE_B 0b0000001000000010000000100000001000000010000000100000001000000010ull
#define FILE_C 0b0000010000000100000001000000010000000100000001000000010000000100ull
#define FILE_D 0b0000100000001000000010000000100000001000000010000000100000001000ull
#define FILE_E 0b0001000000010000000100000001000000010000000100000001000000010000ull
#define FILE_F 0b0010000000100000001000000010000000100000001000000010000000100000ull
#define FILE_G 0b0100000001000000010000000100000001000000010000000100000001000000ull
#define FILE_H 0b1000000010000000100000001000000010000000100000001000000010000000ull
#define RANK_1 0b0000000000000000000000000000000000000000000000000000000011111111ull
#define RANK_2 0b0000000000000000000000000000000000000000000000001111111100000000ull
#define RANK_3 0b0000000000000000000000000000000000000000111111110000000000000000ull
#define RANK_4 0b0000000000000000000000000000000011111111000000000000000000000000ull
#define RANK_5 0b0000000000000000000000001111111100000000000000000000000000000000ull
#define RANK_6 0b0000000000000000111111110000000000000000000000000000000000000000ull
#define RANK_7 0b0000000011111111000000000000000000000000000000000000000000000000ull
#define RANK_8 0b1111111100000000000000000000000000000000000000000000000000000000ull
#define MOVE_FROM_MASK 0b0000000000111111u
#define MOVE_TO_MASK   0b0000111111000000u
#define MOVE_FLAG_MASK 0b1111000000000000u
#define MOVE_QUIET 0
#define MOVE_DOUBLE_PUSH 1
#define MOVE_CASTLE_K 2
#define MOVE_CASTLE_Q 3
#define MOVE_CAPTURE 4
#define MOVE_EP_CAPTURE 5
#define MOVE_PROMO_N 8
#define MOVE_PROMO_B 9
#define MOVE_PROMO_R 10
#define MOVE_PROMO_Q 11
#define MOVE_PROMO_CAPTURE_OFF 4
#define MOVE_PROMO_CAPTURE_N 12
#define MOVE_PROMO_CAPTURE_B 13
#define MOVE_PROMO_CAPTURE_R 14
#define MOVE_PROMO_CAPTURE_Q 15
#define moveFrom(x) ((uint32_t) x & MOVE_FROM_MASK)
#define moveTo(x) (((uint32_t) x & MOVE_TO_MASK) >> 6)
#define moveFlag(x) (((uint32_t) x & MOVE_FLAG_MASK) >> 12)
#if defined(_MSC_VER)
    #include <intrin.h>
    #define popcount64(x) __popcnt64(x)
#else
    #define popcount64(x) __builtin_popcountll(x)
#endif
#define STARTPOS_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

// Board struct
typedef struct {
    uint64_t pieces[TOTAL_PIECE_TYPES], whitePieces, blackPieces, allPieces; // Bitboards (12 for each piece, plus the computed ones for white, black and all pieces)
    uint32_t mailbox[64];
    uint8_t castle, sideToMove; // Store int 8 bits, no more needed
    uint32_t epTarget, halfMoves, fullMoves; // Additional board info for en passant and 50-move rule
} Board;

// Move and unmake move structs
typedef uint16_t Move; // 6 bits from + 6 bits to + 4 bits for flags
typedef struct {
    uint16_t captured;
    uint8_t castle;
    uint32_t epTarget, halfMoves;
} PrevState;

// Castle rights masks
extern const uint8_t castleRightsMask[64];

// En passant capture offset
extern const int32_t epCaptureOffset[2];

// Create the Board struct's object (allocate memory and clear board fields)
Board* createBoard();

// Create the Board struct's object (allocate memory) and set the board in the starting position via setFen(...)
Board* createDefaultBoard();

// Free memory (just free())
void destroyBoard(Board* board);

// makeMove utility
void placePiece(Board* board, uint32_t pType, uint32_t square);
void removePiece(Board* board, uint32_t pType, uint32_t square);
void movePiece(Board* board, uint32_t pType, uint32_t from, uint32_t to);

// Make moves on the boards
void makeMove(Board* board, Move move, PrevState* state);
void unmakeMove(Board* board, Move move, PrevState* state);

// Compute merged bitboards (deprecated)
#if defined(_MSC_VER)
    __declspec(deprecated)
#else
    __attribute__((deprecated))
#endif
void computeBitboards(Board* board);

// Sets the board to the specified FEN
void setFen(Board* board, const char* fen);

// Sets the board to the specified FEN and then executes the given move array
void setFenAndMoves(Board* board, const char* fen, const char** moves, uint32_t moveCount);

// Returns the FEN string representation of the board
int32_t getFen(Board* board, char* fen, uint64_t size);

// Returns the visual string representation of the given bitboard, ready to be printed to the console, or file or anything else
int32_t getVisualBitboardString(uint64_t bitboard, char* str, uint64_t size);

// Returns the visual string representation of the board, ready to be printed to the console, or file or anything else
int32_t getVisualBoardString(Board* board, char* str, uint64_t size);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // BOARD_H

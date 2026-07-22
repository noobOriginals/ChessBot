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

// En passant target "null"
#define NO_EP_TARGET 64

// Utility
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
    uint8_t castle, sideToMove; // Store int 8 bits, no more needed
    uint32_t epTarget, halfMoves, fullMoves; // Additional board info for en passant and 50-move rule
} Board;

// Create the Board struct's object (allocate memory)
Board* createBoard();

// Create the Board struct's object (allocate memory) and set the board in the starting position via setFen(...)
Board* createDefaultBoard();

// Free memory (just free())
void destroyBoard(Board* board);

// Compute merged bitboards
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

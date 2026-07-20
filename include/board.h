#ifndef BOARD_H
#define BOARD_H

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

// Side to move contants
#define WHITE 0
#define BLACK 1

// Castle rights contants
#define WHITE_KC 0b1000
#define WHITE_QC 0b0100
#define BLACK_KC 0b0010
#define BLACK_QC 0b0001

// En passant target "null"
#define NO_EP_TARGET 64

// Utility
#define STARTPOS_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

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
const char* getFen(Board* board);

// Returns the visual string representation of the board, ready to be printed to the console, or file or anything else
int getVisualString(Board* board, char* str, uint64_t size);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // BOARD_H

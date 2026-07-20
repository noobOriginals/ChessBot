#ifndef BOARD_H
#define BOARD_H

#ifdef __cplusplus
extern "C" {
#endif

typedef signed char schar;
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;
typedef unsigned long long ullong;

#define TOTAL_PIECE_TYPES 12
#define BLACK_OFFSET 6
#define PAWN 0
#define KNIGHT 1
#define BISHOP 2
#define ROOK 3
#define QUEEN 4
#define KING 5

#define WHITE 0
#define BLACK 1

#define NO_EP_TARGET 64

#define STARTPOS_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

typedef struct {
    ullong pieces[TOTAL_PIECE_TYPES], whitePieces, blackPieces, allPieces;
    uchar castle, sideToMove;
    uint epTarget, halfMoves, fullMoves;
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
void setFenAndMoves(Board* board, const char* fen, const char** moves, uint moveCount);

// Returns the FEN string representation of the board
const char* getFen(Board* board);

// Returns the visual string representation of the board, ready to be printed to the console, or file or anything else
void getVisualString(Board* board, char* str, ullong len);


#ifdef __cplusplus
} // extern "C"
#endif

#endif // BOARD_H

#ifndef ATTACKS_H
#define ATTACKS_H

// Std includes
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Constants
extern const uint64_t bishopMagics[64];
extern const uint64_t rookMagics[64];

// Precomputed attack bitboards
extern uint64_t pawnAttacks[128];
extern uint64_t knightAttacks[64];
extern uint64_t kingAttacks[64];

// Precomputed relevant occupancy masks
extern uint64_t bishopMasks[64];
extern uint64_t rookMasks[64];

// Precomputed relevant bits for each square
extern uint32_t bishopRelevantBits[64];
extern uint32_t rookRelevantBits[64];

// Magic bitboards' attack tables
extern uint64_t bishopAttacks[5248], * bishopTablePointers[64];
extern uint64_t rookAttacks[102400], * rookTablePointers[64];

// Utility
uint64_t getBishopAttacksSlow(uint32_t square, uint64_t occupancy); // Slow version
uint64_t getRookAttacksSlow(uint32_t square, uint64_t occupancy); // Slow version
uint64_t getQueenAttacksSlow(uint32_t square, uint64_t occupancy); // Bishop | Rooks -> inherently slow
uint64_t getPieceAttacksSlow(uint32_t pieceType, uint32_t square, uint64_t occupancy); // Generalized for every piece type, slow for bishops, rooks, queens

// Initialize attack tables
void initAttackTables();

// Access attack tables
uint64_t getPawnAttacks(uint32_t color, uint32_t square);  // Precomputed array unde the hood
uint64_t getPawnPushes(uint32_t color, uint64_t pawnBitboard, uint64_t occupancies); // Shifts the entire bitboard for computing all pawn's pushes at once
uint64_t getPawnDoublePushes(uint32_t color, uint64_t pawnBitboard, uint64_t occupancies); // 2x getPawnPushes() with a '& RANK_3/6' in between
uint64_t getKnightAttacks(uint32_t square); // Precomputed array unde the hood
uint64_t getBishopAttacks(uint32_t square, uint64_t occupancies); // Fast (magic bitboards)
uint64_t getRookAttacks(uint32_t square, uint64_t occupancies); // Fast (magic bitboards)
uint64_t getQueenAttacks(uint32_t square, uint64_t occupancies); // Bishop | Rook -> inherently fast
uint64_t getKingAttacks(uint32_t square); // Precomputed array unde the hood
uint64_t getPieceAttacks(uint32_t pieceType, uint32_t square, uint64_t occupancy); // Generalized for every piece type

#ifdef __cplusplus
} // extern "C"
#endif

#endif // ATTACKS_H

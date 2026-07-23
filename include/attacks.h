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

// Initialize attack tables
uint64_t getBishopAttacksSlow(uint32_t square, uint64_t occupancy); // Slow version
uint64_t getRookAttacksSlow(uint32_t square, uint64_t occupancy); // Slow version
uint64_t getQueenAttacksSlow(uint32_t square, uint64_t occupancy); // Bishop | Rooks -> inherently slow
uint64_t getPieceAttacksSlow(uint32_t pieceType, uint32_t square, uint64_t occupancy); // Generalized for every piece type, slow for bishops, rooks, queens
void initAttackTables();

// Access attack tables
uint64_t getPawnAttacks(uint32_t color, uint32_t square);
uint64_t getKnightAttacks(uint32_t square);
uint64_t getBishopAttacks(uint32_t square, uint64_t occupancies);
uint64_t getRookAttacks(uint32_t square, uint64_t occupancies);
uint64_t getQueenAttacks(uint32_t square, uint64_t occupancies);
uint64_t getKingAttacks(uint32_t square);
uint64_t getPieceAttacks(uint32_t pieceType, uint32_t square, uint64_t occupancy); // Generalized for every piece type

#ifdef __cplusplus
}
#endif

#endif // ATTACKS_H

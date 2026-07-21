#ifndef ATTACKS_H
#define ATTACKS_H

// Std includes
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Precomputed attack bitboards
extern uint64_t pawnAttacks[2][64];
extern uint64_t knightAttacks[64];
extern uint64_t kingAttacks[64];

// Initialize attack tables
void initAttackTables();

#ifdef __cplusplus
}
#endif

#endif // ATTACKS_H

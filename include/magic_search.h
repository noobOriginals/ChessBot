#ifndef MAGIC_SEARCH_H
#define MAGIC_SEARCH_H

// Std includes
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern uint64_t bishopAttackTable[64][512];
extern uint64_t rookAttackTable[64][4096];
extern uint64_t bishopMagics[64];
extern uint64_t rookMagics[64];

uint64_t nextInt64();
uint64_t randomMagic();
uint32_t tryMagic(uint64_t magic, uint64_t* table, uint64_t* occupancies, uint64_t* attacks, uint32_t count, uint32_t bits);
uint64_t findMagic(uint64_t* table, uint64_t* occupancies, uint64_t* attacks, uint32_t count, uint32_t bits);
void magicSearch();
void optimizedMagicSearch();

#ifdef __cplusplus
}
#endif

#endif // MAGIC_SEARCH_H

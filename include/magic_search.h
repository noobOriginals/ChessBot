#ifndef MAGIC_SEARCH_H
#define MAGIC_SEARCH_H

// Std includes
#include <stdint.h>

// Local includes
#include "pcg32.h"

#ifdef __cplusplus
extern "C" {
#endif

uint64_t findBishopMagic(uint32_t square, PCG32* rng);
uint64_t findRookMagic(uint32_t square, PCG32* rng);
void bishopMagicSearch();
void rookMagicSearch();

#ifdef __cplusplus
} // extern "C"
#endif

#endif // MAGIC_SEARCH_H

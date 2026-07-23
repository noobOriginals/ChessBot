#ifndef MAGIC_SEARCH_H
#define MAGIC_SEARCH_H

// Std includes
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint64_t findBishopMagic(uint32_t square);
uint64_t findRookMagic(uint32_t square);
void bishopMagicSearch();
void rookMagicSearch();

#ifdef __cplusplus
}
#endif

#endif // MAGIC_SEARCH_H

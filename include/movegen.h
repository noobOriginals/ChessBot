#ifndef MOVEGEN_H
#define MOVEGEN_H

// Std includes
#include <stdint.h>

// Local includes
#include "board.h"

#ifdef __cplusplus
extern "C" {
#endif

uint64_t getCheckers(Board* board);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // MOVEGEN_H

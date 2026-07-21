#include "attacks.h"

// Local includes
#include "board.h"

// Precomputed attack bitboads
uint64_t pawnAttacks[2][64];
uint64_t knightAttacks[64];
uint64_t kingAttacks[64];

// Initialize attack tables
void initAttackTables() {
    {
        // Pawn attack tables
        int32_t rankOffsets[2] = { 1,  1};
        int32_t fileOffsets[2] = { 1, -1};

        for (uint32_t rank = 0; rank < 8; rank += 1) {
            for (uint32_t file = 0; file < 8; file += 1) {
                uint32_t square = rank * 8 + file;
                for (uint32_t i = 0; i < 2; i++) {
                    int32_t offsetedRank = (int32_t) rank + rankOffsets[i];
                    int32_t offsetedFile = (int32_t) file + fileOffsets[i];
                    if (offsetedRank > 7 || offsetedRank < 0 || offsetedFile > 7 || offsetedFile < 0) continue;
                    pawnAttacks[WHITE][square] |= (1ull << (offsetedRank * 8 + offsetedFile));
                }
                for (uint32_t i = 0; i < 2; i++) {
                    int32_t offsetedRank = (int32_t) rank - rankOffsets[i];
                    int32_t offsetedFile = (int32_t) file - fileOffsets[i];
                    if (offsetedRank > 7 || offsetedRank < 0 || offsetedFile > 7 || offsetedFile < 0) continue;
                    pawnAttacks[BLACK][square] |= (1ull << (offsetedRank * 8 + offsetedFile));
                }
            }
        }
    }
    {
        // Knight attack tables
        int32_t rankOffsets[8] = { 2,  2,  1,  1, -2, -2, -1, -1};
        int32_t fileOffsets[8] = { 1, -1,  2, -2,  1, -1,  2, -2};

        for (uint32_t rank = 0; rank < 8; rank += 1) {
            for (uint32_t file = 0; file < 8; file += 1) {
                uint32_t square = rank * 8 + file;
                for (uint32_t i = 0; i < 8; i++) {
                    int32_t offsetedRank = (int32_t) rank + rankOffsets[i];
                    int32_t offsetedFile = (int32_t) file + fileOffsets[i];
                    if (offsetedRank > 7 || offsetedRank < 0 || offsetedFile > 7 || offsetedFile < 0) continue;
                    knightAttacks[square] |= (1ull << (offsetedRank * 8 + offsetedFile));
                }
            }
        }
    }
    {
        // King attack tables
        int32_t rankOffsets[8] = { 1,  1,  1,  0,  0, -1, -1, -1};
        int32_t fileOffsets[8] = { 0,  1, -1,  1, -1,  0,  1, -1};

        for (uint32_t rank = 0; rank < 8; rank += 1) {
            for (uint32_t file = 0; file < 8; file += 1) {
                uint32_t square = rank * 8 + file;
                for (uint32_t i = 0; i < 8; i++) {
                    int32_t offsetedRank = (int32_t) rank + rankOffsets[i];
                    int32_t offsetedFile = (int32_t) file + fileOffsets[i];
                    if (offsetedRank > 7 || offsetedRank < 0 || offsetedFile > 7 || offsetedFile < 0) continue;
                    kingAttacks[square] |= (1ull << (offsetedRank * 8 + offsetedFile));
                }
            }
        }
    }
}

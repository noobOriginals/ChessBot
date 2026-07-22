#include "attacks.h"

// Local includes
#include "board.h"

// Precomputed attack bitboards
uint64_t pawnAttacks[2][64];
uint64_t knightAttacks[64];
uint64_t kingAttacks[64];

// Precomputed relevant occupancy masks
uint64_t bishopMasks[64];
uint64_t rookMasks[64];

// Precomputed relevant bits for each square
uint32_t bishopRelevantBits[64];
uint32_t rookRelevantBits[64];

// Utility
static uint64_t getSlidingAttacks(uint32_t square, uint64_t occupancy, const int32_t* rankDirs, const int32_t* fileDirs, const uint32_t numDirs) {
    uint64_t attacks = 0;
    for (uint32_t d = 0; d < numDirs; d += 1) {
        int32_t rank = square / 8, file = square % 8;
        for (;;) {
            rank += rankDirs[d];
            file += fileDirs[d];
            if (rank > 7 || rank < 0 || file > 7 || file < 0) break;
            uint32_t reached = (uint32_t) (rank * 8 + file);
            attacks |= (1ull << reached);
            if (occupancy & (1ull << reached)) break;
        }
    }
    return attacks;
}
static void initOccupancyMasks() {
    // Rook individual ray dirs
    int32_t vertRankDirs[2] = { 1, -1};
    int32_t vertFileDirs[2] = { 0,  0};
    int32_t horzRankDirs[2] = { 0,  0};
    int32_t horzFileDirs[2] = { 1, -1};

    // Compute mask for each square
    for (uint32_t sq = 0; sq < 64; sq += 1) {
        bishopMasks[sq] = getBishopAttacks(sq, 0) & ~(FILE_A | FILE_H | RANK_1 | RANK_8);
        rookMasks[sq] = (getSlidingAttacks(sq, 0, vertRankDirs, vertFileDirs, 2) & ~(RANK_1 | RANK_8)) |
                        (getSlidingAttacks(sq, 0, horzRankDirs, horzFileDirs, 2) & ~(FILE_A | FILE_H));

        // Count number of relevant bits
        bishopRelevantBits[sq] = (uint32_t) popcount64(bishopMasks[sq]);
        rookRelevantBits[sq] = (uint32_t) popcount64(rookMasks[sq]);
    }
}

// Initialize attack tables
void initAttackTables() {
    {
        // Pawn attack tables
        int32_t rankOffsets[2] = { 1,  1};
        int32_t fileOffsets[2] = { 1, -1};

        for (uint32_t rank = 0; rank < 8; rank += 1) {
            for (uint32_t file = 0; file < 8; file += 1) {
                uint32_t square = rank * 8 + file;
                for (uint32_t i = 0; i < 2; i += 1) {
                    int32_t offsetedRank = (int32_t) rank + rankOffsets[i];
                    int32_t offsetedFile = (int32_t) file + fileOffsets[i];
                    if (offsetedRank > 7 || offsetedRank < 0 || offsetedFile > 7 || offsetedFile < 0) continue;
                    pawnAttacks[WHITE][square] |= (1ull << (offsetedRank * 8 + offsetedFile));
                }
                for (uint32_t i = 0; i < 2; i += 1) {
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
                for (uint32_t i = 0; i < 8; i += 1) {
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
                for (uint32_t i = 0; i < 8; i += 1) {
                    int32_t offsetedRank = (int32_t) rank + rankOffsets[i];
                    int32_t offsetedFile = (int32_t) file + fileOffsets[i];
                    if (offsetedRank > 7 || offsetedRank < 0 || offsetedFile > 7 || offsetedFile < 0) continue;
                    kingAttacks[square] |= (1ull << (offsetedRank * 8 + offsetedFile));
                }
            }
        }
    }
    initOccupancyMasks();
}

// Access attack tables
uint64_t getPawnAttacks(uint32_t color, uint32_t square) {
    if (color > BLACK || square > 63) return 0;
    return pawnAttacks[color][square];
}

uint64_t getKnightAttacks(uint32_t square) {
    if (square > 63) return 0;
    return knightAttacks[square];
}

uint64_t getBishopAttacks(uint32_t square, uint64_t occupancy) {
    if (square > 63) return 0;
    int32_t rankDirs[4] = { 1, -1,  1, -1};
    int32_t fileDirs[4] = { 1, -1, -1,  1};
    return getSlidingAttacks(square, occupancy, rankDirs, fileDirs, 4);
}

uint64_t getRookAttacks(uint32_t square, uint64_t occupancy) {
    if (square > 63) return 0;
    int32_t rankDirs[4] = { 1, -1,  0,  0};
    int32_t fileDirs[4] = { 0,  0,  1, -1};
    return getSlidingAttacks(square, occupancy, rankDirs, fileDirs, 4);
}

uint64_t getQueenAttacks(uint32_t square, uint64_t occupancy) {
    if (square > 63) return 0;
    return getBishopAttacks(square, occupancy) | getRookAttacks(square, occupancy);
}

uint64_t getKingAttacks(uint32_t square) {
    if (square > 63) return 0;
    return kingAttacks[square];
}

uint64_t getPieceAttacks(uint32_t pieceType, uint32_t square, uint64_t occupancy) {
    if (pieceType >= TOTAL_PIECE_TYPES || square > 63) return 0;
    uint32_t color = WHITE;
    if (pieceType >= BLACK_OFFSET) {
        color = BLACK;
        pieceType -= BLACK_OFFSET;
    }
    switch (pieceType) {
        case PAWN: return getPawnAttacks(color, square);
        case KNIGHT: return getKnightAttacks(square);
        case BISHOP: return getBishopAttacks(square, occupancy);
        case ROOK: return getRookAttacks(square, occupancy);
        case QUEEN: return getQueenAttacks(square, occupancy);
        case KING: return getKingAttacks(square);
        default: return 0;
    }
}

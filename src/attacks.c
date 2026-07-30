#include "attacks.h"

// Std includes
#include <stdio.h>
#include <stdlib.h>

// Local includes
#include "board.h"

// Debug-only (faster in release mode)
#if !defined(NDEBUG)
    #include <assert.h>
    #define ASSERT(cond, msg) if (!(cond)) { fprintf(stderr, msg); assert(cond); exit(1); }
#else
    #define ASSERT(cond, msg)
#endif

// Constants
const uint64_t bishopMagics[64] = {0xa0020421040874ull, 0x204090424008082ull, 0x30031061000080ull, 0x80a0021000000ull, 0x181104100584880ull, 0x2412060291301ull, 0x2205040114418900ull, 0x400220210044200ull, 0x80020041061c100ull, 0x2000460204212200ull, 0x480080800ca8008ull, 0x4a180a00220000ull, 0x420120610020000ull, 0xc1009010480400ull, 0x4111004210046240ull, 0x200240e080400ull, 0x8002020044880ull, 0x802042014011a00ull, 0x1064010808109010ull, 0xc10848802004000ull, 0x2822c00a00100ull, 0x24080202840400ull, 0x1040048521018ull, 0x688006a080202ull, 0x2010402048020409ull, 0x8050408100480ull, 0x8020044002200ull, 0x4002104020880ull, 0x20088c0010802002ull, 0x1030090004808a84ull, 0x2004612211002ull, 0x9420340002c0202ull, 0x23190860411000ull, 0x220208a003060a05ull, 0x4041200190200ull, 0x64601100280084ull, 0x4002020080a080ull, 0x114a8020140c111ull, 0x488010c21810080ull, 0x25004100048420ull, 0x8c3048804001ull, 0x2c03080110240d00ull, 0x40c0044000800ull, 0x404060522004400ull, 0x3000482104020040ull, 0x1222009020800101ull, 0x1010030144042101ull, 0x752441404800020ull, 0x101008611400108ull, 0x200c308480402ull, 0x408004404041005ull, 0x100108480600ull, 0x48001102020002ull, 0x1021111710110000ull, 0xc12200101021002ull, 0x12080808808802ull, 0x228208050c0200ull, 0x200486c048143020ull, 0x30a080200420806ull, 0x400100804420202ull, 0x2420000220024400ull, 0x100012204100081ull, 0x440010a001010208ull, 0x88b00148010010ull};
const uint64_t rookMagics[64] = {0x10010800520c100ull, 0x244000401000a000ull, 0x2100110020004008ull, 0x4080100028025480ull, 0x280072c00800800ull, 0x2000a0010080124ull, 0x10000840a001100ull, 0x1100042100028a42ull, 0x80800220400082ull, 0x2004400241201002ull, 0x84808050002000ull, 0x102004010204a00ull, 0x205000800050052ull, 0xc2800200808400ull, 0x20010002000c0100ull, 0x2001000845000092ull, 0x340018000406282ull, 0x20004000500021ull, 0x6100808030002000ull, 0x1200808008001000ull, 0x2400808048002c01ull, 0x884008002002480ull, 0x1442840010080122ull, 0x4a0120001006884ull, 0x85014100248000ull, 0x90005240052000ull, 0x1300220200144080ull, 0xa101001000d8ull, 0x2a00280100250010ull, 0x100a000200040890ull, 0x484c00228110ull, 0x100408200040159ull, 0x280002000400644ull, 0x420201000400040ull, 0xa10200043001100ull, 0x402801802801000ull, 0x1800802800806c00ull, 0x1080203018014004ull, 0x425811100c000802ull, 0x2000004402000085ull, 0x2002288040008000ull, 0x10044020004000ull, 0x2001200300310040ull, 0x8008010028028ull, 0x428040801010010ull, 0x81001224010008ull, 0x12010210040008ull, 0x20042240820003ull, 0x2800420020870600ull, 0x100520060810600ull, 0x44c01420010100ull, 0x410010020a900ull, 0x440008018080ull, 0x404000e00148080ull, 0x80080a10010400ull, 0x402086304008a00ull, 0x1000800500a04011ull, 0x2008804000221901ull, 0x11015420004009ull, 0x201000180501ull, 0x1901005044080003ull, 0x4045004802240001ull, 0x4018049028020124ull, 0xc0002a400804102ull};

// Precomputed attack bitboards
uint64_t pawnAttacks[128];
uint64_t knightAttacks[64];
uint64_t kingAttacks[64];

// Precomputed relevant occupancy masks
uint64_t bishopMasks[64];
uint64_t rookMasks[64];

// Precomputed relevant bits for each square
uint32_t bishopRelevantBits[64];
uint32_t rookRelevantBits[64];

// Magic bitboards' attack tables
uint64_t bishopAttacks[5248], * bishopTablePointers[64];
uint64_t rookAttacks[102400], * rookTablePointers[64];

// Utility
uint64_t getSlidingAttacks(uint32_t square, uint64_t occupancy, const int32_t* rankDirs, const int32_t* fileDirs, const uint32_t numDirs) {
    ASSERT(square < 64, "getSlidingAttacks() failed: Invalid square index.\n");
    uint64_t attacks = 0;
    for (uint32_t d = 0; d < numDirs; d += 1) {
        int32_t rank = square >> 3, file = square & 7;
        for (;;) {
            rank += rankDirs[d];
            file += fileDirs[d];
            if (rank > 7 || rank < 0 || file > 7 || file < 0) break;
            uint32_t reached = (uint32_t) ((rank << 3) + file);
            attacks |= (1ull << reached);
            if (occupancy & (1ull << reached)) break;
        }
    }
    return attacks;
}

uint64_t getBishopAttacksSlow(uint32_t square, uint64_t occupancy) {
    ASSERT(square < 64, "getBishopAttacksSlow() failed: Invalid square index!\n");
    int32_t rankDirs[4] = { 1, -1,  1, -1};
    int32_t fileDirs[4] = { 1, -1, -1,  1};
    return getSlidingAttacks(square, occupancy, rankDirs, fileDirs, 4);
}

uint64_t getRookAttacksSlow(uint32_t square, uint64_t occupancy) {
    ASSERT(square < 64, "getRookAttacksSlow() failed: Invalid square index!\n");
    int32_t rankDirs[4] = { 1, -1,  0,  0};
    int32_t fileDirs[4] = { 0,  0,  1, -1};
    return getSlidingAttacks(square, occupancy, rankDirs, fileDirs, 4);
}

uint64_t getQueenAttacksSlow(uint32_t square, uint64_t occupancy) {
    ASSERT(square < 64, "getQueenAttacksSlow() failed: Invalid square index!\n");
    return getBishopAttacksSlow(square, occupancy) | getRookAttacksSlow(square, occupancy);
}

uint64_t getPieceAttacksSlow(uint32_t pieceType, uint32_t square, uint64_t occupancy) {
    ASSERT(pieceType < TOTAL_PIECE_TYPES, "getPieceAttacksSlow() failed: Invalid piece type!\n");
    ASSERT(square < 64, "getPieceAttacksSlow() failed: Invalid square index!\n");
    uint32_t color = WHITE;
    if (pieceType >= BLACK_OFFSET) {
        color = BLACK;
        pieceType -= BLACK_OFFSET;
    }
    switch (pieceType) {
        case PAWN: return getPawnAttacks(color, square);
        case KNIGHT: return getKnightAttacks(square);
        case BISHOP: return getBishopAttacksSlow(square, occupancy);
        case ROOK: return getRookAttacksSlow(square, occupancy);
        case QUEEN: return getQueenAttacksSlow(square, occupancy);
        case KING: return getKingAttacks(square);
        default: return 0;
    }
}

static void initOccupancyMasks() {
    // Rook individual ray dirs
    int32_t vertRankDirs[2] = { 1, -1};
    int32_t vertFileDirs[2] = { 0,  0};
    int32_t horzRankDirs[2] = { 0,  0};
    int32_t horzFileDirs[2] = { 1, -1};

    // Compute mask for each square
    for (uint32_t sq = 0; sq < 64; sq += 1) {
        bishopMasks[sq] = getBishopAttacksSlow(sq, 0) & ~(FILE_A | FILE_H | RANK_1 | RANK_8);
        rookMasks[sq] = (getSlidingAttacks(sq, 0, vertRankDirs, vertFileDirs, 2) & ~(RANK_1 | RANK_8)) |
                        (getSlidingAttacks(sq, 0, horzRankDirs, horzFileDirs, 2) & ~(FILE_A | FILE_H));

        // Count number of relevant bits
        bishopRelevantBits[sq] = (uint32_t) popcountll(bishopMasks[sq]);
        rookRelevantBits[sq] = (uint32_t) popcountll(rookMasks[sq]);
    }
}

// Initialize attack tables
void initAttackTables() {
    initOccupancyMasks();
    {
        // Pawn attack tables
        int32_t rankOffsets[2] = { 1,  1};
        int32_t fileOffsets[2] = { 1, -1};

        for (uint32_t rank = 0; rank < 8; rank += 1) {
            for (uint32_t file = 0; file < 8; file += 1) {
                uint32_t square = (rank << 3) + file;
                for (uint32_t i = 0; i < 2; i += 1) {
                    int32_t offsetedRank = (int32_t) rank + rankOffsets[i];
                    int32_t offsetedFile = (int32_t) file + fileOffsets[i];
                    if (offsetedRank > 7 || offsetedRank < 0 || offsetedFile > 7 || offsetedFile < 0) continue;
                    pawnAttacks[square] |= (1ull << ((offsetedRank << 3) + offsetedFile));
                }
                for (uint32_t i = 0; i < 2; i += 1) {
                    int32_t offsetedRank = (int32_t) rank - rankOffsets[i];
                    int32_t offsetedFile = (int32_t) file - fileOffsets[i];
                    if (offsetedRank > 7 || offsetedRank < 0 || offsetedFile > 7 || offsetedFile < 0) continue;
                    pawnAttacks[square + 64] |= (1ull << ((offsetedRank << 3) + offsetedFile));
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
                uint32_t square = (rank << 3) + file;
                for (uint32_t i = 0; i < 8; i += 1) {
                    int32_t offsetedRank = (int32_t) rank + rankOffsets[i];
                    int32_t offsetedFile = (int32_t) file + fileOffsets[i];
                    if (offsetedRank > 7 || offsetedRank < 0 || offsetedFile > 7 || offsetedFile < 0) continue;
                    knightAttacks[square] |= (1ull << ((offsetedRank << 3) + offsetedFile));
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
                uint32_t square = (rank << 3) + file;
                for (uint32_t i = 0; i < 8; i += 1) {
                    int32_t offsetedRank = (int32_t) rank + rankOffsets[i];
                    int32_t offsetedFile = (int32_t) file + fileOffsets[i];
                    if (offsetedRank > 7 || offsetedRank < 0 || offsetedFile > 7 || offsetedFile < 0) continue;
                    kingAttacks[square] |= (1ull << ((offsetedRank << 3) + offsetedFile));
                }
            }
        }
    }
    {
        // Bishop attack table
        uint32_t tableIndex = 0;
        for (uint32_t square = 0; square < 64; square += 1) {
            bishopTablePointers[square] = &bishopAttacks[tableIndex];
            uint64_t mask = bishopMasks[square], occ[512] = {}, att[512] = {};
            uint64_t subset = 0;
            occ[0] = subset;
            att[0] = getBishopAttacksSlow(square, subset);
            uint32_t size = 1, bits = bishopRelevantBits[square];
            while (subset != mask) {
                subset = (subset - mask) & mask;
                occ[size] = subset;
                att[size] = getBishopAttacksSlow(square, subset);
                size += 1;
            }
            if (size != (1ull << bits)) {
                fprintf(stderr, "FATAL: bishop table build mismatch at square %u\n", square);
                exit(1);
            }

            uint32_t maxIdx = 0;
            for (uint32_t i = 0; i < size; i += 1) {
                uint32_t index = (uint32_t) ((bishopMagics[square] * occ[i]) >> (64 - bits));
                if (index + 1 > maxIdx) maxIdx = index + 1;
                bishopTablePointers[square][index] = att[i];
            }
            tableIndex += maxIdx;
        }
    }
    {
        // Rook attack table
        uint32_t tableIndex = 0;
        for (uint32_t square = 0; square < 64; square += 1) {
            rookTablePointers[square] = &rookAttacks[tableIndex];
            uint64_t mask = rookMasks[square], occ[4096] = {}, att[4096] = {};
            uint64_t subset = 0;
            occ[0] = subset;
            att[0] = getRookAttacksSlow(square, subset);
            uint32_t size = 1, bits = rookRelevantBits[square];
            while (subset != mask) {
                subset = (subset - mask) & mask;
                occ[size] = subset;
                att[size] = getRookAttacksSlow(square, subset);
                size += 1;
            }
            if (size != (1ull << bits)) {
                fprintf(stderr, "FATAL: rook table build mismatch at square %u\n", square);
                exit(1);
            }

            uint32_t maxIdx = 0;
            for (uint32_t i = 0; i < size; i += 1) {
                uint32_t index = (uint32_t) ((rookMagics[square] * occ[i]) >> (64 - bits));
                if (index + 1 > maxIdx) maxIdx = index + 1;
                rookTablePointers[square][index] = att[i];
            }
            tableIndex += maxIdx;
        }
    }
}

// Access attack tables
uint64_t getPawnAttacks(uint32_t color, uint32_t square) {
    ASSERT(color == WHITE || color == BLACK, "getPawnAttacks() failed: Invalid color!");
    ASSERT(square < 64, "getPawnAttacks() failed: Invalid square index!");
    return pawnAttacks[square + color * 64];
}

uint64_t getPawnPushes(uint32_t color, uint64_t pawnBitboard, uint64_t occupancies) {
    ASSERT(color == WHITE || color == BLACK, "getPawnPushes() failed: Invalid color!");
    switch (color) {
        case WHITE: return (pawnBitboard << 8) & ~occupancies;
        case BLACK: return (pawnBitboard >> 8) & ~occupancies;
        default: return 0;
    }
}

uint64_t getPawnDoublePushes(uint32_t color, uint64_t pawnBitboard, uint64_t occupancies) {
    ASSERT(color == WHITE || color == BLACK, "getPawnPushes() failed: Invalid color!");
    return getPawnPushes(color, getPawnPushes(color, pawnBitboard, occupancies) & (color ? RANK_6 : RANK_3), occupancies);
}

uint64_t getKnightAttacks(uint32_t square) {
    ASSERT(square < 64, "getKnightAttacks() failed: Invalid square index!");
    return knightAttacks[square];
}

uint64_t getBishopAttacks(uint32_t square, uint64_t occupancies) {
    ASSERT(square < 64, "getBishopAttacks() failed: Invalid square index!");
    occupancies &= bishopMasks[square];
    occupancies *= bishopMagics[square];
    occupancies >>= (64 - bishopRelevantBits[square]);
    return bishopTablePointers[square][occupancies];
}

uint64_t getRookAttacks(uint32_t square, uint64_t occupancies) {
    ASSERT(square < 64, "getRookAttacks() failed: Invalid square index!");
    occupancies &= rookMasks[square];
    occupancies *= rookMagics[square];
    occupancies >>= (64 - rookRelevantBits[square]);
    return rookTablePointers[square][occupancies];
}

uint64_t getQueenAttacks(uint32_t square, uint64_t occupancies) {
    ASSERT(square < 64, "getQueenAttacks() failed: Invalid square index!");
    return getBishopAttacks(square, occupancies) | getRookAttacks(square, occupancies);
}

uint64_t getKingAttacks(uint32_t square) {
    ASSERT(square < 64, "getKingAttacks() failed: Invalid square index!");
    return kingAttacks[square];
}

uint64_t getPieceAttacks(uint32_t pieceType, uint32_t square, uint64_t occupancy) {
    ASSERT(pieceType < TOTAL_PIECE_TYPES, "getPieceAttacks() failed: Invalid piece type!\n");
    ASSERT(square < 64, "getPieceAttacks() failed: Invalid square index!\n");
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

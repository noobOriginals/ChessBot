#include "attacks.h"

#include "board.h"
#include "debug_utils.h"

// Constants for initializing and using magic bitboards.
// These magics numbers and their coresponding table sizes were found with
// a simple search that I wrote myself, and they are public right here:
// https://github.com/noobOriginals/Magicbit/blob/main/search_runs/concat.txt
static const u64 bishopMagic[64] = {0x821518401104018ull, 0x100328c320b06014ull, 0x8885c0e8800000ull, 0x22aa0160100100ull, 0x40404e103915000ull, 0x120a253a8400408ull, 0x400514a0a130200ull, 0x6c0d1210820803c4ull, 0x20000a0265050138ull, 0x42203065460b060ull, 0x4000418300c05040ull, 0x44a022aa01600101ull, 0x18441044020000ull, 0x507000a254270084ull, 0x20100090aa0a1300ull, 0x680030928a0984ull, 0xc238000606862140ull, 0x401c000d018140c4ull, 0x2010001800d82c18ull, 0x200800022a004000ull, 0x4000202130200ull, 0x200e04071424000ull, 0x803809863030508ull, 0x1c10831818280ull, 0x20200099954b04ull, 0x64820120d028180ull, 0x210c040008880010ull, 0x6c90a00814010020ull, 0x4403001019004002ull, 0x1190002004100ull, 0x81040100004890e6ull, 0x2040280482a34c1ull, 0x1803051a80401000ull, 0xc004c8c280200402ull, 0x4060100080240ull, 0x10c008a0160200ull, 0x3b1c0540103c0100ull, 0x382124d00020081ull, 0x28185a0a60820ull, 0x180a0d0120203ull, 0x801828c0641b801ull, 0x401818283081c00ull, 0x14a61430d0032800ull, 0x984100c010440204ull, 0x180098500b00400ull, 0x3820200d001a0ull, 0x100c02e180611380ull, 0x2430149803041c0ull, 0x689426a05050300ull, 0x8028a28261450800ull, 0x401020c0c14180a0ull, 0x91084044200ull, 0x410109408e39100ull, 0x204027142240a010ull, 0x213144511405280ull, 0x1089850500942814ull, 0x800018820111a100ull, 0x190014142624300ull, 0x82a0028061814188ull, 0x2041201c00a08840ull, 0x10009809408e390ull, 0x40058007844621c0ull, 0x4800130a2a112060ull, 0xd1404c286003024ull};
static const u64 rookMagic[64] = {0x1080002240048018ull, 0x2440004020001000ull, 0x82000c884040820ull, 0x4420080402020421ull, 0x1020192202010020ull, 0x3200300200010428ull, 0x9002500408c0600ull, 0x6e00048022044405ull, 0x109800088204001ull, 0x122402010102040ull, 0x2820016004c8056ull, 0x406000a0046204cull, 0x2001212020408ull, 0x2001d05060010ull, 0x2204c80810204ull, 0x860010a0646482ull, 0x2182021008082010ull, 0x80a2010102040ull, 0xa0848010026000ull, 0x1204290021003000ull, 0x9010034102800ull, 0x108080120405004ull, 0x12000400014948d0ull, 0x80002001425c081ull, 0xa0808010c000ull, 0x1300950100204000ull, 0x4010011680200480ull, 0x10006100150008ull, 0x4000080080800400ull, 0x2c2001200100408ull, 0xc01420400280110ull, 0x204200058407ull, 0x100400080800033ull, 0x100020044002c0ull, 0xa0a0021002802184ull, 0x10080480801000ull, 0x840080802800ull, 0x202001112000408ull, 0x4051004008268ull, 0x201900c2001084ull, 0x28400080248000ull, 0x2020101034c000ull, 0x409282020020ull, 0x2008100063010008ull, 0x40202040a0a0020ull, 0x80002a0004008080ull, 0x80011850a1040002ull, 0x80084040a1020014ull, 0x400420030b09200ull, 0x400030b09300ull, 0x20002a00306b1600ull, 0x20002600290c9200ull, 0x802020408121200ull, 0x2200040003030500ull, 0x50c8c941500400ull, 0x2008404030801840ull, 0x1990052004c2416ull, 0x310021001a18822aull, 0x8002002680280a12ull, 0xc400a001806ull, 0x40c8000300040b29ull, 0x80200b004080102ull, 0x640018810016344ull, 0x13a0218041040722ull};

// Values for using magic numbers, initiazied once at startup via initAttacks()
static u8 bishopShift[64];
static u8 rookShift[64];
static Bitboard bishopMask[64];
static Bitboard rookMask[64];

// Attack tables
static Bitboard pawnAttacks[128];
static Bitboard knightAttacks[64];
static Bitboard bishopAttacks[5009], * bishopPtr[64];
static Bitboard rookAttacks[102359], * rookPtr[64];
static Bitboard kingAttacks[64];

static void initPawnAttacks(void) {
    for (u8 sq = 0; sq < 64; sq++) {
        pawnAttacks[(sq << 1) | (u8) WHITE] = getAttacksAt(sq, -1,  1) | getAttacksAt(sq,  1,  1);
        pawnAttacks[(sq << 1) | (u8) BLACK] = getAttacksAt(sq, -1, -1) | getAttacksAt(sq,  1, -1);
    }
}

static void initKnightAttacks(void) {
    for (u8 sq = 0; sq < 64; sq++) {
        knightAttacks[sq] =
            getAttacksAt(sq,  1,  2) |
            getAttacksAt(sq,  2,  1) |
            getAttacksAt(sq,  2, -1) |
            getAttacksAt(sq,  1, -2) |
            getAttacksAt(sq, -1, -2) |
            getAttacksAt(sq, -2, -1) |
            getAttacksAt(sq, -2,  1) |
            getAttacksAt(sq, -1,  2);
    }
}

static void initBishopAttacks(void) {
    Bitboard* lastPtr = bishopAttacks;
    for (u8 sq = 0; sq < 64; sq++) {
        // Compute relevant occupancy masks and bits
        bishopMask[sq] = getBishopAttacksSlow(sq, 0ull) & ~(RANK_1 | RANK_8 | FILE_A | FILE_H);
        bishopShift[sq] = (u8) (64 - popcountll(bishopMask[sq]));

        // Init all possible attacks
        u64 index, lastIndex = 0;
        Bitboard subset = 0ull;
        do {
            index = (bishopMagic[sq] * subset) >> bishopShift[sq];
            lastPtr[index] = getBishopAttacksSlow(sq, subset);
            if (index > lastIndex) lastIndex = index;
            subset = (subset - bishopMask[sq]) & bishopMask[sq];
        } while (subset != 0ull);

        // Save first index pointer and prepare for next square
        bishopPtr[sq] = lastPtr;
        lastPtr = &lastPtr[lastIndex + 1];
    }
}

static void initRookAttacks(void) {
    Bitboard* lastPtr = rookAttacks;
    for (u8 sq = 0; sq < 64; sq++) {
        // Compute relevant occupancy masks and bits
        rookMask[sq] =
            ((getSlidingAttacks(sq, 0ull, 0, 1) | getSlidingAttacks(sq, 0ull, 0, -1)) & ~(RANK_1 | RANK_8)) |
            ((getSlidingAttacks(sq, 0ull, 1, 0) | getSlidingAttacks(sq, 0ull, -1, 0)) & ~(FILE_A | FILE_H));
        rookShift[sq] = (u8) (64 - popcountll(rookMask[sq]));

        // Init all possible attacks
        u64 index, lastIndex = 0;
        Bitboard subset = 0ull;
        do {
            index = (rookMagic[sq] * subset) >> rookShift[sq];
            lastPtr[index] = getRookAttacksSlow(sq, subset);
            if (index > lastIndex) lastIndex = index;
            subset = (subset - rookMask[sq]) & rookMask[sq];
        } while (subset != 0ull);

        // Save first index pointer and prepare for next square
        rookPtr[sq] = lastPtr;
        lastPtr = &lastPtr[lastIndex + 1];
    }
}

static void initKingAttacks(void) {
    for (u8 sq = 0; sq < 64; sq++) {
        kingAttacks[sq] =
            getAttacksAt(sq,  0,  1) |
            getAttacksAt(sq,  1,  1) |
            getAttacksAt(sq,  1,  0) |
            getAttacksAt(sq,  1, -1) |
            getAttacksAt(sq,  0, -1) |
            getAttacksAt(sq, -1, -1) |
            getAttacksAt(sq, -1,  0) |
            getAttacksAt(sq, -1,  1);
    }
}

// Init attack tables
void initAttacks(void) {
    initPawnAttacks();
    initKnightAttacks();
    initBishopAttacks();
    initRookAttacks();
    initKingAttacks();
}

// For non-sliders
Bitboard getAttacksAt(u8 square, i8 fileOff, i8 rankOffset) {
    Bitboard attacks = 0ull;
    i8 f = (square & 7) + fileOff;
    i8 r = (square >> 3) + rankOffset;
    if (f < 0 || f > 7 || r < 0 || r > 7) return attacks;
    attacks |= bbsq((r << 3) + f);
    return attacks;
}

// 'Ray-Marching' aproach
Bitboard getSlidingAttacks(u8 square, Bitboard occupancy, i8 fileDir, i8 rankDir) {

    PUSH_STACK_TRACE("getSlidingAttacks()");
    ASSERT_MSG(square < 64, "getSlidingAttacks() failed: square index out of bounds");

    Bitboard attacks = 0ull;
    i8 f = (square & 7), r = (square >> 3);
    while (1) {
        f += fileDir;
        r += rankDir;
        if (f < 0 || f > 7 || r < 0 || r > 7) break;
        attacks |= bbsq((r << 3) + f);
        if (attacks & occupancy) break;
    }

    POP_STACK_TRACE();

    return attacks;
}

Bitboard getBishopAttacksSlow(u8 square, Bitboard occupancy) {

    PUSH_STACK_TRACE("getBishopAttacksSlow()");
    ASSERT_MSG(square < 64, "getBishopAttacksSlow() failed: square index out of bounds");
    POP_STACK_TRACE();

    return
        getSlidingAttacks(square, occupancy,  1,  1) |
        getSlidingAttacks(square, occupancy, -1, -1) |
        getSlidingAttacks(square, occupancy,  1, -1) |
        getSlidingAttacks(square, occupancy, -1,  1);
}

Bitboard getRookAttacksSlow(u8 square, Bitboard occupancy) {

    PUSH_STACK_TRACE("getRookAttacksSlow()");
    ASSERT_MSG(square < 64, "getRookAttacksSlow() failed: square index out of bounds");
    POP_STACK_TRACE();

    return
        getSlidingAttacks(square, occupancy,  0,  1) |
        getSlidingAttacks(square, occupancy,  0, -1) |
        getSlidingAttacks(square, occupancy,  1,  0) |
        getSlidingAttacks(square, occupancy, -1,  0);
}

Bitboard getQueenAttacksSlow(u8 square, Bitboard occupancy) {

    PUSH_STACK_TRACE("getQueenAttacksSlow()");
    ASSERT_MSG(square < 64, "getQueenAttacksSlow() failed: square index out of bounds");
    POP_STACK_TRACE();

    return getBishopAttacksSlow(square, occupancy) | getRookAttacksSlow(square, occupancy);
}

Bitboard getPieceAttakcsSlow(u8 piece, u8 square, u8 side, Bitboard occupancy) {

    PUSH_STACK_TRACE("getPieceAttakcsSlow()");
    ASSERT_MSG(piece > 1 && piece < 14, "getPieceAttakcsSlow() failed: piece type out of bounds");
    ASSERT_MSG(square < 64, "getPieceAttakcsSlow() failed: square index out of bounds");
    ASSERT_MSG(side < 2, "getPieceAttakcsSlow() failed: side can only have the value 0 (zero) or 1 (one)");
    POP_STACK_TRACE();

    switch (ptype(piece)) {
        case PAWN: return getPawnAttacks(square, side);
        case KNIGHT: return getKnightAttacks(square);
        case BISHOP: return getBishopAttacksSlow(square, occupancy);
        case ROOK: return getRookAttacksSlow(square, occupancy);
        case QUEEN: return getQueenAttacksSlow(square, occupancy);
        case KING: return getKingAttacks(square);
        default: return 0ull;
    }
}

// Fast, precomputed aproach
Bitboard getPawnAttacks(u8 square, u8 side) {

    PUSH_STACK_TRACE("getPawnAttacks()");
    ASSERT_MSG(square < 64, "getPawnAttacks() failed: square index out of bounds");
    ASSERT_MSG(side < 2, "getPawnAttacks() failed: side can only have the value 0 (zero) or 1 (one)");
    POP_STACK_TRACE();

    return pawnAttacks[(square << 1) | side];
}

Bitboard getKnightAttacks(u8 square) {

    PUSH_STACK_TRACE("getKnightAttacks()");
    ASSERT_MSG(square < 64, "getKnightAttacks() failed: square index out of bounds");
    POP_STACK_TRACE();

    return knightAttacks[square];
}

Bitboard getBishopAttacks(u8 square, Bitboard occupancy) {

    PUSH_STACK_TRACE("getBishopAttacks()");
    ASSERT_MSG(square < 64, "getBishopAttacks() failed: square index out of bounds");

    occupancy &= bishopMask[square];
    occupancy *= bishopMagic[square];
    occupancy >>= bishopShift[square];

    POP_STACK_TRACE();

    return bishopPtr[square][occupancy];
}

Bitboard getRookAttacks(u8 square, Bitboard occupancy) {

    PUSH_STACK_TRACE("getRookAttacks()");
    ASSERT_MSG(square < 64, "getRookAttacks() failed: square index out of bounds");

    occupancy &= rookMask[square];
    occupancy *= rookMagic[square];
    occupancy >>= rookShift[square];

    POP_STACK_TRACE();

    return rookPtr[square][occupancy];
}

Bitboard getQueenAttacks(u8 square, Bitboard occupancy) {

    PUSH_STACK_TRACE("getQueenAttacks()");
    ASSERT_MSG(square < 64, "getQueenAttacks() failed: square index out of bounds");
    POP_STACK_TRACE();

    return getBishopAttacks(square, occupancy) | getRookAttacks(square, occupancy);
}

Bitboard getKingAttacks(u8 square) {

    PUSH_STACK_TRACE("getKingAttacks()");
    ASSERT_MSG(square < 64, "getKingAttacks() failed: square index out of bounds");
    POP_STACK_TRACE();

    return kingAttacks[square];
}

Bitboard getPieceAttakcs(u8 piece, u8 square, u8 side, Bitboard occupancy) {

    PUSH_STACK_TRACE("getPieceAttakcs()");
    ASSERT_MSG(piece > 1 && piece < 14, "getPieceAttakcs() failed: piece type out of bounds");
    ASSERT_MSG(square < 64, "getPieceAttakcs() failed: square index out of bounds");
    ASSERT_MSG(side < 2, "getPieceAttakcs() failed: side can only have the value 0 (zero) or 1 (one)");
    POP_STACK_TRACE();

    switch (ptype(piece)) {
        case PAWN: return getPawnAttacks(square, side);
        case KNIGHT: return getKnightAttacks(square);
        case BISHOP: return getBishopAttacks(square, occupancy);
        case ROOK: return getRookAttacks(square, occupancy);
        case QUEEN: return getQueenAttacks(square, occupancy);
        case KING: return getKingAttacks(square);
        default: return 0ull;
    }
}

// Bulk generation of pawn pushed
Bitboard getPawnPushes(Bitboard bitboard, Bitboard occupancy, u8 side) {

    PUSH_STACK_TRACE("getPawnPushes()");
    ASSERT_MSG(side < 2, "getPawnPushes() failed: side can only have the value 0 (zero) or 1 (one)");
    POP_STACK_TRACE();

    return side ? (bitboard >> 8) & ~occupancy : (bitboard << 8) & ~occupancy;
}

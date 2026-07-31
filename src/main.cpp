#include <iostream>
#include <chrono>
#include <string>

#include "board.h"
#include "attacks.h"
#include "movegen.h"

#define TEST_FEN_1 "r1bqk2r/pppp1ppp/2n2n2/1Bb1p3/4P3/3P1N2/PPP2PPP/RNBQ1RK1 b kq - 0 5"
#define TEST_FEN_2 "r1bqkbnr/ppp2ppp/2np4/4p2Q/2B1P3/8/PPPP1PPP/RNB1K1NR w KQkq - 2 4" // startpos e2e4 e7e5 f1c4 d7d6 d1h5 b8c6 go

#define MAX_DEPTH 255
// #define PERFT_TEST_BOARD_STRUCT
// #define PERFT_USE_MEM_DUMP

void displayBoard(Board* board) {
    char buffer[1024] = {};
    std::cout << getVisualBoardString(board, buffer, 1024);
}

void displayBoardFen(Board* board) {
    char buffer[1024] = {};
    std::cout << getFen(board, buffer, 1024) << "\n";
}

void displayBitboard(uint64_t bitboard) {
    char buffer[1024] = {};
    std::cout << getVisualBitboardString(bitboard, buffer, 1024);
}

int32_t assertPositions(uint64_t actual, uint64_t expected) {
    if (actual != expected) {
        char buffer[1024] = {};
        printf("Position assert failed!\n.Expected: %s\nActual: %s\n", getVisualBitboardString(expected, buffer, 1024), getVisualBitboardString(actual, buffer, 1024));
        return 0;
    }
    return 1;
}

void testMagicBitboards() {
    for (uint32_t sq = 0; sq < 64; sq++) {
        printf("Testing bishop attacks, square %u\n", sq);
        uint64_t mask = bishopMasks[sq];
        uint64_t subset = 0;
        do {
            if (!assertPositions(getBishopAttacks(sq, subset), getBishopAttacksSlow(sq, subset))) return;
            subset = (subset - mask) & mask;
        } while (subset != 0);
    }
    for (uint32_t sq = 0; sq < 64; sq++) {
        printf("Testing rook attacks, square %u\n", sq);
        uint64_t mask = rookMasks[sq];
        uint64_t subset = 0;
        do {
            if (!assertPositions(getRookAttacks(sq, subset), getRookAttacksSlow(sq, subset))) return;
            subset = (subset - mask) & mask;
        } while (subset != 0);
    }
}

void testMagicBitboardsVsSlowVersion() {
    uint32_t iterations = 100000;

    std::cout << "Timing magic bitboards, " << iterations << " iterations\n";
    auto startTime = std::chrono::high_resolution_clock::now();
    for (uint32_t i = 0; i < iterations; i++) {
        for (uint32_t sq = 0; sq < 64; sq++) {
            uint64_t mask = bishopMasks[sq];
            uint64_t subset = 0;
            do {
                getBishopAttacks(sq, subset);
                subset = (subset - mask) & mask;
            } while (subset != 0);
        }
        for (uint32_t sq = 0; sq < 64; sq++) {
            uint64_t mask = rookMasks[sq];
            uint64_t subset = 0;
            do {
                getRookAttacks(sq, subset);
                subset = (subset - mask) & mask;
            } while (subset != 0);
        }
    }
    std::chrono::duration<double, std::milli> elapsed = std::chrono::high_resolution_clock::now() - startTime;
    std::cout << "Took " << elapsed.count() / 1000.0f << " seconds\n";

    std::cout << "Timing magic bitboards, " << iterations << " iterations\n";
    startTime = std::chrono::high_resolution_clock::now();
    for (uint32_t i = 0; i < iterations; i++) {
        for (uint32_t sq = 0; sq < 64; sq++) {
            uint64_t mask = bishopMasks[sq];
            uint64_t subset = 0;
            do {
                getBishopAttacksSlow(sq, subset);
                subset = (subset - mask) & mask;
            } while (subset != 0);
        }
        for (uint32_t sq = 0; sq < 64; sq++) {
            uint64_t mask = rookMasks[sq];
            uint64_t subset = 0;
            do {
                getRookAttacksSlow(sq, subset);
                subset = (subset - mask) & mask;
            } while (subset != 0);
        }
    }
    elapsed = std::chrono::high_resolution_clock::now() - startTime;
    std::cout << "Took " << elapsed.count() / 1000.0f << " seconds\n";
}

void printMoves(Move* moves, uint32_t size) {
    if (!moves) return;
    char buffer[64] = {};
    printf("All legal moves: ");
    for (uint32_t i = 0; i < size - 1; i++) {
        printf("%s, ", getAlgebraicFromMove(moves[i], buffer, 64));
    }
    printf("%s\n", getAlgebraicFromMove(moves[size - 1], buffer, 64));
    printf("Move flags: ");
    for (uint32_t i = 0; i < size - 1; i++) {
        switch (moveFlag(moves[i])) {
            case MOVE_QUIET: printf("MOVE_QUIET, "); break;
            case MOVE_DOUBLE_PUSH: printf("MOVE_DOUBLE_PUSH, "); break;
            case MOVE_CASTLE_K: printf("MOVE_CASTLE_K, "); break;
            case MOVE_CASTLE_Q: printf("MOVE_CASTLE_Q, "); break;
            case MOVE_CAPTURE: printf("MOVE_CAPTURE, "); break;
            case MOVE_EP_CAPTURE: printf("MOVE_EP_CAPTURE, "); break;
            case MOVE_PROMO_N: printf("MOVE_PROMO_N, "); break;
            case MOVE_PROMO_B: printf("MOVE_PROMO_B, "); break;
            case MOVE_PROMO_R: printf("MOVE_PROMO_R, "); break;
            case MOVE_PROMO_Q: printf("MOVE_PROMO_Q, "); break;
            case MOVE_PROMO_CAPTURE_N: printf("MOVE_PROMO_CAPTURE_N, "); break;
            case MOVE_PROMO_CAPTURE_B: printf("MOVE_PROMO_CAPTURE_B, "); break;
            case MOVE_PROMO_CAPTURE_R: printf("MOVE_PROMO_CAPTURE_R, "); break;
            case MOVE_PROMO_CAPTURE_Q: printf("MOVE_PROMO_CAPTURE_Q, "); break;
            default: printf("UNKNOWN, "); break;
        }
    }
    switch (moveFlag(moves[size - 1])) {
        case MOVE_QUIET: printf("MOVE_QUIET"); break;
        case MOVE_DOUBLE_PUSH: printf("MOVE_DOUBLE_PUSH"); break;
        case MOVE_CASTLE_K: printf("MOVE_CASTLE_K"); break;
        case MOVE_CASTLE_Q: printf("MOVE_CASTLE_Q"); break;
        case MOVE_CAPTURE: printf("MOVE_CAPTURE"); break;
        case MOVE_EP_CAPTURE: printf("MOVE_EP_CAPTURE"); break;
        case MOVE_PROMO_N: printf("MOVE_PROMO_N"); break;
        case MOVE_PROMO_B: printf("MOVE_PROMO_B"); break;
        case MOVE_PROMO_R: printf("MOVE_PROMO_R"); break;
        case MOVE_PROMO_Q: printf("MOVE_PROMO_Q"); break;
        case MOVE_PROMO_CAPTURE_N: printf("MOVE_PROMO_CAPTURE_N"); break;
        case MOVE_PROMO_CAPTURE_B: printf("MOVE_PROMO_CAPTURE_B"); break;
        case MOVE_PROMO_CAPTURE_R: printf("MOVE_PROMO_CAPTURE_R"); break;
        case MOVE_PROMO_CAPTURE_Q: printf("MOVE_PROMO_CAPTURE_Q"); break;
        default: printf("UNKNOWN"); break;
    }
    printf("\n");
}

int32_t findMove(Move* moves, uint32_t b, uint32_t e, Move move) {
    for (uint32_t i = b; i <= e; i++) {
        if (moves[i] == move) return i;
    }
    return -1;
}

void minigame() {
    Board* board = createBoard();
    Move moves[1024], legalMoves[MAX_LEGAL_MOVES];
    PrevState prevState[1024];
    uint32_t prevIdx = 0;
    std::string input;
    std::cout << "Enter initial board state: ";
    std::cin >> input;
    if (input == "startpos") setFen(board, STARTPOS_FEN);
    else setFen(board, input.c_str());
    std::cin >> input;
    while (input != "go") {
        moves[prevIdx] = getMoveFromAlgebraic(board, input.c_str());
        makeMove(board, moves[prevIdx], &prevState[prevIdx]);
        prevIdx++;
        std::cin >> input;
    }
    while (true) {
        displayBoard(board);
        displayBoardFen(board);
        uint32_t size = 0;
        generateLegalMoves(board, legalMoves, &size);
        printMoves(legalMoves, size);
        std::cout << "\nEnter move: ";
        std::cin >> input;
        if (input == "quit") break;
        if (input == "print") {
            std::cin >> input;
            for (char c : input) {
                switch (c) {
                    case 'P': displayBitboard(board->pieces[0]); break;
                    case 'N': displayBitboard(board->pieces[1]); break;
                    case 'B': displayBitboard(board->pieces[2]); break;
                    case 'R': displayBitboard(board->pieces[3]); break;
                    case 'Q': displayBitboard(board->pieces[4]); break;
                    case 'K': displayBitboard(board->pieces[5]); break;
                    case 'p': displayBitboard(board->pieces[6]); break;
                    case 'n': displayBitboard(board->pieces[7]); break;
                    case 'b': displayBitboard(board->pieces[8]); break;
                    case 'r': displayBitboard(board->pieces[9]); break;
                    case 'q': displayBitboard(board->pieces[10]); break;
                    case 'k': displayBitboard(board->pieces[11]); break;
                    case 'W': displayBitboard(getPawnPushes(WHITE, board->pieces[0], board->occupancy)); break;
                    case 'w': displayBitboard(getPawnPushes(BLACK, board->pieces[6], board->occupancy)); break;
                    case 'X': displayBitboard(getPawnDoublePushes(WHITE, board->pieces[0], board->occupancy)); break;
                    case 'x': displayBitboard(getPawnDoublePushes(BLACK, board->pieces[6], board->occupancy)); break;
                #if defined(USE_PER_PIECE_BITBOARDS)
                    case 'S': displayBitboard(board->sidePieces[WHITE]); break;
                    case 's': displayBitboard(board->sidePieces[BLACK]); break;
                #endif
                    default: break;
                }
            }
            continue;
        }
        if (input == "undo") {
            if (prevIdx < 1) continue;
            prevIdx--;
            unmakeMove(board, moves[prevIdx], &prevState[prevIdx]);
        } else if (input == "redo") {
            if (moves[prevIdx] == 0) continue;
            makeMove(board, moves[prevIdx], &prevState[prevIdx]);
            prevIdx++;
        } else {
            moves[prevIdx] = getMoveFromAlgebraic(board, input.c_str());
            if (findMove(legalMoves, 0, size - 1, moves[prevIdx]) == -1) {
                printf("Illegal move, try again.\n");
                continue;
            }
            makeMove(board, moves[prevIdx], &prevState[prevIdx]);
            prevIdx++;
        }
    }
    destroyBoard(board);
}

int32_t testBoardStruct(Board* board) {
    uint32_t actualMailbox[64] = {};
    for (uint32_t i = 0; i < 64; i++) actualMailbox[i] = NO_PIECE;
    for (uint32_t bb = 0; bb < TOTAL_PIECE_TYPES; bb++) {
        uint64_t bitboard = board->pieces[bb];
        while (bitboard != 0) {
            uint32_t sq = popLSB(&bitboard);
            actualMailbox[sq] = bb;
        }
    }
    for (uint32_t i = 0; i < 64; i++) {
        if (actualMailbox[i] != board->mailbox[i]) {
            fprintf(stderr, "Mailbox mismatch!\n");
            Board* actual = createDefaultBoard();
            for (uint32_t s = 0; s < 64; s++) actual->mailbox[s] = actualMailbox[s];
            std::cerr << "Expected:\n";
            displayBoard(actual);
            std::cerr << "\nGot:\n";
            displayBoard(board);
            destroyBoard(actual);
            return 1;
        }
    }
    return 0;
}

#if defined(PERFT_USE_MEM_DUMP)
    uint32_t memSize = 0;
    Move memMoves[100];
    Board* memBoard;
    void memDump() {
        std::cout << "\n\n--- DEBUG INFO ---\n";
        printMoves(memMoves, memSize);
        displayBoard(memBoard);
        displayBitboard(memBoard->occupancy);
        displayBitboard(memBoard->pieces[PAWN]);
        displayBitboard(memBoard->pieces[PAWN + BLACK_OFFSET]);
        std::cout << "\n";
        std::cout << "global_getPieceAttacks_pieceType: " << global_getPieceAttacks_pieceType << "\n";
        std::cout << "global_getPieceAttacks_square: " << global_getPieceAttacks_square << "\n";
        std::cout << "global_getPieceAttacks_occupancy:\n";
        displayBitboard(global_getPieceAttacks_occupancy);
        std::cout << "--- DEBUG INFO ---\n\n";
    }
#endif

Move legalMovesBuffer[MAX_DEPTH][MAX_LEGAL_MOVES];

uint64_t perft(Board* board, uint32_t depth) {
    if (depth == 0) return 1;
    uint64_t nodes = 0;
    uint32_t size = 0;
    generateLegalMoves(board, legalMovesBuffer[depth], &size);
    for (uint32_t i = 0; i < size; i++) {
        PrevState state;
        makeMove(board, legalMovesBuffer[depth][i], &state);

    #if defined(PERFT_USE_MEM_DUMP)
        memMoves[memSize - depth] = legalMovesBuffer[depth][i];
        memBoard = board;
    #endif

    #if defined(PERFT_TEST_BOARD_STRUCT)
        if (testBoardStruct(board)) {
            std::cerr << "perft() failed after makeMove() at depth " << depth << ", move index " << memSize - depth << "\n";
            memDump();
            exit(1);
        }
    #endif

        nodes += perft(board, depth - 1);
        unmakeMove(board, legalMovesBuffer[depth][i], &state);

    #if defined(PERFT_TEST_BOARD_STRUCT)
        if (testBoardStruct(board)) {
            std::cerr << "perft() failed after unmakeMove() at depth " << depth << ", move index " << memSize - depth << "\n";
            memDump();
            exit(1);
        }
    #endif
    }
    return nodes;
}

void testPos(const char* fen, uint32_t maxDepth) {
    Board* board = createBoard();
    setFen(board, fen);
    for (uint32_t depth = 0; depth <= maxDepth; depth++) {
    #if defined(PERFT_USE_MEM_DUMP)
        memSize = depth;
    #endif
        auto start = std::chrono::high_resolution_clock::now();
        uint64_t nodes = perft(board, depth);
        auto end = std::chrono::high_resolution_clock::now();
        double milli = std::chrono::duration<double, std::milli>(end - start).count();
        std::cout << "Position: " << fen << "\n";
        std::cout << "Depth:   " << depth << "\n";
        std::cout << "Nodes:   " << nodes << "\n";
        std::cout << "Time:    " << milli << " ms\n";
        std::cout << "NPS:     " << (uint64_t) (nodes / (milli / 1000)) << " nodes/s\n\n";
    }
    destroyBoard(board);
}

int main() {
    initAttackTables();
    initRayTable();
#if defined(PERFT_USE_MEM_DUMP)
    assertHandle = memDump;
#endif
    testPos(STARTPOS_FEN, 7);
    return 0;
}

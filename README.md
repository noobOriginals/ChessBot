## Well, hello there

This is my first try at coding a chess engine from scratch with little knowledge and pack it with all the cool kid features out there

## Current features

- Make/unmake moves (can be transformed from strings to internal type using `getMoveFromAlgebraic()`)
- Has precomputed magic bitboard attack sets for rooks, bishops and, inherently, queens (can be accessed from `attacks.h`)
- Rest of attack sets are easily precomputed for each square (knight/king)
- Pawns have two different 'attacks': the real attack, diagonal captures (precomputed), and the pawn push (which is computed for all pawns at once with `getPawnPushes()` and `getPawnDoublePushes()`)
- If you build and run this (`cmake -B build -S . -G <your_generator_choice> && cmake --build build --config <Debug/Release> -j<threads>`) you can interact with it and make moves on the ASCII string, console-displayed, board. Commands: normal UCI-style moves (`e2e4`), `undo`, `redo` (as long as that move was undoed before) `quit` (or `q`), `bench <depth>` (measures raw speed, movegen + make/unamke every single move), `perft <depth>` (bench but with bulk counting, massive speedup for perft) and `experft <depth>` (extended perft, also provides more detailed counts - though not perfect for discovered checks and double checks - additional move types for more insight into the position's perft).
- Also when it prompts for `initial board state`, the command is something like this `startpos moves e2e4 e7e5 g1f3 b8c6 f1b5 a7a6 go` or `fen rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 moves e2e4 e7e5 g1f3 b8c6 f1b5 a7a6 go`. `startpos` means reset the board to the starting fen, or you can input any FEN you want, each move after that will be made on the board before displaying anything else, and `go` means the list of moves ended and it can start the interactive interaction
- Movegen ('blazingly fast' with magic bitbords and pin masks, etc...) via `getLegalMoves()`

## Upcoming features (hopefully completed sooner than later, but I won't promise anything)

- Some basic eval/search algorithm for simple play
- UCI protocol implementation so anyone who has a UCI GUI can play this engine

## Far away (but hopefully implemented at some point) features

- Refined search algorithm
- Some kind of top-tier evaluation (NNUE or similar)
- Improve attack lookup speed even more with **PEXT** on x86 and **Dual Hyperbola Quintessence** on ARM (due to efficient RBIT instruction)

## Side note

This README is not meant as a documentation, so don't expect a lot from it, I did my best I swear.

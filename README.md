## Well, hello there

This is my first try at coding a chess engine from scratch with little knowledge and pack it with all the cool kid features out there

## Current features

- Make/unmake moves (can be transformed from strings to internal type using `getMoveFromAlgebraic()`)
- Has precomputed magic bitboard attack sets for rooks, bishops and, inherently, queens (can be accessed from `attacks.h`)
- Rest of attack sets are easily precomputed for each square (knight/king)
- Pawns have two different 'attacks': the real attack, diagonal captures (precomputed), and the pawn push (which is computed for all pawns at once with `getPawnPushes()` and `getPawnDoublePushes()`)
- If you build and run this (`cmake -B build -S . -G <your_generator_choice> && cmake --build build --config <Debug/Release> -j<threads>`) you can interact with it and make moves on the ASCII string, console-displayed, board. Commands: normal UCI-style moves (`e2e4`), `undo`, `redo` (as long as that move was undoed before) `quit` and `print <x>`, where `x` can be any ONE of `P N B R Q K p n b r q k W w X x` (`W w` represent the single push bitboard for respective colored pawns, `X x` for double push, rest are self explanatory)
- Also when it prompts for `initial board state`, the command is something like this `startpos e2e4 e7e5 g1f3 b8c6 f1b5 a7a6 go`. `startpos` means reset the board, each move after that will be made on the board before displaying anything else, and `go` means the list of moves ended and it can start the interactive interaction

## Upcoming features (hopefully completed sooner than later, but I won't promise anything)

- Movegen ('blazingly fast' with magic bitbords and pin masks, etc...)
- Some basic eval/search algorithm for simple play
- UCI protocol implementation so anyone who has a UCI GUI can play this engine

## Far away (but hopefully implemented at some point) features

- Refined search algorithm
- Some kind of top-tier evaluation (NNUE or similar)
- Improve attack lookup speed even more with **PEXT** on x86 and **Dual Hyperbola Quintessence** on ARM (due to efficient RBIT instruction)

## Side note

This README is not meant as a documentation, so don't expect a lot from it, I did my best I swear.

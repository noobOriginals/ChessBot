namespace ChessBot;

public class Board {
    // Square array: the uni-dimasional representation of the chess board.
    // CastleRights array: holds the possibility of castling of the two sides.
    // WhiteToMove: represents which oponent's turn it is to move.
    // LastMove: holds the last move that was made, useful for identifying en passants and seraching for legal moves.
    // HalfMoves: I still haven't read about what this means, but I'm keeping track of it when loaded from a FEN, as it may be needed later.
    // FullMoves: The number of white-black turns taken until now.
    // WhitePieces, BlackPieces: keeps track of the indices of all of white's and black's pieces. Useful for legal move generation.
    static byte[] Square = new byte[64];
    static bool[] CastleRights = new bool[4];
    static bool WhiteToMove = true;
    static Move? LastMove;
    static int HalfMoves, FullMoves;
    static int[] WhitePieces = new int[16];
    static int[] BlackPieces = new int[16];
    
    // Reset(fen): resets the board to the given FEN string position.
    // Reset(): resets the board to startpos with the respective FEN string.
    public static void Reset(string fen) {
        Square = new byte[64];
        CastleRights = new bool[4];
        WhitePieces = new int[16];
        BlackPieces = new int[16];
        int wpi = 0, bpi = 0;
        char c;
        int s = 0;
        int last = 0;
        for (int i = 0; i < fen.Length; i++) {
            c = fen.ElementAt(i);
            if (c < 58 && c > 47) {
                s += int.Parse(c.ToString());
            } else if (c == '/') {
                s += (s % 8 == 0) ? 0 : 8 - (s % 8);
            } else {
                byte piece = Piece.ToPiece(c);
                Square[s] = piece;
                if (Piece.IsColor(piece, Piece.White)) {
                    WhitePieces[wpi] = s;
                    wpi++;
                } else {
                    BlackPieces[bpi] = s;
                    bpi++;
                }
                s++;
            }
            if (s > 63) {
                last = i;
                break;
            }
        }
        last += 2;
        c = fen.ElementAt(last);
        WhiteToMove = c == 'w';
        last += 2;
        string castle = fen[last..fen.IndexOf(' ', last)];
        if (castle.Length > 0) {
            last = fen.IndexOf(' ', last);
            for (int i = 0; i < castle.Length; i++) {
                c = castle.ElementAt(i);
                switch (c) {
                    case 'K':
                        CastleRights[0] = true;
                        break;
                    case 'Q':
                        CastleRights[1] = true;
                        break;
                    case 'k':
                        CastleRights[2] = true;
                        break;
                    case 'q':
                        CastleRights[3] = true;
                        break;
                }
            }
        }
        last++;
        c = fen.ElementAt(last);
        if (c != '-') {
            string en = fen[last..fen.IndexOf(' ', last)];
            LastMove = Move.FromEnPassantTargetSquare(en);
        } else {
            LastMove = null;
        }
        last = fen.IndexOf(' ', last) + 1;
        string hm = fen[last..fen.IndexOf(' ', last)];
        HalfMoves = int.Parse(hm);
        last = fen.IndexOf(' ', last) + 1;
        string fm = fen[last..];
        FullMoves = int.Parse(fm);
    }
    public static void Reset() {
        Reset("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    }

    public static byte[,] To2DArray() {
        byte[,] square = new byte[8, 8];
        for (int i = 0; i < 64; i++) {
            int file, rank;
            file = i % 8;
            rank = (i - file) / 8;
            square[rank, file] = Square[i];
        }
        return square;
    }
    public static Move[] GenerateLegalMoves() {
        foreach (int friendlyPiece in WhiteToMove ? WhitePieces : BlackPieces) {
            byte pieceType = Piece.GetPieceType(Square[friendlyPiece]);
            int friendlyFile = friendlyPiece % 8;
            int friendlyRank = (friendlyPiece - friendlyFile) / 8;
            int[] closestOpponent = Enumerable.Repeat(-1, 8).ToArray();
            int[] closestOppPos = Enumerable.Repeat(8, 8).ToArray();
            foreach (int opponentPiece in WhiteToMove ? BlackPieces : WhitePieces) {
                int opponentFile = opponentPiece % 8;
                int opponentRank = (opponentPiece - opponentFile) / 8;
                if (pieceType == Piece.Knight) { goto knights; }
                if (pieceType == Piece.Bishop) { goto bishops; }
                // Same file
                if (opponentFile - friendlyFile == 0) {
                    if (opponentPiece < friendlyPiece && friendlyRank - opponentRank < closestOppPos[0]) {
                        closestOpponent[0] = opponentPiece;
                        closestOppPos[0] = friendlyRank - opponentRank;
                    } else if (opponentRank - friendlyRank < closestOppPos[2]) {
                        closestOpponent[2] = opponentPiece;
                        closestOppPos[2] = opponentRank - friendlyRank;
                    }
                }
                // Same rank
                if (opponentRank - friendlyRank == 0) {
                    if (opponentPiece > friendlyPiece && opponentFile - friendlyFile < closestOppPos[1]) {
                        closestOpponent[1] = opponentPiece;
                        closestOppPos[1] = opponentFile - friendlyFile;
                    } else if (friendlyFile - opponentFile < closestOppPos[3]) {
                        closestOpponent[3] = opponentPiece;
                        closestOppPos[3] = friendlyFile - opponentFile;
                    }
                }
                if (pieceType == Piece.Rook) { continue; }
            bishops:
                // Upwards diagonal
                if (opponentFile - friendlyFile + opponentRank - friendlyRank == 0) {
                    if (opponentPiece < friendlyPiece && opponentFile - friendlyFile < closestOppPos[4]) {
                        closestOpponent[4] = opponentPiece;
                        closestOppPos[4] = opponentFile - friendlyFile;
                    } else if (friendlyFile - opponentFile < closestOppPos[6]) {
                        closestOpponent[6] = opponentPiece;
                        closestOppPos[6] = friendlyFile - opponentFile;
                    }
                }
                // Downwards diagonal
                if (opponentFile - friendlyFile - opponentRank + friendlyRank == 0) {
                    if (opponentPiece > friendlyPiece && opponentFile - friendlyFile < closestOppPos[5]) {
                        closestOpponent[5] = opponentPiece;
                        closestOppPos[5] = opponentFile - friendlyFile;
                    } else if (friendlyFile - opponentFile < closestOppPos[7]) {
                        closestOpponent[7] = opponentPiece;
                        closestOppPos[7] = friendlyFile - opponentFile;
                    }
                }
            knights:
                if (Math.Abs(opponentRank - friendlyRank) - Math.Abs(opponentFile - friendlyFile) == 1) {
                    
                }
            }

            // Move generation:

            
        }

        int maxMoves = 120;
        Move[] moves = new Move[maxMoves];
        return moves;
    }

    // Print(): writes a visual representation of the board to the console.
    // PrintInfo(): also writes additional information that is stored in the Board class.
    public static void Print() {
        Console.WriteLine("+---+---+---+---+---+---+---+---+");
        Console.Write("|");
        for (int i = 0; i < 64; i++) {
            Console.Write(" " + Piece.ToChar(Square[i]) + " |");
            if ((i + 1) % 8 == 0) {
                Console.WriteLine(" " + (9 - (i + 1) / 8));
                Console.WriteLine("+---+---+---+---+---+---+---+---+");
                if (i == 63) {
                    break;
                }
                Console.Write("|");
            }
        }
        Console.WriteLine("  a   b   c   d   e   f   g   h  ");
    }
    public static void PrintInfo() {
        Console.WriteLine("Castle Rights:");
        Console.WriteLine("  White King-Side: " + CastleRights[0]);
        Console.WriteLine("  White Queen-Side: " + CastleRights[1]);
        Console.WriteLine("  Black King-Side: " + CastleRights[2]);
        Console.WriteLine("  Black Queen-Side: " + CastleRights[3]);
        Console.WriteLine(WhiteToMove ? "White to move" : "Black to move");
        Console.WriteLine("Last move: " + ((LastMove == null) ? "-" : LastMove.ToString()));
        Console.WriteLine("Half moves: " + HalfMoves + "  Full moves: " + FullMoves);
        Console.WriteLine("White pieces indices: ");
        for (int i = 0; i < 16; i++) {
            string add = WhitePieces[i] < 10 ? " " : "";
            Console.Write("  " + WhitePieces[i] + add);
        }
        Console.WriteLine();
        Console.WriteLine("Black pieces indices: ");
        for (int i = 0; i < 16; i++) {
            string add = BlackPieces[i] < 10 ? " " : "";
            Console.Write("  " + BlackPieces[i] + add);
        }
        Console.WriteLine();
        Console.WriteLine();
        Print();
    }
}
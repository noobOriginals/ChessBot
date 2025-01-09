using System.Drawing;

namespace ChessBot;

class Program {
    static void Main(string[] args) {
        // Debug
        // Board.Reset("rnbqkbnr/pp4p1/2pP3p/3Ppp2/8/8/PPP2PPP/RNBQKBNR w KQkq e6 0 6");
        // Board.PrintInfo();
        // Board.GenerateLegalMoves();

        Bitboards.Print(Bitboards.GenDiagLeft(7, 4, 4, false));
        Bitboards.Print(Bitboards.GenDiagLeft(4, 4, 4, true));
        Bitboards.Print(Bitboards.GenDiagRight(0, 4, 4, false));
        Bitboards.Print(Bitboards.GenDiagLeft(4, 0, 0, false));
    }
}

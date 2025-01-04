namespace ChessBot;

class Program {
    static void Main(string[] args) {
        // Debug
        // Board.Reset("rnbqkbnr/pp4p1/2pP3p/3Ppp2/8/8/PPP2PPP/RNBQKBNR w KQkq e6 0 6");
        // Board.PrintInfo();
        // Board.GenerateLegalMoves();

        // ulong b = 24260;
        // string bs = Bitboards.Convert(b, 36, 0, false);
        // Console.WriteLine(bs);
        // Bitboards.Print(b);
        ulong b = Bitboards.GenHorizontal(0, 2, 5);
        Bitboards.Print(b);
    }
}

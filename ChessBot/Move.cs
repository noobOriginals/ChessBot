namespace ChessBot;

public class Move {
    // StartSquare, EndSquare: the simplest yet powerful representation of a move only by square indices.
    public int StartSquare, EndSquare;

    // Move(startSquare, endSquare): initializes member variables.
    // Move(move): generates a move base on a sigle string in ?long algebraic? notation (e.g. e2e3).
    // Move(startSquare, endSquare): generates a move based on a start and end square provided as strings.
    public Move(int startSquare, int endSquare) {
        StartSquare = startSquare;
        EndSquare = endSquare;
    }
    public Move(string move) {
        string alph = "abcdefgh";
        int rank, file;
        rank = 8 - int.Parse(move.ElementAt(1).ToString());
        file = alph.IndexOf(move.ElementAt(0));
        StartSquare = rank * 8 + file;
        rank = 8 - int.Parse(move.ElementAt(3).ToString());
        file = alph.IndexOf(move.ElementAt(2));
        EndSquare = rank * 8 + file;
    }
    public Move(string startSquare, string endSquare) {
        string  move = startSquare + endSquare;
        string alph = "abcdefgh";
        int rank, file;
        rank = 8 - int.Parse(move.ElementAt(1).ToString());
        file = alph.IndexOf(move.ElementAt(0));
        StartSquare = rank * 8 + file;
        rank = 8 - int.Parse(move.ElementAt(3).ToString());
        file = alph.IndexOf(move.ElementAt(2));
        EndSquare = rank * 8 + file;
    }

    // Generates a string in ?long algebraic? notation (e.g. e2e3) based on current values of member variables.
    public override string ToString() {
        string move = "";
        string alph = "abcdefgh";
        int rank, file;
        file = StartSquare % 8;
        rank = 8 - (StartSquare - file) / 8;
        move += "" + alph.ElementAt(file);
        move += "" + rank;
        file = EndSquare % 8;
        rank = 8 - (EndSquare - file) / 8;
        move += "" + alph.ElementAt(file);
        move += "" + rank;
        return move;
    }

    // Generates a move from a string representing the enPassant target square in the current position.
    // Useful for calculating the last move made based on a FEN string.
    public static Move FromEnPassantTargetSquare(string targetSquare) {
        string file = targetSquare.ElementAt(0).ToString();
        Move move;
        if (targetSquare.ElementAt(1) == '3') {
            move = new Move(file + "2" + file + "4");
        } else {
            move = new Move(file + "7" + file + "5");
        }
        return move;
    }
}
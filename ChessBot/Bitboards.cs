namespace ChessBot;

public class Bitboards {
    public static ulong[,] Queen = new ulong[64, 8];

    // Init(): initializes the bitboard lookup tables. 
    public static void Init() {
        Queen = new ulong[64, 8];
        for (int i = 0; i < 64; i++) {
            int file = i % 8;
            int rank = (i - file) / 8;
            Queen[i, 0] = (ulong) (9);
        }    
    }

    // Utility:
    // GenerateRow(idx): generates a blank 8 bit row with a 1 at the specified index.
    // AddRows(row0, row1): adds two rows togheter, putting the first on top of the second (shifts the first row 8 bits to the left).
    // GenVertical(file, rowcount, offsetrows): generates a bitboard with vertical-oriented indicees on the given file.
    // GenHorizontal(startfile, count, offsetrows): generates a bitboard with horizontal-oriented indicees starting from the given file.
    // GenDiagRight(startfile, count, offsetrows, reverse): generates a bitboard with bits oriented from top-left to bottom right.
    // GenDiagLeft(startfile, count, offsetrows, reverse): generates a bitboard with bits oriented from top-left to bottom right.
    public static ulong GenerateRow(int idx) {
        return (ulong) (1 << (7 - idx));
    }
    public static ulong AddRows(ulong row0, ulong row1) {
        row0 <<= 8;
        return row0 | row1;
    }
    public static ulong GenVertical(int file, int rowcount, int offsetrows) {
        ulong row = (ulong) (1 << (7 - file));
        for (int i = 0; i < rowcount - 1; i++) {
            row = AddRows(row, row);
        }
        row <<= 8 * offsetrows;
        return row;
    }
    public static ulong GenHorizontal(int startfile, int count, int offsetrows) {
        ulong row = 0;
        for (int i = 0; i < count; i++) {
            row <<= 1;
            row |= 1;
        }
        row <<= 8 - (startfile + count);
        row <<= 8 * offsetrows;
        return row;
    }
    public static ulong GenDiagRight(int startfile, int count, int offsetrows, bool reverse) {
        if (reverse) {
            if (count == 0) {
                count = startfile + 1;
                return GenDiagRight(startfile - count + 1, count, offsetrows, false);
            }
            return GenDiagRight(startfile - count + 1, count, offsetrows, false);
        }
        ulong row = 1;
        if (count == 0) {
            count = 8 - startfile;
            for (int i = 0; i < count - 1; i++) {
                row <<= 9;
                row |= 1;
            }
        } else {
            for (int i = 0; i < count - 1; i++) {
                row <<= 9;
                row |= 1;
            }
        }
        row <<= 7 - startfile - (count - 1);
        row <<= 8 * offsetrows;
        return row;
    }
    public static ulong GenDiagLeft(int startfile, int count, int offsetrows, bool reverse) {
        if (reverse) {
            if (count == 0) {
                count = 8 - startfile;
                return GenDiagLeft(startfile - 1 + count, count, offsetrows, false);
            }
            return GenDiagLeft(startfile - 1 + count, count, offsetrows, false);
        }
        ulong row = 1;
        if (count == 0) {
            count = startfile + 1;
            for (int i = 0; i < count - 1; i++) {
                row <<= 7;
                row |= 1;
            }
        } else {
            for (int i = 0; i < count - 1; i++) {
                row <<= 7;
                row |= 1;
            }
        }
        row <<= 7 - startfile + (count - 1);
        row <<= 8 * offsetrows;
        return row;
    }

    // Print(bitboard): prints the bitboard to the console on a chess board, using 1s and 0s.
    public static void Print(ulong bitboard) {
        Console.WriteLine("+---+---+---+---+---+---+---+---+");
        Console.Write("|");
        string binStr = ToString(bitboard);
        for (int i = 0; i < 64; i++) {
            Console.Write(" " + BitAt(bitboard, i, true) + " |");
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
    // BitAt(bitboard, idx, reverse): returns the value of the bit of a number at a given index.
    // If reverse is true, than the bit is indexed as a char in a string, from left to right.
    // Otherwise, the bit is indexed based on the power of the base (from right to left).
    public static byte BitAt(ulong bitboard, int idx, bool reverse) {
        if (reverse) {
            bitboard <<= idx;
        } else {
            bitboard <<= 63 - idx;
        }
        return (byte) (bitboard >> 63);
    }

    // Convert(bitboard, numbase, chars, reverse):
    // Fun little conversion function which can convert any unsigned integers (up to 64 bits) to any given base (from 2 to 36).
    // If chars is set to 0 than only the "bits" that are generated are outputed to the output string.
    // Otherwise, the total number of chars in the returned string can be controled with the "chars" parameter.
    // If reverse is set to true, then the "bits" of the number are going to be read backwards
    // (e.g. input 231 and convert to base 10 reversed -> 132 as a string).
    public static string Convert(ulong bitboard, uint numbase, int chars, bool reverse) {
        if (numbase > 36) throw new Exception("Cannot convert to bases higher than 36. Given base: " + numbase + ".");
        if (numbase < 2) throw new Exception("Cannot convert to base 1.");
        if (chars > 64) throw new Exception("Cannot display numbers with more than 64 characters." + numbase);
        if (chars < 0) throw new Exception("Cannot display numbers with less than 1 character.");
        string alph = "0123456789abcdefghijklmnopqrstuvwxyz";
        string val = "";
        while (bitboard > 1) {
            if (reverse) {
                val += alph.ElementAt((int)(bitboard % numbase));
            } else {
                val = alph.ElementAt((int)(bitboard % numbase)) + val;
            }
            bitboard = bitboard / numbase;
        }
        if (reverse) {
            val += alph.ElementAt((int)bitboard);
        } else {
            val = alph.ElementAt((int)bitboard) + val;
        }
        if (chars == 0) return val.ElementAt(0) == '0' ? val.Substring(1) : val;
        int max = chars - val.Length;
        if (max < 0) {
            val = val.Substring(val.Length - chars, chars);
        } else {
            for (int i = 0; i < max; i++) {
                if (reverse) {
                    val += "0";
                } else {
                    val = "0" + val;
                }
            }
        }
        return val;
    }
    // ToString(bitboard): simplifies the call of Convert(bitboard, 2, 64, false) to only ToString(bitboard) for easier use.
    public static string ToString(ulong bitboard) {
        return Convert(bitboard, 2, 64, false);
    }
}
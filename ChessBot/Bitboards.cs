namespace ChessBot;

public class Bitboards {
    public static ulong[,] Queen = new ulong[64, 8];

    public static void Init() {
        Queen = new ulong[64, 8];
        for (int i = 0; i < 64; i++) {
            int file = i % 8;
            int rank = (i - file) / 8;
            Queen[i, 0] = (ulong) (9);
        }    
    }

    public static ulong GenerateRow(int idx) {
        return (ulong) (1 << (7 - idx));
    }
    public static ulong AddRows(ulong row0, ulong row1) {
        row0 <<= 8;
        return row0 | row1;
    }
    public static ulong GenVertical(int idx, int rowcount, int offsetrows) {
        ulong row = (ulong)(1 << (7 - idx));
        for (int i = 0; i < rowcount - 1; i++) {
            row = AddRows(row, row);
        }
        row <<= 8 * offsetrows;
        return row;
    }
    public static ulong GenHorizontal(int idx, int count, int offsetrows) {
        ulong row = 0;
        for (int i = 0; i < count; i++) {
            row <<= 1;
            row |= 1;
        }
        row <<= 7 - (idx + count);
        row <<= 8 * offsetrows;
        return row;
    }

    public static void Print(ulong bitboard) {
        Console.WriteLine("+---+---+---+---+---+---+---+---+");
        Console.Write("|");
        string binStr = ToString(bitboard);
        for (int i = 0; i < 64; i++) {
            Console.Write(" " + binStr.ElementAt(i) + " |");
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
        if (chars == 0) return val;
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
    public static string ToString(ulong bitboard) {
        return Convert(bitboard, 2, 64, false);
    }
}
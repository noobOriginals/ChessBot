namespace ChessBot;

public class Piece {
    // Constants for representing a piece/square with a single Byte.
    public const byte White = 0b1000;
    public const byte Black = 0b10000;
    public const byte None = 0b0;
    public const byte Pawn = 0b1;
    public const byte Knight = 0b10;
    public const byte Bishop = 0b11;
    public const byte Rook = 0b100;
    public const byte Queen = 0b101;
    public const byte King = 0b110;

    // GetPieceColor(piece): returns the color of the piece equivalent to the color constant of the piece.
    // GetPieceType(piece): returns the type of the piece equivalent to the type constant of the piece.
    // IsColor(piece, color): returns true or false depending of the piece matching the color given;
    // IsType(piece, type): returns true or false depending of the piece matching the type given;
    public static byte GetPieceColor(byte piece) {
        piece = (byte) (piece >> 3);
        return (byte) (piece << 3);;
    }
    public static byte GetPieceType(byte piece) {
        piece = (byte) (piece | (3 << 3));
        return (byte) (piece ^ (3 << 3));;
    }
    public static bool IsColor(byte piece, byte color) {
        return GetPieceColor(piece) == color; 
    }
    public static bool IsType(byte piece, byte type) {
        return GetPieceType(piece) == type;
    }

    // ToChar(piece): based on a sigle character, returns a piece with a certain type. Useful for reading FEN's
    // ToPiece(value): based on a piece, returns a single character representing that piece. Useful for creating FEN's
    public static char ToChar(byte piece) {
        bool uppercase = IsColor(piece, White);
        piece = GetPieceType(piece);
        return piece switch {
            None => ' ',
            Pawn => uppercase ? 'P' : 'p',
            Knight => uppercase ? 'N' : 'n',
            Bishop => uppercase ? 'B' : 'b',
            Rook => uppercase ? 'R' : 'r',
            Queen => uppercase ? 'Q' : 'q',
            King => uppercase ? 'K' : 'k',
            _ => ' ',
        };
    }
    public static byte ToPiece(char value) {
        bool uppercase = Char.IsUpper(value);
        return Char.ToUpper(value) switch {
            ' ' => None,
            'P' => (byte) ((uppercase ? White : Black) | Pawn),
            'N' => (byte) ((uppercase ? White : Black) | Knight),
            'B' => (byte) ((uppercase ? White : Black) | Bishop),
            'R' => (byte) ((uppercase ? White : Black) | Rook),
            'Q' => (byte) ((uppercase ? White : Black) | Queen),
            'K' => (byte) ((uppercase ? White : Black) | King),
            _ => None,
        };
    }
}
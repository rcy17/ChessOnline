#ifndef CHESS_H
#define CHESS_H
#include <QList>

class Chess
{
public:
    enum Type
    {
        KING,
        QUEEN,
        BISHOP,
        ROOK,
        KNIGHT,
        PAWN,
    };
    static Chess * addChess(Type type, bool white);
    static void initialize();
    Type type;
    bool white;
    ~Chess();

private:
    Chess(Type type, bool white);
    static QList <Chess *> allChess;

};

#endif // CHESS_H

#include "chess.h"

QList <Chess *> Chess::allChess = {};

Chess::Chess(Type type, bool white): type(type), white(white)
{

}

Chess * Chess::addChess(Type type, bool white)
{
    auto chess = new Chess(type, white);
    allChess.append(chess);
    return chess;
}

Chess::~Chess()
{

}


void Chess::initialize()
{
    for (auto chess: allChess)
    {
        delete chess;
    }
    allChess.clear();
}

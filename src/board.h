#ifndef BOARD_H
#define BOARD_H

#include <QWidget>
#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QPixmap>
#include "instruction.h"
#include "chess.h"
#include <QDebug>

namespace Ui {
class Board;
}

class Board : public QWidget
{
    Q_OBJECT
public:
    enum Mask
    {
        NONE,
        SELECTED,
        MOVABLE,
        KILLABLE,
        FRIEND,
        ENEMY,
    };

    enum Result
    {
        BLACKWIN,
        WHITEWIN,
        DRAW,
        STALEMATE,
        BLACKADMIT,
        WHITEADMIT,
        BLACKTIMEOUT,
        WHITETIMEROUT
    };

signals:
    void operated(Instruction ins);
    void pawnUpdated(int pos);
    void gameOvered(Result winner);
    void playingChanged(bool isPlaying);

public slots:
    void execute(Instruction ins);


public:
    explicit Board(QWidget *parent = nullptr);
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);

    void updateBackground();
    void updateChess();
    void updateMask();
    void initialize();
    void setColor(bool white);
    static void staticInitialize();
    void loadDefaultEnd();
    void loadEnd(QTextStream &);
    bool executing();
    QStringList save();
    void gameStart();
    void forceOver(Result);
    bool isPlaying();

    // if it remains over window
    bool isChached();

    ~Board();

    Chess * board[64];

private:
    Ui::Board *ui;
    QPixmap background, image, masked;
    static int eachWidth, eachHeight;
    // save current move color
    bool currentWhite;
    // save player's color
    bool playWhite;
    // if king and rook have swapped
    bool swapped;
    // while updating pawn
    bool updating;
    // should always be playWhite == currentWhite
    bool playing;
    bool gameover;

    int selected;
    Mask mask[64];
    int kingPos[2];
    bool danger[64];
    bool cache;
    QList <int> availablePos;

    static QImage chessImage[2][8];

    static const int imageWidth = 1000;
    static const int imageHeight = 1000;
    static const QColor deep, light;
    static QList <QString> nameList;

    void addChess(Chess::Type type, const QList<QString> & data, bool white);
    void updateSelected(bool white);
    void calculateMask(bool white);
    void move(int pos1, int pos2);
    void swap(int pos1, int pos2);
    bool checkUpdate(int pos2);
    Mask check(int pos, bool white);
    void pawnUpdate(int pos, int target);
    void exchangeTurn();
    void gameOver(Result winner);
    bool checkStalemate(bool white);
    void getDangerousArea(bool white);
};


inline int wordToPos(QString s)
{
    auto string = s.toStdString();
    auto x = string[0] - 'a';
    auto y = string[1] - '1';
    // use computer coordinate system
    return  (7 - y) * 8 + x;
}

inline QString posToWord(int pos)
{
    int y = 7 - pos / 8;
    char x = pos % 8;
    return QString(QChar(x + 'a')) + QString("%1").arg(y + 1);
}




#endif // BOARD_H

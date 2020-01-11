#include "board.h"
#include "ui_board.h"
#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QMessageBox>

const QString _nameList = "king,queen,bishop,rook,knight,pawn";

const QColor Board::deep = QColor("gray");
const QColor Board::light = QColor("white");
int Board::eachWidth = Board::imageWidth / 8;
int Board::eachHeight = Board::imageHeight / 8;
QList <QString> Board::nameList = _nameList.split(',');
QImage Board::chessImage[2][8] = {};

Board::Board(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Board),
    background(imageWidth, imageHeight),
    playWhite(true)
{
    ui->setupUi(this);
    initialize();
    updateBackground();
}

Board::~Board()
{
    delete ui;
}

void Board::updateChess()
{
    image = background.copy();
    QPainter p(&image);
    p.setRenderHint(QPainter::Antialiasing, true);
    for(int pos = 0;pos < 64; pos++)
    {
        auto chess = board[pos];
        if(!chess)
            continue;
        // rotate board
        int _pos = playWhite ? pos : 63 - pos;
        QRect r(_pos % 8 * eachWidth, _pos / 8 * eachHeight, eachWidth, eachHeight);
        p.drawImage(r, chessImage[chess->white][chess->type]);
    }
    updateMask();
}

void Board::updateMask()
{
    masked = image.copy();
    QPainter p(&masked);
    for(int pos = 0;pos < 64; pos++)
        if(mask[pos])
        {
            auto _pos = playWhite ? pos : 63 - pos;
            p.save();
            p.translate(eachWidth * (_pos % 8), eachHeight * (_pos / 8));
            switch (mask[pos])
            {
            case SELECTED:
            {
                p.setPen(Qt::NoPen);
                p.setBrush(QColor(0, 255, 0, 127));
                p.drawRect(0, 0, eachWidth, eachHeight);
                break;
            }
            case MOVABLE:
            {
                auto penWidth = eachWidth / 20;
                p.setPen(QPen(QColor(0, 255, 0), penWidth));
                p.drawRect(penWidth / 2,  penWidth / 2, eachWidth - penWidth, eachHeight - penWidth);
                break;
            }
            case KILLABLE:
            {
                auto penWidth = eachWidth / 20;
                p.setPen(QPen(QColor(255, 0, 0), penWidth));
                p.drawRect(penWidth / 2,  penWidth / 2, eachWidth - penWidth, eachHeight - penWidth);
                break;
            }
            case ENEMY:
            {
                p.setPen(Qt::NoPen);
                p.setBrush(QColor(255, 0, 0, 127));
                p.drawRect(0, 0, eachWidth, eachHeight);
                break;
            }
            default:
                break;
            }
            p.restore();
        }

    if(danger[kingPos[currentWhite]])
    {
        p.setPen(Qt::NoPen);
        const QColor redMask(127, 0, 0, 127);
        const QColor greenMask(0, 0, 127, 127);
        p.setBrush(playing ? redMask : greenMask);
        p.drawRect(0, 0, imageWidth, imageHeight);
    }

    if(updating)
    {
        p.setBrush(QColor(60,60,60));
        p.translate(eachWidth * 3, eachHeight * 3);
        p.drawRoundedRect(0, 0, eachWidth * 2, eachHeight * 2, eachWidth / 10, eachHeight / 10);
        p.drawImage(QRect(0, 0, eachWidth, eachHeight), chessImage[currentWhite][playWhite ? Chess::QUEEN : Chess::ROOK]);
        p.drawImage(QRect(eachWidth, 0, eachWidth, eachHeight), chessImage[currentWhite][playWhite ? Chess::BISHOP: Chess::KNIGHT]);
        p.drawImage(QRect(0, eachHeight, eachWidth, eachHeight), chessImage[currentWhite][playWhite ? Chess::KNIGHT: Chess::BISHOP]);
        p.drawImage(QRect(eachWidth, eachHeight, eachWidth, eachHeight), chessImage[currentWhite][playWhite ? Chess::ROOK: Chess::QUEEN]);
    }

    repaint();
}

void Board::mousePressEvent(QMouseEvent * e)
{
    if(gameover)
        return;
    if(playing && e->button() == Qt::LeftButton)
    {
        auto x = e->x() / (width() / 8);
        auto y = e->y() / (height() / 8);
        if(x == 8)
            x = 7;
        if(y == 8)
            y = 7;
        auto _pos = y * 8 + x;
        auto pos = playWhite ? _pos : 63 - _pos;
        if(updating)
        {
            // pawn update
            int target;
            switch(pos)
            {
            case 3 * 8 + 3:
                target = Chess::QUEEN;
                break;
            case 3 * 8 + 4:
                target = Chess::BISHOP;
                break;
            case 4 * 8 + 3:
                target = Chess::KNIGHT;
                break;
            case 4 * 8 + 4:
                target = Chess::ROOK;
                break;
            default:
                target = -1;
                return;
            }
            if(target != -1)
                emit operated({Instruction::UPDATE, selected, target});
        }
        else
        {
            if(mask[pos] == MOVABLE || mask[pos] == KILLABLE)
            {
                if( pos != selected)
                {
                    if(selected == kingPos[currentWhite] && abs(selected - pos) > 1 && selected / 8 == pos / 8)
                        // swap king and rook
                        emit operated({Instruction::SWAP, selected, pos});
                    else
                        emit operated({Instruction::MOVE, selected, pos});
                }
            }
            else if(board[pos])
                selected = pos;
            else
                selected = -1;
            updateSelected(playWhite);
        }
    }
    QWidget::mousePressEvent(e);
}

void Board::updateSelected(bool white)
{
    if(updating)
        return;
    memset(mask, NONE, sizeof(mask));
    if(selected != -1)
    {
        if(!board[selected])
        {
            assert(0);
        }
        if(board[selected]->white != white)
            mask[selected] = ENEMY;
        else
        {
            mask[selected] = SELECTED;
            if(playing)
                calculateMask(white);
        }
    }
 updateMask();
}

inline bool illegal(int x, int y)
{
    // x <0 || x >= 8 || y < 0 || y >= 8
    return (x | y) & 8;
}

Board::Mask Board::check(int pos, bool white)
{
    auto chess = board[pos];
    if(!chess)
        return MOVABLE;
    else if(chess->white == white)
        return FRIEND;
    else
        return KILLABLE;
}

void Board::calculateMask(bool white)
{
    // all move rules are implemented here
    assert(selected > -1);
    auto y = selected / 8;
    auto x = selected % 8;
    auto dy = y;
    auto dx = x;
    availablePos.clear();
    switch (board[selected]->type)
    {
    case Chess::KING:
    {
        for(dx = x - 1; dx < x + 2;dx++)
            for(dy = y - 1; dy < y + 2; dy++)
            {
                if(dx == x && dy == y)
                    continue;
                if(illegal(dx, dy))
                    continue;
                auto pos = dy * 8 + dx;
                mask[pos] = check(pos, white);
                if(mask[pos] == MOVABLE || mask[pos] == KILLABLE)
                    availablePos.append(pos);
            }
        if(!danger[kingPos[white]] && x == 4 && !((y + white) & 7))
        {
            // swap king and rook
            bool flag = true;
            if(board[y * 8  + 0] && board[y * 8 + 0]->type == Chess::ROOK)
            {
                for(dx = x - 1;dx > 0;dx--)
                    if(board[y * 8 + dx] || danger[y * 8 + dx])
                        flag = false;
            }
            else
                flag = false;
            if(flag)
            {
                mask[y * 8 + 2] = MOVABLE;
                availablePos.append(y * 8 + 2);
            }

            flag = true;
            if(board[y * 8  + 7] && board[y * 8 + 7]->type == Chess::ROOK)
            {
                for(dx = x + 1;dx < 7;dx++)
                    if(board[y * 8 + dx] || danger[y * 8 + dx])
                        flag = false;
            }
            else
                flag = false;
            if(flag)
            {
                mask[y * 8 + 6] = MOVABLE;
                availablePos.append(y * 8 + 6);
            }
        }
        break;
    }
    case Chess::QUEEN:
    {
        const int delta_y[] = {1, 1, -1, -1, 1, -1, 0, 0};
        const int delta_x[] = {1, -1, 1, -1, 0, 0, 1, -1};
        for(int i = 0; i < 8; i++)
        {
            dy = y + delta_y[i];
            dx = x + delta_x[i];
            while(!illegal(dx, dy))
            {
                auto pos = dy * 8 + dx;
                auto result = check(pos, white);
                mask[pos] = result;
                if(mask[pos] == MOVABLE || mask[pos] == KILLABLE)
                    availablePos.append(pos);
                if(result != MOVABLE)
                    break;
                dy += delta_y[i];
                dx += delta_x[i];
            }
        }
        break;
    }
    case Chess::BISHOP:
    {
        const int delta_y[] = {1, 1, -1, -1};
        const int delta_x[] = {1, -1, 1, -1};
        for(int i = 0; i < 4; i++)
        {
            dy = y + delta_y[i];
            dx = x + delta_x[i];
            while(!illegal(dx, dy))
            {
                auto pos = dy * 8 + dx;
                auto result = check(pos, white);
                mask[pos] = result;
                if(mask[pos] == MOVABLE || mask[pos] == KILLABLE)
                    availablePos.append(pos);
                if(result != MOVABLE)
                    break;
                dy += delta_y[i];
                dx += delta_x[i];
            }
        }
        break;
    }
    case Chess::KNIGHT:
    {
        const int delta_y[] = {1, -1, 1, -1, 2, -2, 2, -2};
        const int delta_x[] = {2, 2, -2, -2, 1, 1, -1, -1};
        for(int i = 0; i < 8; i++)
        {
            dy = y + delta_y[i];
            dx = x + delta_x[i];
            if(!illegal(dx, dy))
            {
                auto pos = dy * 8 + dx;
                auto result = check(pos, white);
                mask[pos] = result;
                if(mask[pos] == MOVABLE || mask[pos] == KILLABLE)
                    availablePos.append(pos);
            }
        }
        break;
    }
    case Chess::ROOK:
    {
        const int delta_y[] = {1, -1, 0, 0};
        const int delta_x[] = {0, 0, 1, -1};
        for(int i = 0; i < 4; i++)
        {
            dy = y + delta_y[i];
            dx = x + delta_x[i];
            while(!illegal(dx, dy))
            {
                auto pos = dy * 8 + dx;
                auto result = check(pos, white);
                mask[pos] = result;
                if(mask[pos] == MOVABLE || mask[pos] == KILLABLE)
                    availablePos.append(pos);
                if(result != MOVABLE)
                    break;
                dy += delta_y[i];
                dx += delta_x[i];
            }
        }
        break;
    }
    case Chess::PAWN:
    {
        int factor = white ? -1 : 1;
        // (x, y + 1)
        auto pos = selected + factor * 8;
        auto result = check(pos, white);
        if(result == MOVABLE)
        {
            mask[pos] = MOVABLE;
            availablePos.append(pos);
        }
        if( result == MOVABLE && (white ? y == 6: y == 1))
        {
            // if this is first move, check (x, y + 2)
            pos = selected + factor * 16;
            result = check(pos, white);
            if(result==MOVABLE)
            {
                mask[pos] = MOVABLE;
                availablePos.append(pos);
            }
        }
        dy = y + factor;
        for(dx = x - 1; dx <= x + 1; dx+=2)
        {
            if(illegal(dx, dy))
                continue;
            pos = dy * 8 + dx;
            result = check(pos, white);
            if(result == KILLABLE)
            {
                mask[pos] = KILLABLE;
                availablePos.append(pos);
            }
        }
        break;
    }

    }
}

void Board::loadDefaultEnd()
{
    auto filename = ":/end/default.txt";
    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(parentWidget(), "open failed", QString("can't open file ") + filename);
        return;
    }

    QTextStream ts(&file);
    loadEnd(ts);
    file.close();
}

void Board::loadEnd(QTextStream & ts)
{
    initialize();
    QString s;
    bool firstline = true;
    bool white = false;
    while(true)
    {
        s = ts.readLine();
        if(!s.length())
            break;
        auto list = s.split(" ");
        auto name = list.first();
        if(list.length() == 1)
        {
            // list has only one element, which means this is white / black
            if(name == "black")
            {
                white = false;
                if(firstline)
                {
                    // notice that this will be reversed
                    currentWhite = true;
                    firstline = false;
                }
            }
            else if(name == "white")
            {
                white = true;
                if(firstline)
                {
                    currentWhite = false;
                    firstline = false;
                }
            }
            else
            {
                qDebug() << "read wrong word " << name;
            }
        }
        else
        {
            bool found = false;
            for(int i = 0; i < nameList.length(); i++)
                if(nameList[i] == name)
                {
                    found = true;
                    assert(list.length()>2);
                    addChess(static_cast<Chess::Type>(i), list.mid(2), white);
                    break;
                }
            if(!found)
                qDebug()<< "can't parse name " << name;
        }

    }
    exchangeTurn();
    updateChess();
}

void Board::updateBackground()
{
    QPainter p(&background);
    p.setPen(Qt::NoPen);
    p.setBrush(deep);
    for(int row = 0;row < 8;row++)
        for(int col = 0; col < 8;col++)
        {
            p.setBrush((row ^ col) & 1 ? light : deep);
            p.drawRect(eachWidth * row, eachHeight * col, eachWidth, eachHeight);
        }
    updateChess();
}

void Board::addChess(Chess::Type type, const QList<QString> & data, bool white)
{
    // auto n = list.first().toInt();
    for(auto s: data)
    {
        if(s.length()!=2)
        {
            qDebug()<<"can't parse coordinate " << s;
            continue;
        }
        auto pos = wordToPos(s);
        board[pos] = Chess::addChess(type, white);
        if(type == Chess::KING)
            kingPos[white] = pos;
    }
}

void Board::paintEvent(QPaintEvent * e)
{
    QPainter p(this);
    p.drawPixmap(rect(), masked);
    QWidget::paintEvent(e);
}

void Board::initialize()
{
    memset(board, 0, sizeof(board));
    memset(mask, 0, sizeof(mask));
    memset(danger, 0, sizeof(danger));
    memset(kingPos, 0 ,sizeof(kingPos));
    Chess::initialize();
    availablePos.clear();
    swapped = false;
    selected = -1;
    playing = false;
    updating = false;
    currentWhite = true;
    gameover = true;
    cache = false;
}

void Board::staticInitialize()
{
    for(int white=0; white < 2; white++)
        for(int chess = 0; chess < 6; chess++)
            chessImage[white][chess] = QImage(QString(":/png/%1_%2.png").arg(white?"white":"black").arg(nameList[chess]));
}

void Board::move(int pos1, int pos2)
{
    board[pos2] = board[pos1];
    board[pos1] = nullptr;
    selected = pos2;
    if(!playing)
        updateSelected(playWhite);
    if(pos1 == kingPos[currentWhite])
        kingPos[currentWhite] = pos2;
    if(pos2 == kingPos[!currentWhite])
    {
        updateChess();
        gameover = true;
        emit gameOvered(static_cast<Result>(currentWhite));
        return;
    }
    if(!checkUpdate(pos2))
        exchangeTurn();
}

void Board::swap(int pos1, int pos2)
{
    auto oldPos = pos2 > pos1? pos2 + 1: pos2 - 2;
    auto newPos = pos2 > pos1? pos2 - 1: pos2 + 1;
    board[pos2] = board[pos1];
    board[pos1] = nullptr;
    board[newPos] = board[oldPos];
    board[oldPos] = nullptr;
    kingPos[currentWhite] = pos2;
    selected = pos2;
    exchangeTurn();
}

void Board::execute(Instruction ins)
{
    switch(ins.type)
    {
    case Instruction::MOVE:
        move(ins.pos1, ins.pos2);
        break;
    case Instruction::SWAP:
        swap(ins.pos1, ins.pos2);
        break;
    case Instruction::UPDATE:
        pawnUpdate(ins.pos1, ins.pos2);
        break;
    }
    updateChess();
}

bool Board::checkUpdate(int pos)
{
    if((pos / 8 && pos / 8 != 7) || board[pos]->type != Chess::PAWN)
        return false;

    memset(mask, 0, sizeof(mask));
    mask[pos] = playing ? SELECTED: ENEMY;
    updating = true;
    selected = pos;
    updateChess();
    return true;
}

void Board::pawnUpdate(int pos, int target)
{
    board[pos]->type = static_cast<Chess::Type>(target);
    updating = false;
    updateChess();
    exchangeTurn();
}

void Board::setColor(bool white)
{
    playWhite = white;
    playing = playWhite;
    selected = -1;
    updateSelected(currentWhite);
}

void Board::exchangeTurn()
{
    // playWhite = !currentWhite;    // for debug
    currentWhite = !currentWhite;
    playing = currentWhite == playWhite;
    playingChanged(playing);

    getDangerousArea(currentWhite);
    updateMask();
    if(!danger[kingPos[currentWhite]] && checkStalemate(currentWhite))
    {
        // stalemate
        gameover = true;
        emit gameOvered(STALEMATE);
    }
}

void Board::gameStart()
{
    connect(this, &Board::operated, this, &Board::execute);
    connect(this, &Board::gameOvered, this, &Board::gameOver);
    gameover = false;
    updateChess();
}

void Board::gameOver(Board::Result winner)
{
    playing = false;
    selected = -1;
    gameover = true;
    cache = true;
    disconnect(this, &Board::operated, this, &Board::execute);
    disconnect(this, &Board::gameOvered, this, &Board::gameOver);

    qDebug() << "over" << winner;
}

inline int normalize(int n)
{
    return n ? (n > 0 ? 1 : -1) :0;
}

bool Board::checkStalemate(bool white)
{
    // only check if king is currently safe
    // check if king can be safe by moving some chess
    int _selected = selected;
    bool flag = true;
    Mask _mask[64];
    memcpy(_mask, mask, sizeof(mask));

    // check king
    selected = kingPos[white];
    memset(mask, 0, sizeof(mask));
    calculateMask(white);
    auto kingY = kingPos[white] / 8;
    auto kingX = kingPos[white] % 8;
    for(int pos = 0; pos < 64; pos++)
    {
        if(!danger[pos] && (mask[pos] == MOVABLE || mask[pos] == KILLABLE))
        {
            flag = false;
            break;
        }
    }

    // check others
    for(int pos = 0;flag && pos < 64; pos++)
    {
        auto y = pos / 8;
        auto x = pos % 8;
        auto deltaY = y - kingY;
        auto deltaX = x - kingX;

        if(board[pos] && board[pos]->white == white && pos != kingPos[white])
        {
            selected = pos;
            memset(mask, 0, sizeof(mask));
            calculateMask(white);
            // chess can't move has no contributions
            if(!availablePos.length())
                continue;

            if(deltaX && deltaY && deltaX - deltaY && deltaX + deltaY)
            {
                // chess not in the same line with king
                // if there is such chess can move, then no stalemate
                flag = false;
            }
            else
            {
                // chess in the same line with king
                // if this chess has chance to end up opponent's king, then no stalemate
                if(mask[kingPos[!white]] == KILLABLE)
                    flag = false;
                for(int factor = -1;flag && factor < 2; factor += 2)
                {
                    auto aim = (y + factor * normalize(deltaY)) * 8 + x + factor * normalize(deltaX);
                    if(illegal(aim % 8, y / 8))
                        continue;
                    // if this chess can move alone the line, no stalemate
                    if(mask[aim] == MOVABLE || mask[aim] == KILLABLE)
                    {
                        flag = false;
                        break;
                    }
                }
                // else move to any place to check if this check is essential to be here
                if(flag)
                {
                    auto aim = availablePos.first();
                    auto save = board[aim];
                    board[aim] = board[pos];
                    board[pos] = nullptr;
                    getDangerousArea(white);
                    if(!danger[kingPos[white]])
                        flag = false;
                    board[pos] = board[aim];
                    board[aim] = save;
                }
            }
        }
    }

    selected = _selected;
    memcpy(mask, _mask, sizeof(mask));
    getDangerousArea(white);
    return flag;
}

QStringList Board::save()
{
    QList<int> posSave[2][6];
    QStringList list;
    for(int pos = 0; pos < 64; pos++)
        if(board[pos])
            posSave[board[pos]->white][board[pos]->type].append(pos);

    bool player = currentWhite;
    for(int white = 0; white < 2; white++)
    {
        list.append(player ? "white" : "black");
        for(int type = 0;type < 6;type++)
        {
            auto _list = posSave[player][type];
            if(!_list.length())
                continue;
            QString s(QString("%1 %2").arg(nameList[type]).arg(posSave[player][type].length()));
            for(auto pos: posSave[player][type])
            {
                s.append(QString(" ") + posToWord(pos));
            }
            list.append(s);
        }
        player = !player;
    }
    return list;
}

void Board::getDangerousArea(bool white)
{
    int _selected = selected;
    Mask _mask[64];
    memcpy(_mask, mask, sizeof(mask));
    memset(danger, 0, sizeof(danger));

    // notice that this flag must be true before call updateSlelected
    for(int pos = 0;pos < 64; pos++)
    {
        if(board[pos] && board[pos]->white != white)
        {
            selected = pos;
            if(board[pos]->type == Chess::PAWN)
            {
                // pawn should be check specially
                auto x = pos % 8;
                auto y = pos / 8 + (white ? 1: -1);
                if(!illegal(x + 1, y))
                    danger[y * 8 + x + 1] = true;
                if(!illegal(x - 1, y))
                    danger[y * 8 + x - 1] = true;
            }
            else
            {
                memset(mask, 0, sizeof(mask));
                calculateMask(!white);
                for(int _pos = 0; _pos < 64; _pos++)
                    if(_pos != pos && mask[_pos])
                        danger[_pos] = true;
            }
        }
    }

    selected = _selected;
    memcpy(mask, _mask, sizeof(mask));
}

bool Board::executing()
{
    return !gameover;
}

void Board::forceOver(Result result)
{
    gameover = true;
    emit gameOvered(result);
}

bool Board::isPlaying()
{
    return playing;
}

bool Board::isChached()
{
    return cache;
}


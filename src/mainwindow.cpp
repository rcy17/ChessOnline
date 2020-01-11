#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "board.h"
#include "starter.h"
#include "connectwindow.h"
#include "message.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QCloseEvent>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    server(new QTcpServer),
    socket(nullptr),
    ui(new Ui::MainWindow),
    starter(nullptr),
    box(new QMessageBox(this))
{
    ui->setupUi(this);
    Board::staticInitialize();
    ui->board->loadDefaultEnd();

    countLCD[0] = ui->lcdNumber_2;
    countLCD[1] = ui->lcdNumber;
    box->setModal(false);

    ui->pushButtonAdmit->setEnabled(false);
    whitePlayer = false;
    memset(isReady, 0, sizeof(isReady));
    timer.setInterval(1000);
    connect(&timer, &QTimer::timeout, this, &MainWindow::updateTimer);
    connect(ui->board, &Board::operated, this, &MainWindow::sendOperation);
    connect(ui->board, &Board::gameOvered, this, &MainWindow::gameOver);
    connect(ui->board, &Board::playingChanged, this, &MainWindow::siwtchPlaying);
}


MainWindow::~MainWindow()
{
    delete server;
    if(connecter)
        delete connecter;
    delete ui;
}

void MainWindow::startGame(bool start)
{
    if(!start)
    {
        if(connecter)
            connecter->close();
        close();
        return;
    }
    if(connecter)
        connecter->accept();
    connecter = nullptr;
    show();
    ui->board->updateChess();
    connect(socket, &QTcpSocket::disconnected, this, &MainWindow::loseConnection);
    connect(socket, &QTcpSocket::readyRead, this, &MainWindow::receiveData);
    ui->statusBar->showMessage("please ready");
}

void MainWindow::gameOver(Board::Result winner)
{
    sendWord(Message::RESULT, winner);
    const QString str[] = {
        "The winner is black!",
        "The winner is white",
        "You made a draw!",
        "End up with stalemate!",
        "The black admits defeated!",
        "The white admits defeated!",
        "The black time out!",
        "The white time out!"
    };
    memset(isReady, 0, sizeof(isReady));
    timer.stop();
    playing = false;
    ui->pushButtonLoad->setEnabled(whitePlayer);
    ui->pushButtonAdmit->setEnabled(false);
    ui->pushButtonStart->setEnabled(true);
    updateReady();
    qDebug()<<"game over";
    box->setIcon(QMessageBox::Information);
    box->setWindowTitle("game over");
    box->setText(str[winner]);
    box->show();
}

void MainWindow::on_pushButtonSave_clicked()
{
    auto filename = QFileDialog::getSaveFileName(this, "save file", ".", "text files (*.txt);;all files (*)");
    if(!filename.length())
        return;
    QFile file(filename);
    if(!file.open(QFileDevice::WriteOnly))
    {
        QMessageBox::warning(this, "creating failed", QString("fail to create file ") + filename);
    }
    auto data = ui->board->save().join('\n').toLatin1();
    file.write(data);
    file.close();
}

void MainWindow::on_pushButtonAdmit_clicked()
{
    if(QMessageBox::warning(this, "admit", "Are you sure to admit fail?", QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No) ==
        QMessageBox::StandardButton::Yes)
    {
        auto result = whitePlayer ? Board::WHITEADMIT : Board::BLACKADMIT;
        ui->board->forceOver(result);
    }
}

void MainWindow::on_pushButtonLoad_clicked()
{
    auto name = QFileDialog::getOpenFileName(this, "load file", ".", "text files (*.txt);;all files (*)");
    if(!name.length())
    {
        return;
    }

    QFile file(name);
    if(!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::critical(this, "open failed", QString("can't open file ") + name);
        return;
    }

    sendEnd(name);

    QTextStream ts(&file);
    ui->board->loadEnd(ts);
    file.close();

    memset(isReady, 0, sizeof(isReady));
    updateReady();
}

void MainWindow::on_pushButtonStart_clicked()
{
    if(ui->board->isChached())
        ui->board->loadDefaultEnd();
    isReady[whitePlayer] = !isReady[whitePlayer];
    sendWord(Message::READY, isReady[whitePlayer]);
    updateReady();
}

void MainWindow::closeEvent(QCloseEvent * e)
{
    if(socket && socket->isOpen() && socket->isValid())
        if(QMessageBox::question(this, "exit", "Are you sure to exit(disconnection)?") != QMessageBox::StandardButton::Yes)
        {
            e->ignore();
            return;
        }
    if(starter)
        starter->show();
    if(socket)
    {
        disconnect(socket, &QTcpSocket::disconnected, this, &MainWindow::loseConnection);
        disconnect(socket, &QTcpSocket::readyRead, this, &MainWindow::receiveData);
        socket->close();
    }
}

void MainWindow::setStarter(Starter * s)
{
    starter = s;
}

void MainWindow::setHost(bool host)
{
    whitePlayer = host;
    ui->board->setColor(whitePlayer);
    if(!host)
    {
        ui->pushButtonLoad->setStyleSheet(QString("border-image: url(:/png/noload.png);"));
        ui->pushButtonLoad->setEnabled(false);
    }
}

void MainWindow::startConnect()
{
    connecter = new ConnectWindow(this);
    connecter->setHost(whitePlayer);
    connect(connecter, &ConnectWindow::connectFinished, this, &MainWindow::startGame);
    connecter->show();
}

void MainWindow::loseConnection()
{
    socket->close();
    QMessageBox::warning(this, "disconnect", QString("Lost connection with %1!").arg(whitePlayer ? "client" : "host"));
    close();
}

void MainWindow::sendOperation(Instruction ins)
{
    auto zip = Instruction::zip(ins);
    auto data = Message::pack(Message::OPERATION, zip);
    sendData(data);

}

void MainWindow::sendEnd(QString name)
{
    auto data = Message::pack(name);
    sendData(data);
}

void MainWindow::sendWord(Message::Type type, int content)
{
    auto data = Message::pack(type, content);
    sendData(data);
}

void MainWindow::sendData(const QByteArray & data)
{
    socket->write(data);
}

void MainWindow::receiveData()
{
    auto data = socket->readAll();
    loadData(data);
}

void MainWindow::loadData(const QByteArray &rawData)
{
    auto pair = Message::getSizeType(rawData);
    auto len = pair.first;
    auto type = pair.second;
    auto data = rawData.left(len);

    int content;
    switch(type)
    {
    case Message::OPERATION:
        content = Message::unpack(data);
        ui->board->execute(Instruction::parse(content));
        qDebug()<<"receive operation " << content;
        break;
    case Message::FILE:
    {
        auto result = Message::unpackFile(data);
        QTextStream ts(&result);
        ui->board->loadEnd(ts);
        memset(isReady, 0, sizeof(isReady));
        updateReady();
        ui->statusBar->showMessage("load end! click ready to start!");
        break;
    }
    case Message::RESULT:
        content = Message::unpack(data);
        if(ui->board->executing())
        {
            ui->board->forceOver(static_cast<Board::Result>(content));
        }
        qDebug()<<"receive result " << content;
        break;
    case Message::READY:
        content = Message::unpack(data);
        isReady[!whitePlayer] = content;
        updateReady();
        qDebug()<<"receive ready " << content;
        break;
    case Message::COUNTDOWN:
        countDown = Message::unpack(data);
        setOpponentCount(countDown);
        break;
    default:
        qDebug() << "receive message type" << type;
        break;
    }

    // sticky
    if(len < rawData.length())
    {
        ui->board->updateChess();
        loadData(rawData.mid(len));
    }
}

void MainWindow::showReady(QString s)
{
    const QString unready = "please ready";
    const QString ready = "waiting opponent...";
    if(!s.length())
    {
        if(isReady[0] && isReady[1])
            ui->statusBar->showMessage(playing ? "your turn": "opponent's turn");
        else
        {
            ui->statusBar->showMessage(isReady[whitePlayer] ? ready: unready);
        }
    }
}

void MainWindow::updateTimer()
{
    countDown--;
    setMyCount(countDown);
    sendWord(Message::COUNTDOWN, countDown);
    if(countDown == 0)
    {
        timer.stop();
        ui->board->forceOver(whitePlayer ? Board::WHITETIMEROUT: Board::BLACKTIMEOUT);
    }
}

void MainWindow::updateReady()
{
    QString imageName[2] = {"unready", "ready"};
    ui->pushButtonStart->setStyleSheet(QString("border-image: url(:/png/%1.png);").arg(imageName[isReady[whitePlayer]]));
    if(isReady[0] && isReady[1])
    {
        ui->pushButtonStart->setEnabled(false);
        ui->board->gameStart();
        setMyCount(maxWaitTime);
        setOpponentCount(maxWaitTime);
        countDown = maxWaitTime;
        ui->pushButtonLoad->setEnabled(false);
        ui->pushButtonAdmit->setEnabled(true);
        if(ui->board->isPlaying())
            timer.start();
    }
    showReady("");
}

void MainWindow::setOpponentCount(int value)
{
    countLCD[1]->display(value);
}

void MainWindow::setMyCount(int value)
{
    countLCD[0]->display(value);
}

void MainWindow::siwtchPlaying(bool playing)
{
    this->playing = playing;
    if(ui->board->executing() && playing)
    {
        timer.start();
    }
    else
    {
        timer.stop();
    }
    setMyCount(maxWaitTime);
    setOpponentCount(maxWaitTime);
    countDown = maxWaitTime;
    showReady("");
}

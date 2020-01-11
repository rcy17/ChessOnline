#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "board.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <QMessageBox>
#include <QTimer>
#include <QLCDNumber>
#include "message.h"

class Starter;
class ConnectWindow;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    QTcpServer * server;
    QTcpSocket * socket;
    void closeEvent(QCloseEvent *);
    void setStarter(Starter *);
    void setHost(bool host);
    void startConnect();

public slots:
    void on_pushButtonAdmit_clicked();
    void on_pushButtonLoad_clicked();
    void on_pushButtonStart_clicked();
    void on_pushButtonSave_clicked();
    void startGame(bool);
    void gameOver(Board::Result winner);
    void loseConnection();
    void sendOperation(Instruction);
    void showReady(QString);
    void updateTimer();
    void siwtchPlaying(bool playing);

private:
    Ui::MainWindow *ui;
    bool whitePlayer;
    bool isReady[2];
    Starter * starter;
    ConnectWindow * connecter;
    QTimer timer;
    QLCDNumber * countLCD[2];
    bool playing;
    QMessageBox * box;
    int countDown;
    const int maxWaitTime = 30;

    void sendData(const QByteArray & data);
    void sendEnd(QString);
    void sendWord(Message::Type type, int data);
    void sendReady(bool ready);
    void receiveData();
    void loadData(const QByteArray & data);
    void updateReady();
    void setMyCount(int value);
    void setOpponentCount(int value);

};

#endif // MAINWINDOW_H

#include "connectwindow.h"
#include "ui_connectwindow.h"
#include "mainwindow.h"
#include <QMessageBox>
#include <QAbstractSocket>

ConnectWindow::ConnectWindow(QWidget *parent) :
    QDialog(parent),
    server(dynamic_cast<MainWindow *>(parent)->server),
    socket(dynamic_cast<MainWindow *>(parent)->socket),
    ui(new Ui::ConnectWindow),
    host(false),
    connected(false)
{
    assert(parent);
    ui->setupUi(this);
    ui->lineEdit->setFocus();
}

ConnectWindow::~ConnectWindow()
{
    delete ui;
}

void ConnectWindow::setHost(bool build)
{
    ui->pushButton->setVisible(false);
    ui->buttonBox->setVisible(true);
    host = build;
    ui->label->setText("");
    if(socket)
        socket->close();
    socket = nullptr;
    if(host)
    {
        ui->lineEdit->setReadOnly(true);
        for(auto addr: QNetworkInterface::allAddresses())
        {
            qDebug()<<addr;
            if(addr.toIPv4Address() && addr != QHostAddress::LocalHost)
            {
                qDebug()<<true;
                ui->lineEdit->setText(addr.toString());
            }
        }
    }
    else
    {
        ui->lineEdit->setReadOnly(false);
        // ui->lineEdit->setText("");
    }
}

void ConnectWindow::reject()
{
    abortConnect();
    emit connectFinished(false);
    QDialog::reject();
}

void ConnectWindow::accept()
{
    if(connected)
    {
        QDialog::accept();
        return;
    }
    host ? newHost() : connectHost();
}

void ConnectWindow::connectHost()
{
    auto text = ui->lineEdit->text();
    address.setAddress(text);

    if(address.toString()!=text)
    {
        QMessageBox::warning(this, "wrong address", "please input legal ip address!");
        return;
    }
    ui->buttonBox->setVisible(false);
    ui->pushButton->setVisible(true);
    ui->label->setText("waiting...");
    socket = new QTcpSocket(server);
    socket->connectToHost(address, 11235);
    connect(socket, &QTcpSocket::connected, this, &ConnectWindow::dealResult);
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(connectFailed(QAbstractSocket::SocketError)));
    // connect(socket, &QTcpSocket::disconnected, this, &ConnectWindow::dealResult);
}

void ConnectWindow::newHost()
{
    server->listen(QHostAddress::Any, 11235);
    connect(server, &QTcpServer::newConnection, this, &ConnectWindow::acceptConnection);
    ui->buttonBox->setVisible(false);
    ui->pushButton->setVisible(true);
    ui->label->setText("waiting...");
}

void ConnectWindow::acceptConnection()
{
    // assert(!socket && "there is already a socket!");
    socket = server->nextPendingConnection();
    server->pauseAccepting();
    connected = true;
    emit connectFinished(true);
    disconnect(server, &QTcpServer::newConnection, this, &ConnectWindow::acceptConnection);
}

void ConnectWindow::abortConnect()
{
    if(server->isListening())
    {
        disconnect(server, &QTcpServer::newConnection, this, &ConnectWindow::acceptConnection);
        server->close();
    }
    setHost(host);
}

void ConnectWindow::dealResult()
{
    connected = true;
    disconnect(socket, &QTcpSocket::connected, this, &ConnectWindow::dealResult);
    disconnect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(connectFailed(QAbstractSocket::SocketError)));
    emit connectFinished(true);
}

void ConnectWindow::connectFailed(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error);
    if(!socket)
        return;
    QString s = socket->errorString();
    abortConnect();
    QMessageBox::critical(this, "connect failed", s);
}

void ConnectWindow::closeEvent(QCloseEvent * e)
{
    Q_UNUSED(e);
    abortConnect();
    reject();
}

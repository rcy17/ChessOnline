#ifndef CONNECTWINDOW_H
#define CONNECTWINDOW_H

#include <QDialog>
#include <QNetworkInterface>
#include <QTcpServer>
#include <QTcpSocket>

namespace Ui {
class ConnectWindow;
}

class ConnectWindow : public QDialog
{
    Q_OBJECT

public:
    explicit ConnectWindow(QWidget *parent = nullptr);
    ~ConnectWindow();
    void setHost(bool build);
    void accept();
    void reject();
    void closeEvent(QCloseEvent *);
    QTcpServer * & server;
    QTcpSocket * & socket;

signals:
    void connectFinished(bool);

private slots:
    void acceptConnection();
    void abortConnect();
    void dealResult();
    void connectFailed(QAbstractSocket::SocketError socketError);


private:
    Ui::ConnectWindow *ui;
    bool host;
    bool connected;
    QHostAddress address;

    void newHost();
    void connectHost();
};

#endif // CONNECTWINDOW_H

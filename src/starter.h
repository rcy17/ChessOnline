#ifndef STARTER_H
#define STARTER_H

#include <QMainWindow>
#include <QResizeEvent>

namespace Ui {
class Starter;
}

class MainWindow;

class Starter : public QMainWindow
{
    Q_OBJECT

public:
    explicit Starter(QWidget *parent = nullptr);
    ~Starter();
    void resizeEvent(QResizeEvent *);

public slots:
    void newServer();
    void joinServer();

private:
    Ui::Starter *ui;
    MainWindow * window;

};

#endif // STARTER_H

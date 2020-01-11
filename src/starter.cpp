#include "starter.h"
#include "ui_starter.h"
#include "connectwindow.h"
#include "mainwindow.h"

Starter::Starter(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Starter)
{
    ui->setupUi(this);
    window = nullptr;
}

Starter::~Starter()
{
    delete ui;
}

void Starter::resizeEvent(QResizeEvent * e)
{
    auto w = e->size().width();
    auto h = e->size().height();
    auto minner = w > h ? h : w;
    setStyleSheet("");
    setStyleSheet(QString("QPushButton{font: %1pt \"Arial\";}").arg(minner / 30));
    ui->label->setStyleSheet("");
    ui->label->setStyleSheet(QString("font: %1pt \"kaiti\";").arg(minner / 15));
}

void Starter::joinServer()
{
    if(window)
        delete window;
    window = new MainWindow;
    window->setStarter(this);
    window->setHost(false);
    window->startConnect();
    hide();
}

void Starter::newServer()
{
    if(window)
        delete window;
    window = new MainWindow;
    window->setStarter(this);
    window->setHost(true);
    window->startConnect();
    hide();
}

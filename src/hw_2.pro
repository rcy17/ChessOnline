#-------------------------------------------------
#
# Project created by QtCreator 2019-08-26T22:35:27
#
#-------------------------------------------------

QT       += core gui
QT       += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = hw_2
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        board.cpp \
        chess.cpp \
        connectwindow.cpp \
        main.cpp \
        mainwindow.cpp \
        message.cpp \
        starter.cpp

HEADERS += \
        board.h \
        chess.h \
        connectwindow.h \
        instruction.h \
        mainwindow.h \
        message.h \
        starter.h

FORMS += \
        board.ui \
        connectwindow.ui \
        mainwindow.ui \
        starter.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    end.qrc \
    png.qrc

DISTFILES +=

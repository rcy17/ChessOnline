#ifndef MESSAGE_H
#define MESSAGE_H

#include <QByteArray>
#include <QFile>
#include <QDataStream>
#include <QPair>

namespace Message
{
    enum Type
    {
        OPERATION,
        FILE,
        READY,
        COUNTDOWN,
        UPDATE,
        RESULT
    };

    QByteArray pack(Type type, int content);

    QByteArray pack(QString filename);

    int unpack(const QByteArray & data);

    QByteArray unpackFile(const QByteArray & data);

    Type getType(const QByteArray & data);

    QPair<int, Type> getSizeType(const QByteArray & data);
};

#endif // MESSAGE_H

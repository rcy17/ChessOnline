#include "message.h"
using Message::Type;

QByteArray Message::pack(Type type, int content)
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << 12 << type << content;
    return data;
}

QByteArray Message::pack(QString filename)
{
    QByteArray data;
    QDataStream s(&data, QIODevice::WriteOnly);
    QFile file(filename);
    file.open(QIODevice::ReadOnly);
    auto content = file.readAll();
    s << (content.length() + 12);
    s << FILE;
    s << content;
    file.close();
    return data;
}

int Message::unpack(const QByteArray & data)
{
    QDataStream stream(data);
    stream.skipRawData(sizeof(Type) + sizeof(int));
    int content;
    stream >> content;
    return content;
}

QByteArray Message::unpackFile(const QByteArray & data)
{
    return data.mid(2 * sizeof(int) + sizeof(Type));
}

Type Message::getType(const QByteArray & data)
{
    QDataStream stream(data);
    // notice that stream don't know sizeof type when ">>", but it knows when "<<"
    int type;
    stream >> type;
    return static_cast<Type>(type);
}

QPair<int, Type> Message::getSizeType(const QByteArray & data)
{
    QDataStream stream(data);
    // notice that stream don't know sizeof type when ">>", but it knows when "<<"
    int size;
    int type;
    stream >> size >> type;
    return QPair<int, Type>(size, static_cast<Type>(type));
}

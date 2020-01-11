#ifndef INSTRUCTION_H
#define INSTRUCTION_H
#include <QByteArray>

struct  Instruction
{
    enum Type
    {
        MOVE,
        SWAP,
        UPDATE,
    };
    Type type;
    int pos1;
    int pos2;
    static Instruction parse(int data)
    {
        auto type = static_cast<Type>(data & 15);
        auto pos1 = (data >> 4) & 63;
        auto pos2 = (data >> 10) & 63;
        return {type, pos1, pos2};
    }
    static int zip(const Instruction & ins)
    {
        int data = (ins.pos2 << 10) | (ins.pos1 << 4) | ins.type;
        return data;
    }
};



#endif // INSTRUCTION_H

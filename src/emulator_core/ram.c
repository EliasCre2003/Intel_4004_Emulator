#include "headers/ram.h"

void initializeRAM(RAM *ram)
{
    ram->index = 0;
}

void select(RAM *ram, word address)
{
    ram->chip = address >> 6;
    ram->reg = (address >> 4) & 0b11;
    ram->character = address & 0b1111;
}
void writeMain(RAM *ram, byte data)
{
    ram->index = ram->chip << 5 | ram->reg << 3 | ram->character >> 1;
    if (ram->character % 2 == 0)
    {
        data <<= 4;
        ram->memory[ram->index] &= 0x0F;
    }
    else
    {
        ram->memory[ram->index] &= 0xF0;
    }
    ram->memory[ram->index] |= data;
}
byte readMain(RAM *ram)
{
    ram->index = ram->chip << 5 | ram->reg << 3 | ram->character >> 1;
    if (ram->character % 2 == 0)
    {
        return ram->memory[ram->index] >> 4;
    }
    return ram->memory[ram->index] & 0x0F;
}
void writeStatus0(RAM *ram, byte data)
{
    ram->index = ram->chip << 3 | ram->reg << 1 | 0;
    ram->status[ram->index] &= 0x0F;
    ram->status[ram->index] |= data << 4;
}
byte readStatus0(RAM *ram)
{
    ram->index = ram->chip << 3 | ram->reg << 1 | 0;
    return ram->status[ram->index] >> 4;
}
void writeStatus1(RAM *ram, byte data)
{
    ram->index = ram->chip << 3 | ram->reg << 1 | 0;
    ram->status[ram->index] &= 0xF0;
    ram->status[ram->index] |= data;
}
byte readStatus1(RAM *ram)
{
    ram->index = ram->chip << 3 | ram->reg << 1 | 0;
    return ram->status[ram->index] & 0x0F;
}
void writeStatus2(RAM *ram, byte data)
{
    ram->index = ram->chip << 3 | ram->reg << 1 | 1;
    ram->status[ram->index] &= 0x0F;
    ram->status[ram->index] |= data << 4;
}
byte readStatus2(RAM *ram)
{
    ram->index = ram->chip << 3 | ram->reg << 1 | 1;
    return ram->status[ram->index] >> 4;
}
void writeStatus3(RAM *ram, byte data)
{
    ram->index = ram->chip << 3 | ram->reg << 1 | 1;
    ram->status[ram->index] &= 0xF0;
    ram->status[ram->index] |= data;
}
byte readStatus3(RAM *ram)
{
    ram->index = ram->chip << 3 | ram->reg << 1 | 1;
    return ram->status[ram->index] & 0x0F;
}
void writeOutput(RAM *ram, byte data)
{
    if (ram->chip % 2 == 0)
    {
        ram->outputs[ram->chip >> 1] &= 0x0F;
        ram->outputs[ram->chip >> 1] |= data << 4;
    }
    else
    {
        ram->outputs[ram->chip >> 1] &= 0xF0;
        ram->outputs[ram->chip >> 1] |= data;
    }
}
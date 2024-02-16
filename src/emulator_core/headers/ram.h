#ifndef RAM_H
#define RAM_H

#include "globals.h"

typedef struct
{
    byte memory[128];
    byte status[32];
    byte outputs[2];
    word chip : 2;
    word reg : 2;
    word character : 4;
    word index;
} RAM;

void initializeRAM(RAM *ram);
void select(RAM *ram, word address);
void writeMain(RAM *ram, byte data);
byte readMain(RAM *ram);
void writeStatus0(RAM *ram, byte data);
byte readStatus0(RAM *ram);
void writeStatus1(RAM *ram, byte data);
byte readStatus1(RAM *ram);
void writeStatus2(RAM *ram, byte data);
byte readStatus2(RAM *ram);
void writeStatus3(RAM *ram, byte data);
byte readStatus3(RAM *ram);
void writeOutput(RAM *ram, byte data);

#endif // RAM_H
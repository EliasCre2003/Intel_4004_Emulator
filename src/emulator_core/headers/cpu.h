#ifndef CPU_H
#define CPU_H

#include <stdlib.h>
#include <stdbool.h>

#include "globals.h"
#include "ram.h"
#include "rom.h"

#define CPU_RAM_BANKS_N 8
#define CPU_REGISTER_PAIRS_N 8

typedef struct
{
    RAM *ramBanks[CPU_RAM_BANKS_N];
    word registerPairs[CPU_REGISTER_PAIRS_N];
    ROM *programRom;
    uint64 C : 1;
    uint64 C0 : 1;
    uint64 TEST : 1;
    uint64 ACC : 4;
    uint64 OPA : 4;
    uint64 OPR : 4;
    uint64 PC : 12;
    uint64 STACK : 36;
    uint16 CM : 4;
    uint32 reg : 4;
    uint32 regPair : 3;
    uint32 address12 : 12;
    word address8 : 8;
    char *lastInstruction;
} CPU;

void initializeCPU(CPU *cpu, ROM *rom, uint16 startAddress, word startPage);
void step(CPU *cpu, bool debug);
void execute(CPU *cpu, int steps, bool debug);
#endif // CPU_H

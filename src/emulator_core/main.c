// @c-ignore
#include <emscripten.h>
#include <stdio.h>
#include <stdlib.h>

#include "headers/globals.h"

#include "headers/cpu.h"
#include "headers/ram.h"
#include "headers/rom.h"

CPU *cpu; // global CPU
ROM *rom; // global ROM

int main()
{
    return 0;
}

/////////////////////////
// TEMPORARY FUNCTIONS //
/////////////////////////

// EMSCRIPTEN_KEEPALIVE
// void wack()
// {
//     rom = (ROM *)malloc(sizeof(ROM));
//     cpu = (CPU *)malloc(sizeof(CPU));
// }

EMSCRIPTEN_KEEPALIVE
void stepCPU(int steps)
{
    execute(cpu, steps, true);
    // wack(cpu);
    // return cpu->lastInstruction;
}

EMSCRIPTEN_KEEPALIVE
ROM *initROM(word *program, int length)
{
    // initializeCPU(cpu, rom, 0, 0);
    if (rom == NULL)
        free(rom);
    rom = (ROM *)malloc(sizeof(ROM));
    initializeROMfromArray(rom, program, length);
    return rom;
}

EMSCRIPTEN_KEEPALIVE
CPU *resetCPU()
{
    if (cpu == NULL)
        cpu = (CPU *)malloc(sizeof(CPU));
    initializeCPU(cpu, rom, 0, 0);
    return cpu;
}

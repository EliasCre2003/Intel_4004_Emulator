#include <emscripten.h>
#include <stdio.h>
#include <stdlib.h>

#include "headers/globals.h"

#include "headers/cpu.h"
#include "headers/ram.h"
#include "headers/rom.h"

CPU *cpu; // global CPU
ROM *rom; // global ROM

int test = 1;

int main()
{

    // word testProgram[] = {
    //     0x40, 0x49, 0x73, 0x05, 0x62, 0xC0, 0x85, 0xb5, 0x1a, 0x0b, 0x64, 0xc0, 0xb5, 0x95, 0xb5, 0x1a,
    //     0x15, 0xd1, 0xb4, 0x94, 0xb4, 0xc0, 0xa0, 0x23, 0xe0, 0x50, 0x02, 0xa1, 0x23, 0xe0, 0x50, 0x02,
    //     0xc0, 0x26, 0x00, 0x25, 0xe9, 0xb7, 0xd2, 0x50, 0x06, 0x25, 0xe9, 0x87, 0xb1, 0x26, 0x00, 0x1a,
    //     0x33, 0x26, 0x01, 0xd3, 0x50, 0x0c, 0x25, 0xe9, 0xb6, 0xd2, 0x50, 0x06, 0x25, 0xe9, 0x86, 0x12,
    //     0x5d, 0x87, 0x12, 0x5d, 0xb0, 0xd1, 0x50, 0x06, 0xc0, 0x20, 0x00, 0x22, 0x00, 0x24, 0x01, 0x26,
    //     0x00, 0x50, 0x16, 0x20, 0x01, 0x50, 0x16, 0x50, 0x21, 0x50, 0x16, 0x40, 0x57, 0x40, 0x5d};

    // word testProgram[0x5e];

    // bool debug = false;
    // if (argc > 1 && *argv[1] == '1')
    // {
    //     debug = true;
    // }
    // printf("Start\n");
    // rom = (ROM *)malloc(sizeof(ROM));
    // initializeROMfromArray(rom, testProgram, 0x5e);
    // initializeROMfromFile(rom, "resources/fib_nums.bin");
    // printf("ROM initialized\n");
    // cpu = (CPU *)malloc(sizeof(CPU));
    // initializeCPU(cpu, rom, 0, 0);
    // printf("CPU initialized\n");
    // execute(cpu, 3000, debug);

    // printf("CPU executed, results: ");
    // for (int i = 0; i < 14; i++)
    // {
    // int result = cpu->ramBanks[0]->memory[i];
    //     printf("%d ", result);
    // }
    // printf("\n");
    // free(rom);
    // free(cpu);

    // rom = (ROM *)malloc(sizeof(ROM));
    // cpu = (CPU *)malloc(sizeof(CPU));

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
const char *stepCPU(int steps)
{
    execute(cpu, steps, true);
    // wack(cpu);
    // return cpu->lastInstruction;
    return cpu->lastInstruction;
}

EMSCRIPTEN_KEEPALIVE
int initROM(word *program, int length)
{
    // initializeCPU(cpu, rom, 0, 0);
    if (rom == NULL)
        rom = (ROM *)malloc(sizeof(ROM));
    return initializeROMfromArray(rom, program, length);
}

EMSCRIPTEN_KEEPALIVE
int resetCPU()
{
    if (cpu == NULL)
        cpu = (CPU *)malloc(sizeof(CPU));
    initializeCPU(cpu, rom, 0, 0);
    return 0;
}

void initialize()
{
}
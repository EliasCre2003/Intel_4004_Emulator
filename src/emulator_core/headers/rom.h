#ifndef ROM_H
#define ROM_H

#include <stdio.h>
#include "globals.h"

#define ROM_SIZE 4096  // (256 * 16)

typedef struct
{
    word memory[ROM_SIZE];
    word ioPorts : 4;
} ROM;

int initializeROMfromFile(ROM *rom, char *programPath);
int initializeROMfromArray(ROM *rom, word *data, int dataLenght);
word read(ROM *rom, uint16 address);

#endif // ROM_H
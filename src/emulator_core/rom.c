#include "headers\rom.h"

// void initializeROMfromFile(ROM *rom, char *programPath)
// {
//     FILE *file = fopen(programPath, "rb");
//     if (file == NULL) {
//         perror("Could not open file\n");
//         return;
//     }
//     fread(rom->memory, sizeof(word), 0x5E, file);
//     fclose(file);
// }

int initializeROMfromArray(ROM *rom, word *data, int dataLenght)
{
    uint16 n = dataLenght < ROM_SIZE ? dataLenght : ROM_SIZE;
    for (uint16 i = 0; i < n; i++)
    {
        rom->memory[i] = data[i] & 0xFF;
    }
    return 0;
}

word read(ROM *rom, uint16 address)
{
    return rom->memory[address];
}
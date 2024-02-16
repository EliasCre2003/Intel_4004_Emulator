#ifndef GLOBALS_H
#define GLOBALS_H

#include <emscripten.h>
#include <stdlib.h>

typedef unsigned char word;
typedef unsigned char byte;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long long uint64;

void *wasmMalloc(size_t n);
void wasmFree(void *ptr);

#endif
#include "headers/globals.h"

EMSCRIPTEN_KEEPALIVE
void *wasmMalloc(size_t n)
{
    return malloc(n);
}

EMSCRIPTEN_KEEPALIVE
void wasmFree(void *ptr)
{
    free(ptr);
}
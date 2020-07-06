#include <sgi/sgi.h>
#include <stdlib.h>

void *SGI_Malloc(size_t size)
{
    return malloc(size);
}

void SGI_Free(void *ptr)
{
    free(ptr);
}
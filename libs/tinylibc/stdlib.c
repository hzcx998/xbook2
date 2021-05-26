#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

void panic(char *m)
{
    puts(m);
    exit(-100);
}

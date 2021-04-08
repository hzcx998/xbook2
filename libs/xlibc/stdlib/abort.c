#include <stdlib.h>
#include <sys/proc.h>
#include <signal.h>
#include <stdio.h>

void abort(void)
{
    fflush(stdout);
    exit(1);
}

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>

int main(int argc, char **argv)
{
    walltime_t wt;
    walltime(&wt);
    struct tm tm;
    walltime_switch(&wt, &tm);
    printf("%s\n", asctime(&tm));
    return 0;
}

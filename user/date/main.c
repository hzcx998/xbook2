#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>

int main(int argc, char **argv)
{
    ktime_t ktm;
    ktime(&ktm);
    struct tm tm;
    ktimeto(&ktm, &tm);
    printf("%s\n", asctime(&tm));
    return 0;
}

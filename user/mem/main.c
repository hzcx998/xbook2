#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/vmm.h>
#include <const.h>

int main(int argc, char **argv)
{
    if (argc > 1) {
        printf("free: no arguments support!\n");
        return -1;
    }
    mstate_t ms;
    mstate(&ms);
    printf("          TOTAL           USED           FREE\n");
    printf("%14dB%14dB%14dB\n", ms.ms_total, ms.ms_used, ms.ms_free);
    printf("%14dM%14dM%14dM\n", ms.ms_total / MB, ms.ms_used / MB, ms.ms_free / MB);
    return 0;
}

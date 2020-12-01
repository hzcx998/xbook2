#include <stdlib.h>
#include <sys/proc.h>
#include <signal.h>
#include <stdio.h>

void abort(void)
{
    signal(SIGABRT, SIG_UNBLOCK);
    signal(SIGABRT, SIG_DFL);
    fflush(NULL);
    raise(SIGABRT);
    exit(1);    /* shoult never here. */
}

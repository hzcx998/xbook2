#include <stdlib.h>
#include <sys/trigger.h>
#include <sys/proc.h>

void abort(void)
{
    triggeron(TRIGHSOFT, getpid());
}

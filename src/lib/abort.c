#include <stdlib.h>
#include <xbook/debug.h>

void abort(void)
{
    spin("abort");
}

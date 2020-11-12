#include <stdlib.h>
#include <sys/exception.h>
#include <sys/proc.h>

void abort(void)
{
    expraise(EXP_CODE_ABORT, 0);
}

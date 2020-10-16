#include <exit.h>
#include <sys/proc.h>
#include <stddef.h>

/* only flush output buffers when necessary */
int (*_clean)(void) = NULL;

extern void _exit_cleanup();

void exit(int status)
{
    if (_clean) _clean();
    _exit_cleanup();
    _exit(status);
}
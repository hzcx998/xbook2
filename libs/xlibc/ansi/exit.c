#include <exit.h>
#include <sys/proc.h>
#include <stddef.h>
#include <unistd.h>

extern void _exit_cleanup();

void exit(int status)
{
    _exit_cleanup();
    _exit(status);
}
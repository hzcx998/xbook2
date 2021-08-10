#define _GNU_SOURCE
#include <unistd.h>
#include <limits.h>

int getpagesize(void)
{
	return PAGE_SIZE;
}

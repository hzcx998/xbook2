#include "test.h"

#define STDOUT  STDOUT_FILENO

int dup2_test(int argc, char* argv[])
{
    int fd = dup2(STDOUT, 100);
	assert(fd != -1);
	const char *str = "  from fd 100\n";
	write(100, str, strlen(str));
    return 0;
}

#include "test.h"

#define STDOUT  STDOUT_FILENO

int dup_test(int argc, char* argv[])
{
    int fd = dup(STDOUT);
	assert(fd >=0);
	printf("  new fd is %d.\n", fd);
    return 0;
}

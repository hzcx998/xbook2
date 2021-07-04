#include "test.h"
#include <sys/stat.h>

int test_file(int argc, char *argv[])
{
    mode_t old;
    printf("%s: %x\n", $(umask), old = umask(0));
    printf("%s: %x\n", $(umask), old = umask(old));
    printf("%s: %x\n", $(umask), old = umask(0123456));
    printf("%s: %x\n", $(umask), old = umask(0111));
    printf("%s: %x\n", $(umask), old = umask(0));
    printf("%s: %d\n", $(readahead), readahead(0, 0, 0));
    return 0;
}

#include "test.h"
#include <sys/stat.h>
#include <sys/uio.h>

int test_file(int argc, char *argv[])
{
    mode_t old;
    printf("%s: %x\n", $(umask), old = umask(0));
    printf("%s: %x\n", $(umask), old = umask(old));
    printf("%s: %x\n", $(umask), old = umask(0123456));
    printf("%s: %x\n", $(umask), old = umask(0111));
    printf("%s: %x\n", $(umask), old = umask(0));
    printf("%s: %d\n", $(readahead), readahead(0, 0, 0));
    printf("%s: %d\n", $(renameat), renameat(AT_FDCWD, "/bin/ts", AT_FDCWD, "/bin/ts2"));
    printf("%s: %d\n", $(fchmod), fchmod(0, 0606));
    
    char *str0 = "hello ";
    char *str1 = "world\n";
    struct iovec iov[2];
    ssize_t nwritten;

    iov[0].iov_base = str0;
    iov[0].iov_len = strlen(str0);
    iov[1].iov_base = str1;
    iov[1].iov_len = strlen(str1);

    nwritten = writev(1, iov, 2);
    printf("%s: %d\n", $(writev), nwritten);

    return 0;
}

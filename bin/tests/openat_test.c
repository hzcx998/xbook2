#include "test.h"

int openat_test(int argc, char *argv[])
{
    printf("openat test\n");

    int fd = openat(123, "/bin/tests", O_RDONLY, 0);
    if (fd < 0) {
        printf("openat abosule path error\n");
        return -1;
    }
    char buf[32];
    if (read(fd, buf, 32) < 0) {
        printf("read file error\n");
    }
    close(fd);

    fd = openat(AT_FDCWD, "bin/tests", O_RDONLY, 0);
    if (fd < 0) {
        printf("openat AT_FDCWD path error\n");
        return -1;
    }
    if (read(fd, buf, 32) < 0) {
        printf("read file error\n");
    }
    close(fd);

    /* open dir */
    int dirfd = open("/bin", O_DIRECTORY);
    if (dirfd < 0) {
        printf("open O_DIRECTORY path error\n");
        return -1;
    }
    fd = openat(dirfd, "tests", O_RDONLY, 0);
    if (fd < 0) {
        printf("openat dirfd path error\n");
        return -1;
    }
    if (read(fd, buf, 32) < 0) {
        printf("read file error\n");
    }
    assert(!close(dirfd));
    assert(!close(fd));

    dirfd = open("/sbin/", O_DIRECTORY);
    if (dirfd < 0) {
        printf("open O_DIRECTORY path error\n");
        return -1;
    }
    fd = openat(dirfd, "init", O_RDONLY, 0);
    if (fd < 0) {
        printf("openat dirfd path error\n");
        return -1;
    }
    if (read(fd, buf, 32) < 0) {
        printf("read file error\n");
    }
    assert(!close(dirfd));
    assert(!close(fd));

    printf("openat test pass\n");
    return 0;
}

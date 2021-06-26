#include "test.h"
#include <sys/xattr.h>

int test_xattr(int argc, char *argv[])
{
    printf("%s: %d\n", $(removexattr), removexattr("x", "y"));
    printf("%s: %d\n", $(lremovexattr), lremovexattr("x", "y"));
    printf("%s: %d\n", $(fremovexattr), fremovexattr(0, "y"));
    printf("%s: %d\n", $(setxattr), setxattr("x", "y", NULL, 0, 0));
    printf("%s: %d\n", $(lsetxattr), lsetxattr("x", "y", NULL, 0, 0));
    printf("%s: %d\n", $(fsetxattr), fsetxattr(0, "y", NULL, 0, 0));
    printf("%s: %d\n", $(getxattr), getxattr("x", "y", NULL, 0));
    printf("%s: %d\n", $(lgetxattr), lgetxattr("x", "y", NULL, 0));
    printf("%s: %d\n", $(fgetxattr), fgetxattr(0, "y", NULL, 0));
    printf("%s: %d\n", $(listxattr), listxattr("x", "y", 0));
    printf("%s: %d\n", $(llistxattr), llistxattr("x", "y", 0));
    printf("%s: %d\n", $(flistxattr), flistxattr(0, "y", 0));
    return 0;
}

#include "test.h"
#include <sys/utsname.h>

int test_misc(int argc, char *argv[])
{
    struct utsname un;
    printf("%s: %d\n", $(uname), uname(&un));
    printf("Uname: %s %s %s %s %s %s\n", 
		un.sysname, un.nodename, un.release, un.version, un.machine, un.domainname);

    return 0;
}

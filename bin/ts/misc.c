#include "test.h"
#include <sys/utsname.h>

int test_misc(int argc, char *argv[])
{
    struct utsname un;
    printf("%s: %d\n", $(uname), uname(&un));
    printf("Uname: %s %s %s %s %s %s\n", 
		un.sysname, un.nodename, un.release, un.version, un.machine, un.domainname);

    char hostname[64] = {0};
    printf("%s: %d\n", $(gethostname), gethostname(hostname, 64));
    printf("hostname: %s\n", hostname);
    printf("%s: %d\n", $(sethostname), sethostname("xbook not linux", strlen("xbook not linux")));
    memset(hostname, 0, 64);
    printf("%s: %d\n", $(gethostname), gethostname(hostname, 64));
    printf("hostname: %s\n", hostname);

    return 0;
}

#include "test.h"
#include <sys/utsname.h>
#include <sys/resource.h>

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

    struct rlimit rlim, orlim;
    printf("%s: %d\n", $(getrlimit), getrlimit(RLIMIT_CPU, &rlim));
    printf("%s: %d\n", $(setrlimit), setrlimit(RLIMIT_CPU, &rlim));
    printf("%s: %d\n", $(getrlimit), getrlimit(-1, &rlim));
    printf("%s: %d\n", $(setrlimit), setrlimit(50, &rlim));
    printf("%s: %d\n", $(getrlimit), getrlimit(RLIMIT_CPU, NULL));
    printf("%s: %d\n", $(setrlimit), setrlimit(RLIMIT_CPU, NULL));
    
    printf("%s: %d\n", $(prlimit), prlimit(0, RLIMIT_CPU, NULL, NULL));
    printf("%s: %d\n", $(prlimit), prlimit(0, RLIMIT_CPU, &rlim, &orlim));
    printf("%s: %d\n", $(prlimit), prlimit(4, RLIMIT_CPU, &rlim, &orlim));
    printf("%s: %d\n", $(prlimit), prlimit(-1, RLIMIT_CPU, &rlim, &orlim));
    
    return 0;
}

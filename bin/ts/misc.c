#define _GNU_SOURCE

#include "test.h"
#include <sys/utsname.h>
#include <sys/resource.h>
#include <sys/prctl.h>

extern char **_environ;
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
    
    printf("%s: %s: %d\n", $(prctl), $(PR_CAP_AMBIENT), prctl(PR_CAP_AMBIENT, 0));
    printf("%s: %s: 0: %d\n", $(prctl), $(PR_SET_NO_NEW_PRIVS), prctl(PR_SET_NO_NEW_PRIVS, 0));
    printf("%s: %s: 1: %d\n", $(prctl), $(PR_SET_NO_NEW_PRIVS), prctl(PR_SET_NO_NEW_PRIVS, 1));
    printf("%s: %d\n", $(exec), execve("/bin/ps", NULL, NULL));
    printf("%s: %s: %d\n", $(prctl), $(PR_GET_NO_NEW_PRIVS), prctl(PR_GET_NO_NEW_PRIVS));
    printf("%s: %s: 0: %d\n", $(prctl), $(PR_SET_NO_NEW_PRIVS), prctl(PR_SET_NO_NEW_PRIVS, 0));
    
    printf("%s: %s: 0: %d\n", $(prctl), $(PR_SET_PDEATHSIG), prctl(PR_SET_PDEATHSIG, 0));
    printf("%s: %s: 1: %d\n", $(prctl), $(PR_SET_PDEATHSIG), prctl(PR_SET_PDEATHSIG, 1));
    printf("%s: %s: 0: %d\n", $(prctl), $(PR_SET_PDEATHSIG), prctl(PR_SET_PDEATHSIG, 0));
    #if 0
    if (!fork()) {
        printf("child %s: %s: SIGTERM: %d\n", $(prctl), $(PR_SET_PDEATHSIG), prctl(PR_SET_PDEATHSIG, SIGTERM));
        printf("child %s: %s: %d\n", $(prctl), $(PR_GET_PDEATHSIG), prctl(PR_GET_PDEATHSIG));
        while (1)
        {
            /* code */
        }        
    } else {
        printf("parent sleep 3s\n");
        sleep(3);
        printf("parent exit\n");
        exit(1);
    }
    #endif
    char **p = argv;
    int i = 0;
    while (p[i]) {
        printf("arg: %s\n", p[i]);
        i++;
    }
    char *argv_[3] = {"/bin/ts", "env", 0};
    char *envp_[3] = {"/bin", "/sbin", 0};
    pid_t pid = fork();
    if (!pid) {
        printf("child exec\n");
        printf("%s: %d\n", $(execve), execve(argv_[0], argv_, envp_));
        printf("child exec failed!\n");
    } else {
        wait(NULL);
    }
    return 0;
}

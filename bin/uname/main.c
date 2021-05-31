#include <stdio.h>
#include <unistd.h>

#if defined(__TINYLIBC__)
#define _HAS_UNAME
#elif defined(__XLIBC__) 
#include <sys/proc.h>
#include <sys/sys.h>
#endif

#ifdef _HAS_UNAME
struct utsname {
	char sysname[65];
	char nodename[65];
	char release[65];
	char version[65];
	char machine[65];
	char domainname[65];
};
struct utsname un;
#endif

int main(int argc, char *argv[])
{
    #ifdef _HAS_UNAME
    int test_ret = uname(&un);
	printf("Uname: %s %s %s %s %s %s\n", 
		un.sysname, un.nodename, un.release, un.version, un.machine, un.domainname);
    #else
	char buf[SYS_VER_LEN] = {0};
    getver(buf, SYS_VER_LEN);
    printf("%s\n",buf);
    #endif
    return 0;
}

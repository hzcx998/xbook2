#include <stdio.h>
#include <sys/proc.h>
#include <sys/sys.h>

int cmd_ver(int argc, char **argv)
{
	char buf[SYS_VER_LEN] = {0};
    getver(buf, SYS_VER_LEN);
    printf("%s\n",buf);
    return 0;
}

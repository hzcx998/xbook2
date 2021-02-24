#include <unistd.h>
#include <string.h>

int gethostname(char *name, size_t len)
{
    if (!name)
        return -1;
    /* FIXME: get host name from kernel */
    char *hostnm = "xbook";
    strncpy(name, hostnm, min(strlen(hostnm), len));
    return 0;
}
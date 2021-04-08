#include "test.h"
#include <sys/sys.h>
int perm_test(int argc, char *argv[])
{
    printf("Perm test\n");
    int fd = open("/dev/disk0", 0);
    printf("open %d\n", fd);
    
    int retval = login("jason", "1234");
    printf("login %d\n", retval);

    retval = register_account("zhuyu", "1234");
    printf("register %d\n", retval);

    retval = login("zhuyu", "1234");
    printf("login %d\n", retval);

    retval = logout("zhuyu");
    printf("logout %d\n", retval);

    retval = unregister_account("zhuyu");
    printf("unregister %d\n", retval);

    retval = login("admin", "1234");
    printf("admin login %d\n", retval);
    
    retval = unregister_account("zhuyu");
    printf("unregister %d\n", retval);

    return 0;
}

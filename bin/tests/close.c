#include "test.h"

int close_test(int argc, char* argv[])
{
    int fd = open("test_close.txt", O_CREATE | O_RDWR);
    //assert(fd > 0);
    const char *str = "  close error.\n";
    int str_len = strlen(str);
    //assert(write(fd, str, str_len) == str_len);
    write(fd, str, str_len);
    int rt = close(fd);	
    assert(rt == 0);
    printf("  close %d success.\n", fd);
	
    return 0;
}
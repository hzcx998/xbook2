#include "test.h"

/* 测试 unlink
 * 测试通过，应有如下输出：
 * "  unlink success!"
 * 测试失败，应有如下输出：
 * "  unlink error!"
 */

static int test_unlink()
{
    TEST_START(__func__);

    char *fname = "./test_unlink";
    int fd, ret;

    fd = open(fname, O_CREATE | O_WRONLY);
    assert(fd > 0);
    close(fd);

    // unlink test
    ret = unlink(fname);
    assert(ret == 0);
    fd = open(fname, O_RDONLY);
    if(fd < 0){
        printf("  unlink success!\n");
    }else{
	printf("  unlink error!\n");
        close(fd);
    }
    // It's Ok if you don't delete the inode and data blocks.

    TEST_END(__func__);
}

int unlink_test(int argc, char* argv[])
{
    test_unlink();
    return 0;
}

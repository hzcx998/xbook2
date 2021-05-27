#include "test.h"

static struct kstat kst;
int fstat_test(int argc, char* argv[])
{
    TEST_START(__func__);
	//int fd = open("./text.txt", 0);
	int fd = open("./text.txt", O_CREAT | O_RDONLY);
	assert(fd >= 0);
    int ret = fstat(fd, &kst);
	printf("fstat ret: %d\n", ret);
	assert(ret >= 0);

	printf("fstat: dev: %d, inode: %d, mode: %d, nlink: %d, size: %d, atime: %d, mtime: %d, ctime: %d\n",
	      kst.st_dev, kst.st_ino, kst.st_mode, kst.st_nlink, kst.st_size, kst.st_atime_sec, kst.st_mtime_sec, kst.st_ctime_sec);

	TEST_END(__func__);
    return 0;
}

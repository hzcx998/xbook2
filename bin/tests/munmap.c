#include "test.h"
#include <sys/mman.h>

/*
 * 测试成功时输出：
 * "  Hello, mmap success"
 * 测试失败时输出：
 * "mmap error."
 */
static struct kstat kst;
int munmap_test(int argc, char* argv[])
{
    TEST_START(__func__);
    char *array;
    const char *str = "  Hello, mmap successfully!";
    int fd;

    fd = open("test_mmap.txt", O_RDWR | O_CREATE);
    write(fd, str, strlen(str));
    fstat(fd, &kst);
    printf("file len: %d\n", kst.st_size);
    array = mmap(NULL, kst.st_size, PROT_WRITE | PROT_READ, MAP_FILE | MAP_SHARED, fd, 0);
    //printf("return array: %x\n", array);

    if (array == MAP_FAILED) {
	printf("mmap error.\n");
    }else{
	//printf("mmap content: %s\n", array);

    	int ret = munmap(array, kst.st_size);
	printf("munmap return: %d\n",ret);
	assert(ret == 0);

	if (ret == 0)
		printf("munmap successfully!\n");
    }
    close(fd);

    TEST_END(__func__);
    return 0;
}
#include "test.h"
#include <sys/time.h>

static int64 get_time()
{
    struct timeval time;
    int err = gettimeofday(&time, 0);
    if (err == 0)
    {
        return ((time.tv_sec & 0xffff) * 1000 + time.tv_usec / 1000);
    }
    else
    {
        return -1;
    }
}

/*
 * 测试通过时的输出：
 * "gettimeofday success."
 * "start:[num], end:[num]"
 * "interval: [num]"	注：数字[num]的值应大于0
 * 测试失败时的输出：
 * "gettimeofday error."
 */
int gettimeofday_test(int argc, char* argv[])
{
    TEST_START(__func__);
	int test_ret1 = get_time();
	volatile int i = 12500000;	// qemu时钟频率12500000
	while(i > 0) i--;
	int test_ret2 = get_time();
	if(test_ret1 > 0 && test_ret2 > 0){
		printf("gettimeofday success.\n");
		printf("start:%d, end:%d\n", test_ret1, test_ret2);
                printf("interval: %d\n", test_ret2 - test_ret1);
	}else{
		printf("gettimeofday error.\n");
	}
	TEST_END(__func__);
    return 0;
}

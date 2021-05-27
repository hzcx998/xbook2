#include "test.h"

int sleep_test(int argc, char *argv[])
{
    printf("sleep 3 s.");
    sleep(1);
    
    printf("usleep1.");
    usleep(2000);
    printf("usleep2.");
    if (usleep(3000000) < 0)
        perror("sleep err:");

    printf("usleep done.");
    return 0;
}

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
 * "sleep success."
 * 测试失败时的输出：
 * "sleep error."
 */
int sleep_test2(int argc, char *argv[])
{
	TEST_START(__func__);

	int time1 = get_time();
	assert(time1 >= 0);
	int ret = sleep(1);
	assert(ret == 0);
	int time2 = get_time();
	assert(time2 >= 0);

	if(time2 - time1 >= 1){	
		printf("sleep success.\n");
	}else{
		printf("sleep error.\n");
	}
	TEST_END(__func__);
}

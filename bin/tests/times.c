#include "test.h"

#include <sys/times.h>

struct tms mytimes;

int times_test(int argc, char *argv[])
{
    TEST_START(__func__);

	int test_ret = times(&mytimes);
	assert(test_ret >= 0);

	printf("mytimes success\n{tms_utime:%d, tms_stime:%d, tms_cutime:%d, tms_cstime:%d}\n",
		mytimes.tms_utime, mytimes.tms_stime, mytimes.tms_cutime, mytimes.tms_cstime);
	TEST_END(__func__);
    return 0;
}

#include "test.h"

#include <sched.h>

int test_sched(int argc, char *argv[])
{
    cpu_set_t set;
    CPU_ZERO(&set);
    int err = sched_getaffinity(0, sizeof(set), &set);
    if (err) {
        printf("warning: cound not get thread affinity, continuing... %d\n", err);
    }
    int i;
    for (i = 0; i < 2; i++)
    {
        if (CPU_ISSET(i, &set))//判断线程与哪个CPU有亲和力
        {
            printf("this thread %d is running processor : %d\n", i,i);
        }
    }
    err = sched_setaffinity(getpid(), sizeof(set), &set);
    if (err) {
        printf("warning: cound not set thread affinity, continuing...\n");
    }
    err = sched_getaffinity(0, sizeof(set), &set);
    if (err) {
        printf("warning: cound not get thread affinity, continuing... %d\n", err);
    }
    for (i = 0; i < 2; i++)
    {
        if (CPU_ISSET(i, &set))//判断线程与哪个CPU有亲和力
        {
            printf("this thread %d is running processor : %d\n", i,i);
        }
    }

    printf("sched_get_priority_max: SCHED_FIFO %d\n", sched_get_priority_max(SCHED_FIFO));
    printf("sched_get_priority_max: SCHED_RR %d\n", sched_get_priority_max(SCHED_RR));
    printf("sched_get_priority_max: SCHED_OTHER %d\n", sched_get_priority_max(SCHED_OTHER));
    printf("sched_get_priority_max: SCHED_IDLE %d\n", sched_get_priority_max(SCHED_IDLE));
    printf("sched_get_priority_max: SCHED_DEADLINE %d\n", sched_get_priority_max(SCHED_DEADLINE));

    printf("sched_get_priority_min: SCHED_FIFO %d\n", sched_get_priority_min(SCHED_FIFO));
    printf("sched_get_priority_min: SCHED_RR %d\n", sched_get_priority_min(SCHED_RR));
    printf("sched_get_priority_min: SCHED_OTHER %d\n", sched_get_priority_min(SCHED_OTHER));
    printf("sched_get_priority_min: SCHED_IDLE %d\n", sched_get_priority_min(SCHED_IDLE));
    printf("sched_get_priority_min: SCHED_DEADLINE %d\n", sched_get_priority_min(SCHED_DEADLINE));

    return 0;
}

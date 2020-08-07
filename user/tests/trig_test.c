#include "test.h"

#include <sys/trigger.h>

void trig_handler(int trig)
{
    printf("trig: %d handled!\n", trig);
}

void alarm_handler(int trig)
{
    printf("alarm: handled\n");
}
int trig_test(int argc, char *argv[])
{
    printf("----trig test----\n");

    /* 忽略信号 */
    //trigger(TRIGLSOFT, TRIG_IGN);

    trig_action_t ta, oldta;
    ta.handler = trig_handler;
    ta.mask = 0;
    ta.flags = TA_ONCSHOT;
    
    trigger_action(TRIGLSOFT, &ta, &oldta);

    printf("old ta: %x\n", oldta.handler);

    triggeron(TRIGLSOFT, getpid());

    trigset_t set;
    trigaddset(&set, TRIGLSOFT);
    trigprocmask(TRIG_BLOCK, &set, NULL);

    trigger(TRIGALARM, alarm_handler);
    alarm(3);

    sleep(5);
    
    trigprocmask(TRIG_UNBLOCK, &set, NULL);

    while (1)
    {
        usleep(500*1000);
        trigpending(&set);
        printf("trig set: %x\n", set);
    }
    
    return 0;
}
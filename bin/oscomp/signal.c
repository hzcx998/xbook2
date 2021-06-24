#include "test.h"

static void mem_signal()
{
    char *q = (char *)0x1001;
    *q = 100;
    *q = *q;
    char *v = (char *)0xf0000000;
    *v = 100;
}
static int occur = 0;
static int alarm_count = 0;
static void user_signal_handler(int signo)
{
    printf("in user handler\n");
    occur = 1;
    alarm_count++;
}

extern void __restore_rt();
static void user_signal()
{
    struct sigaction act, oact;
    act.sa_flags = SA_RESTORER;
    act.sa_handler = user_signal_handler;
    sigfillset(&act.sa_mask);
    act.sa_restorer = __restore_rt;
    int err = rt_sigaction(SIGALRM, &act, &oact, sizeof(struct sigaction));
    printf("err: %d\n", err);
    printf("old set: %x %x %x\n", oact.sa_flags, oact.sa_handler, oact.sa_restorer);
    alarm(1);
    while (1)
    {
        if (occur) {
            printf("set alrarm again %d\n", alarm_count);
            occur = 0;        
            alarm(1);
        }
    }
}

int oscamp_signal(int argc, char *argv[])
{
    // mem_signal();
    user_signal();
    return 0;
}

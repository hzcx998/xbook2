#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/proc.h>

static const char *proc_print_status[] = {
    "READY",
    "RUNNING",
    "BLOCKED",
    "WAITING",
    "STOPED",
    "ZOMBIE",
    "DIED"
};

int main(int argc, char **argv)
{
    tstate_t ts;
    int num = 0;
    
    int all = 0;

    if (argc > 1) {
        char *p = (char *)argv[1];
        if (*p == '-') {
            p++;
            switch (*p)
            {
            case 'a':   /* 显示所有信息 */
                all = 1;
                break;
            case 'h':   /* 显示帮助信息 */
                printf("Usage: ps [option]\n");
                printf("Option:\n");
                printf("  -a    Print all tasks. Example: ps -a \n");
                printf("  -h    Get help of ps. Example: ps -h \n");
                printf("Note: If no arguments, only print user process.\n");
                return 0;
            default:
                printf("ps: unknown option!\n");
                return -1;
            }
        } else {
            printf("ps: unknown argument!\n");
            return -1;
        }
    }

    printf("   PID   PPID   PGID     STAT    PRO      TICKS    NAME\n");
    while (!tstate(&ts, &num)) {
        /* 如果没有全部标志，就只显示用户进程。也就是ppid不为-1的进程 */
        if (!all) {
            if (ts.ts_ppid == -1)
                continue;
        }
        printf("%6d %6d %6d %8s %6d %10d    %s\n", 
            ts.ts_pid, ts.ts_ppid, ts.ts_pgid, proc_print_status[(unsigned char) ts.ts_state], ts.ts_priority,
            ts.ts_runticks, ts.ts_name);
    }
    return 0;
}

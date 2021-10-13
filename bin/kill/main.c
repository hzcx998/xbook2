#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <ctype.h>
#include <string.h>

/**
 * kill命令
 * 1.命令格式：
 * kill [参数] [进程号]
 * 2.命令功能：
 * 发送指定的信号到相应进程。不指定型号将发送SIGTERM（15）终止指定进程。
 * 3.命令参数：
 * -l  信号，若果不加信号的编号参数，则使用“-l”参数会列出全部的信号名称
 * -s  指定发送信号
 * 
 * 4.举例：
 * kill 10          // 默认方式杀死进程10
 * kill -9 10       // 用SIGKILL杀死进程10
 * kill -SIGKILL 10 // 用SIGKILL杀死进程10
 * kill -KILL 10    // 用SIGKILL杀死进程10
 * kill -l          // 列出所有信号
 * kill -l KILL     // 列出KILL对应的信号值
 * kill -l SIGKILL  // 列出KILL对应的信号值
 * 就实现以上命令，足矣。
 * 
 */


#define MAX_SUPPORT_SIG_NR      32

/* 信号的字符串列表，用于查找信号对应的信号值 */
char *signal_table[MAX_SUPPORT_SIG_NR][2] = {
    {"NULL", "NULL"},       
    {"SIGUSR1", "USR1"},       
    {"SIGTINT", "TTIN"},       
    {"SIGKILL", "KILL"},       
    {"SIGTRAP", "TRAP"},  
    {"SIGABRT", "ABRT"},    /* 5 */       
    {"SIGBUS", "BUS"},       
    {"SIGSEGV", "SEGV"},       
    {"SIGFPE", "FPE"},       
    {"SIGILL", "ILL"},         
    {"SIGPIPE", "PIPE"},    /* 10 */    
    {"SIGSTKFLT", "STKFLT"},       
    {"SIGALRM", "ALRM"},       
    {"SIGTERM", "TERM"},       
    {"SIGCHLD", "CHLD"},       
    {"SIGCONT", "CONT"},    /* 15 */
    {"SIGSTOP", "STOP"},       
    {"SIGTTIN", "TTIN"},       
    {"SIGTTOU", "TTOU"},       
    {"SIGSYS", "SYS"},       
    {"SIGIO", "IO"},        /* 20 */
    {"SIGHUP", "HUP"},     
    {"SIGWINCH", "WINCH"},       
    {"SIGVTALRM", "VTALRM"},       
    {"SIGPROF", "PROF"},       
    {"SIGQUIT", "QUIT"},       
};

/**
 * find_signal_in_table - 在表中查找信号
 * @name：信号名称
 * 
 * 
 * 找到返回信号值，没找到返回0
 */
int find_signal_in_table(char *name)
{
    int idx;
    int signo = -1;
    
    for (idx = 1; idx < MAX_SUPPORT_SIG_NR; idx++) {
        /* 字符串相同就找到 */
        if (!strcmp(signal_table[idx][0], name) || !strcmp(signal_table[idx][1], name)) {
            signo = idx;
            break;
        }
    }

    /* 找到就返回信号 */
    if (signo != -1) {
        return signo;
    }
    return 0;
}

#define D(v) printf("%s:%d\n", #v, v)

static void dump(void)
{
    D(SIGINT);
    D(SIGILL);
    D(SIGTRAP);
    D(SIGABRT);
    D(SIGBUS);
    D(SIGFPE);
    D(SIGKILL);
    D(SIGUSR1);
    D(SIGSEGV);
    D(SIGPIPE);
    D(SIGALRM);
    D(SIGTERM);
    D(SIGSTKFLT);
    D(SIGCHLD);
    D(SIGCONT);
    D(SIGSTOP);
    D(SIGTSTP);
    D(SIGTTIN);
    D(SIGTTOU);
    D(SIGSYS);
    D(SIGIO);
    D(SIGHUP);
    D(SIGWINCH);
    D(SIGVTALRM);
    D(SIGPROF);
    D(SIGQUIT);
}

int main(int argc, char *argv[])
{
    dump();
	if(argc < 2){
		printf("\nusage: kill [-l | -signo | -signame] [pid]\n");
        return 0;
	}

	//默认 argv[1]是进程的pid
	
    int signo = SIGTERM;    /* 默认是TERM信号，如果没有指定信号的话 */
    int pid = -1;

    char *p;

    /* 不列出信号 */
    bool list_signal = false;
    /* 是否有可选参数 */
    bool has_option = false;    

    bool pid_negative = false;    /* pid为负数 */

    /* 找出pid和signo */
    int idx = 1;

    /* 扫描查看是否有可选参数 */
    while (idx < argc) {
        /* 有可选参数，并且是在第一个参数的位置 */
        if (argv[idx][0] == '-' && argv[idx][1] != '\0' && idx == 1) {
            has_option = true;
            p = (char *)(argv[idx] + 1);
            /* 查看是什么选项 */

            if (*p == 'l' && p[1] == '\0') {    /* 是 -l选项 */
                list_signal = true;     /* 需要列出信号 */
            } else {
                /* 是纯数字，即 -12 */
                if (isdigitstr((const char *)p)) {
                    /* 转换成数值 */
                    signo = atoi(p);

                    if (1 > signo || signo >= MAX_SUPPORT_SIG_NR) {
                        printf("kill: signal %s number not support!\n", signo);
                        return -1;
                    }

                } else { /* 不是是纯数字，即 -KILL，但也可能找不到 */
                    signo = find_signal_in_table(p);

                    /* 找到信号 */
                    if (signo <= 0) {
                        
                        printf("kill: signal %s not found!\n", p);
                        return -1;
                    }
                }
            }
        } else if (argv[idx][0] != '\0' && idx == 1) {  /* 还是第一个参数，是没有选项的状态 */
            p = (char *)(argv[idx]);

            /* 此时就是进程pid */

            /* 是纯数字，即 -12 */
            if (*p == '-') {    /* 可能是负数 */
                pid_negative = true;
                p++;
            }

            if (isdigitstr((const char *)p)) {
                /* 转换成数值 */
                pid = atoi(p);
                /* 如果是负的，就要进行负数处理 */
                if (pid_negative)
                    pid = -pid;

            } else {
                printf("kill: process id %s error!\n", p);
                return -1;
            }
        } else if (argv[idx][0] != '\0' && idx == 2) {  /* 第二个参数 */
            p = (char *)(argv[idx]);

            /* 
            如果是纯数字，就是进程id。
            如果是字符串，就是信号值
             */
            if (list_signal) {

                /* 是列出信号，就直接解析信号值 */
                signo = find_signal_in_table(p);
                
                /* 找到信号 */
                if (signo <= 0) {
                    printf("kill: signal %s not found!\n", p);
                    return -1;
                }
                

            } else {
                /* 是纯数字，即 -12 */
                if (*p == '-') {    /* 可能是负数 */
                    pid_negative = true;
                    p++;
                }

                if (isdigitstr((const char *)p)) {
                    /* 转换成数值 */
                    pid = atoi(p);
                    /* 如果是负的，就要进行负数处理 */
                    if (pid_negative)
                        pid = -pid;
                } else {
                    printf("kill: process id %s must be number!\n", p);
                    return -1;
                }
            }
        }
        idx++;
    }

    if (has_option) {
        /* 是列出信号 */
        if (list_signal) {
            if (argc == 2) {
                /* kill -l # 列出所有 */
                //printf("list all signum\n");
                printf(" 1) SIGUSR1      2) SIGINT       3) SIGKILL      4) SIGTRAP      5) SIGABRT     ");
                printf(" 6) SIGBUS       7) SIGSEGV      8) SIGFPE       9) SIGKILL     10) SIGPIPE     ");
                printf("11) SIGSTKFLT   12) SIGALRM     13) SIGTERM     14) SIGCHLD     15) SIGCONT     ");
                printf("16) SIGSTOP     17) SIGTTIN     18) SIGTTOU     19) SIGSYS      20) SIGIO       ");
                printf("21) SIGHUP      22) SIGWINCH    23) SIGVTALRM   24) SIGPROF     25) SIGQUIT     ");
            } else {
                /* 单独列出信号值 */
                printf("%d\n", signo);
            }
            
        } else {    /* 不是列出信号，就是执行信号 */
            if (argc == 2) {
                //printf("send signal %d no pid\n", signo);
                printf("kill: please order process id!\n");
                return -1;
            } else {
                //printf("send signal %d to pid %d\n", signo, pid);
                if (kill(pid, signo) == -1) {
                    printf("kill: pid %d failed.\n", pid);
                    return -1;
                }
            }
        }
    } else {
        //printf("send signal %d tp pid %d\n", signo, pid);
        if (kill(pid, signo) == -1) {
            printf("kill: pid %d failed.\n", pid);	
            return -1;
        }
    }

	return 0;
}

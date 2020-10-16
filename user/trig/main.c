#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <sys/trigger.h>

/**
 * cmd_trig - trig命令
 * 1.命令格式：
 * trig [参数] [进程号]
 * 2.命令功能：
 * 发送指定的信号到相应进程。不指定型号将发送LSOFT（6）终止指定进程。
 * 3.命令参数：
 * -l  信号，若果不加信号的编号参数，则使用“-l”参数会列出全部的信号名称
 * -s  指定发送信号
 * 
 * 4.举例：
 * trig 10          // 默认方式杀死进程10
 * trig -5 10       // 用TRIGHSOFT杀死进程10
 * trig -TRIGHSOFT 10 // 用TRIGHSOFT杀死进程10
 * trig -HSOFT 10    // 用TRIGHSOFT杀死进程10
 * trig -l          // 列出所有信号
 * trig -l HSOFT     // 列出trig对应的信号值
 * trig -l TRIGHSOFT  // 列出trig对应的信号值
 * 就实现以上命令，足矣。
 * 
 */

#define MAX_SUPPORT_SIG_NR      10

/* 信号的字符串列表，用于查找信号对应的信号值 */
char *trigger_table[MAX_SUPPORT_SIG_NR][2] = {
    {"NULL", "NULL"},       /* 信号对应的值 */
    {"TRIGHW", "HW"},     /* 信号对应的值 */
    {"TRIGDBG", "DBG"},       /* 信号对应的值 */
    {"TRIGPAUSE", "PAU"},       /* 信号对应的值 */
    {"TRIGRESUM", "RESUM"},       /* 信号对应的值 */
    {"TRIGHSOFT", "HSOFT"},       /* 信号对应的值 */
    {"TRIGLSOFT", "LSOFT"},       /* 信号对应的值 */
    {"TRIGUSR0", "USR0"},       /* 信号对应的值 */
    {"TRIGUSR1", "USR1"},       /* 信号对应的值 */
    {"TRIGALARM", "ALARM"},       /* 信号对应的值 */
};

/**
 * find_trigger_in_table - 在表中查找信号
 * @name：信号名称
 * 
 * 
 * 找到返回信号值，没找到返回0
 */
int find_trigger_in_table(char *name)
{
    int idx;
    int trigno = -1;
    
    for (idx = 1; idx < MAX_SUPPORT_SIG_NR; idx++) {
        /* 字符串相同就找到 */
        if (!strcmp(trigger_table[idx][0], name) || !strcmp(trigger_table[idx][1], name)) {
            trigno = idx;
            break;
        }
    }

    /* 找到就返回信号 */
    if (trigno != -1) {
        return trigno;
    }
    return 0;
}

int main(int argc, char **argv)
{
	if(argc < 2){
		printf("trig: too few arguments.\n");	
		return -1;
	}

	//默认 argv[1]是进程的pid
	
    int trigno = TRIGLSOFT;    /* 默认是TERM信号，如果没有指定信号的话 */
    int pid = -1;

    char *p;

    /* 不列出信号 */
    bool list_trigger = false;
    /* 是否有可选参数 */
    bool has_option = false;    

    bool pid_negative = false;    /* pid为负数 */

    /* 找出pid和trigno */
    int idx = 1;

    /* 扫描查看是否有可选参数 */
    while (idx < argc) {
        /* 有可选参数，并且是在第一个参数的位置 */
        if (argv[idx][0] == '-' && argv[idx][1] != '\0' && idx == 1) {
            has_option = true;
            p = (char *)(argv[idx] + 1);
            /* 查看是什么选项 */

            if (*p == 'l' && p[1] == '\0') {    /* 是 -l选项 */
                list_trigger = true;     /* 需要列出信号 */
            } else {
                /* 是纯数字，即 -12 */
                if (isdigitstr((const char *)p)) {
                    /* 转换成数值 */
                    trigno = atoi(p);

                    if (1 > trigno || trigno >= MAX_SUPPORT_SIG_NR) {
                        printf("trig: trigger %s number not support!\n", trigno);
                        return -1;
                    }

                } else { /* 不是是纯数字，即 -trig，但也可能找不到 */
                    trigno = find_trigger_in_table(p);

                    /* 找到信号 */
                    if (trigno <= 0) {
                        
                        printf("trig: trigger %s not found!\n", p);
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
                printf("trig: process id %s error!\n", p);
                return -1;
            }
        } else if (argv[idx][0] != '\0' && idx == 2) {  /* 第二个参数 */
            p = (char *)(argv[idx]);

            /* 
            如果是纯数字，就是进程id。
            如果是字符串，就是信号值
             */
            if (list_trigger) {

                /* 是列出信号，就直接解析信号值 */
                trigno = find_trigger_in_table(p);
                
                /* 找到信号 */
                if (trigno <= 0) {
                    printf("trig: trigger %s not found!\n", p);
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
                    printf("trig: process id %s must be number!\n", p);
                    return -1;
                }
            }
        }
        idx++;
    }

    if (has_option) {
        /* 是列出信号 */
        if (list_trigger) {
            if (argc == 2) {
                /* trig -l # 列出所有 */
                //printf("list all signum\n");
                printf(" 1) TRIGHW       2) TRIGDBG      3) TRIGPAUSE    4) TRIGRESUM   5) TRIGHSOFT    "\
                       " 6) TRIGLSOFT    7) TRIGUSR0     8) TRIGUSR1     9) TRIGALARM\n");

            } else {
                /* 单独列出信号值 */
                printf("%d\n", trigno);
            }
            
        } else {    /* 不是列出信号，就是执行信号 */
            if (argc == 2) {
                //printf("send trigger %d no pid\n", trigno);
                printf("trig: please order process id!\n");
                return -1;
            } else {
                //printf("send trigger %d to pid %d\n", trigno, pid);
                if (triggeron(trigno, pid) == -1) {
                    printf("trig: pid %d failed.\n", pid);
                    return -1;
                }
            }
        }
    } else {
        //printf("send trigger %d tp pid %d\n", trigno, pid);
        if (triggeron(trigno, pid) == -1) {
            printf("trig: pid %d failed.\n", pid);	
            return -1;
        }
    }

	return 0;
}
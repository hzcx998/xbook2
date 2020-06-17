#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sgi/sgi.h>
#include "terminal.h"
#include "window.h"

#define CMD_LINE_LEN 256
#define MAX_ARG_NR 16
/* 路径长度 */
#define MAX_PATH_LEN 256

char cwd_cache[MAX_PATH_LEN] = {0};
char cmd_line[CMD_LINE_LEN] = {0};
char *global_cmd_argv[MAX_ARG_NR] = {0};


int cmd_parse(char * cmd_str, char **argv, char token);
int execute_cmd(int argc, char **argv);
int do_buildin_cmd(int cmd_argc, char **cmd_argv);

int main(int argc, char *argv[])
{
    printf("The app %s is started.\n", APP_NAME);

    if (init_con_screen() < 0) {
        return -1;
    }
    if (con_open_window() < 0) {
        return -1;
    }

    memset(cwd_cache, 0, MAX_PATH_LEN);
    strcpy(cwd_cache, "0:/");

    while (1) {
        print_prompt();
        memset(cmd_line, 0, CMD_LINE_LEN);
        if (con_event_loop (cmd_line, CMD_LINE_LEN) < 0)
            break;
        /* 如果什么也没有输入，就回到开始处 */
		if(cmd_line[0] == 0)
			continue;

        /* 处理数据 */
        //printf("cmd: %s\n", cmd_line);

        int argnum = -1;
        argnum = cmd_parse(cmd_line, global_cmd_argv, ' ');
        if(argnum == -1){
            printf("%s: num of arguments exceed %d\n", APP_NAME, MAX_ARG_NR);
            continue;
        }
#if 0        
        /* 打印参数 */
        int i;
        for (i = 0; i < argnum; i++) {
            printf("arg[%d]=%s\n", i, global_cmd_argv[i]);
        }
#endif
        if (execute_cmd(argnum, global_cmd_argv)) {
            printf("%s: execute cmd %s falied!\n", APP_NAME, global_cmd_argv[0]);
        }
    }
    
    con_close_window();
    return 0;
}

void print_prompt() 
{
    cprintf("%s>", cwd_cache);
}


/**
 * cmd_parse - 从输入的命令行解析参数
 * @cmd_str: 命令行缓冲
 * @argv: 参数数组
 * @token: 分割符
 */
int cmd_parse(char * cmd_str, char **argv, char token)
{
	if(cmd_str == NULL){
		return -1;
	}
	int arg_idx = 0;
	while(arg_idx < MAX_ARG_NR){
		argv[arg_idx] = NULL;
		arg_idx++;
	}
	char *next = cmd_str;
	int argc = 0;
	while(*next){
		//跳过token字符
		while(*next == token){
			next++;
		}
		//如果最后一个参数后有空格 例如"cd / "
		if(*next ==0){
			break;
		}
		//存入一个字符串地址，保存一个参数项
		argv[argc] = next;
		
		//每一个参数确定后，next跳到这个参数的最后面
		while(*next && *next != token){
			next++;
		}
		//如果此时还没有解析完，就把这个空格变成'\0'，当做字符串结尾
		if(*next){
			*next++ = 0;
		}
		//参数越界，解析失败
		if(argc > MAX_ARG_NR){
			return -1;
		}
		//指向下一个参数
		argc++;
		//让下一个字符串指向0
		argv[argc] = 0;
	}
	return argc;
}

int execute_cmd(int argc, char **argv)
{
    if (argc < 1)
        return -1; /* at least 1 arg */

    int status = 0;
    int daemon = 0;
    int arg_idx = 0;
    /* scan deamon */
    while (arg_idx < argc) {
        /* 如果在末尾，并且是单独的'&'符号，才当做后台应用 */
        if (!strcmp(argv[arg_idx], "&") && (arg_idx == argc - 1)) {
            daemon = 1;     /* 是后台进程 */
            argc--; /* 参数-1 */
            break;
        }
        arg_idx++;
    }

    /* 先执行内建命令，再选择磁盘中的命令 */
    if (do_buildin_cmd(argc, argv)) {
        /* 在末尾添加上结束参数 */
        argv[argc] = NULL;
        int pid;
        /* 检测文件是否存在，以及是否可执行，不然就返回命令错误 */
        if (access(argv[0], F_OK)) {
            return -1;
        } else {
            /* 创建一个进程 */
            pid = fork();

            if (pid == -1) {  /* 父进程 */
                printf("%s: fork child failed!\n", APP_NAME);
                return -1;
            } else if (pid > 0) {  /* 父进程 */
                if (!daemon) {
                    printf("%s: parent wait child %d\n", APP_NAME, pid);
                    /* shell程序等待子进程退出 */
                    pid = waitpid(pid, &status, 0);
                    /* 执行失败 */
                    if ( status == pid )
                        return -1;
                }
            } else {    /* 子进程 */
                /* 子进程执行程序 */
                pid = execv((const char *) argv[0], (const char **) argv);
                /* 如果执行出错就退出 */
                if (pid == -1) {
                    printf("execv file %s failed!\n", argv[0]);
                    exit(pid);  /* 退出 */
                }
            }
        }
    }
    return 0;
}

typedef int (*cmd_func_t) (int argc, char **argv);

int cmd_cls(int argc, char **argv)
{
	//printf("cls: argc %d\n", argc);
	if ( argc != 1 ) {
		printf("cls: no argument support!\n");
		return -1;
	}

    screen.clear();

    return 0;
}

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

/* buildin cmd struct */
struct buildin_cmd {
    char *name;
    cmd_func_t cmd_func;
};

/* cmd table */
struct buildin_cmd buildin_cmd_table[] = {
    {"cls", cmd_cls},
};

int do_buildin_cmd(int cmd_argc, char **cmd_argv)
{
    int cmd_nr = ARRAY_SIZE(buildin_cmd_table);
    int cmd_idx;
    struct buildin_cmd *cmd_ptr;
    int i = 0;

    /* scan cmd table */
    for (i = 0; i < cmd_nr; i++) {
        cmd_ptr = &buildin_cmd_table[i];
        if (!strcmp(cmd_ptr->name, cmd_argv[0])) {
            if (cmd_ptr->cmd_func(cmd_argc, cmd_argv)) {
                printf("do_buildin_cmd: %s failed!\n");
            }
            return 0;
        }
    }
    /* not a buildin cmd */
    return -1;
}

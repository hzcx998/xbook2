#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <sys/exception.h>

#include "sh.h"

#define CMDLINE_LEN 128


#define CMD_LINE_LEN 128
#define MAX_ARG_NR 16

/* 标准输入输出，错误的备份 */
int sh_stdin_backup;
int sh_stdout_backup;
int sh_stderr_backup;

char cmd_line[CMD_LINE_LEN] = {0};
char cwd_cache[MAX_PATH] = {0};
char *cmd_argv[MAX_ARG_NR];

/* 设置环境变量 */
char *sh_environment[4] = {
    "/bin",
    "/sbin",
    "/usr",
    NULL
};

int main(int argc, char *argv[])
{
    sh_stdin_backup = dup(0);
    sh_stdout_backup = dup(1);
    sh_stderr_backup = dup(2);
    
	memset(cwd_cache, 0, MAX_PATH);
	getcwd(cwd_cache, 32);

    int pid = getpid();
    ioctl(0, TTYIO_HOLDER, &pid);

    expcatch(EXP_CODE_USER, sh_exit_handler);
    expblock(EXP_CODE_TERM);
    expblock(EXP_CODE_INT);
    
    // set environment value
    environ = sh_environment;

    print_logo();

    /* 备份标准输入 */
	while(1){ 
        /* 显示提示符 */
		print_prompt();
        
		memset(cmd_line, 0, CMD_LINE_LEN);
		/* 读取命令行 */
		readline(cmd_line, CMD_LINE_LEN);
		
        /* 如果什么也没有输入，就回到开始处 */
		if(cmd_line[0] == 0){
			continue;
		}

        /* 解析成参数 */
        argc = -1;
        argc = cmd_parse(cmd_line, cmd_argv, ' ');
        if(argc == -1){
            printf("sh: num of arguments exceed %d\n",MAX_ARG_NR);
            continue;
        }
        /* 管道执行 */
        if (execute_cmd(argc, cmd_argv)) {
            printf("sh: execute cmd %s falied!\n", cmd_argv[0]);
        }
    }
	return 0;
}

/**
 * print_prompt - 打印提示符
 * 
 */
void print_prompt()
{
	printf("%s>", cwd_cache);
}

/**
 * readline - 读取一行输入
 * @buf: 缓冲区
 * @count: 数据量
 * 
 * 输入回车结束输入
 */
void readline(char *buf, uint32_t count)
{
    int len = 0;
    char *pos = buf;
    while (len < count)
    {
        read(STDIN_FILENO, pos, 1);
        if (*pos == '\n') {
            *(pos) = '\0'; // 修改成0
            break;
        } else if (*pos == '\b') {
            if (pos > buf) {
                *(pos) = '\0'; // 修改成0
                --pos;
                *(pos) = '\0'; // 修改成0
                len--;
            } else {
                len = 0;
            }
        } else {
            len++;
            pos++;
        }
    }
}

void sh_exit(int ret, int relation)
{
    close(sh_stdin_backup);
    close(sh_stdout_backup);
    close(sh_stderr_backup);
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    if (relation) {
        pid_t ppid = getppid();
        if (ppid > 0) /* 关闭父进程 */
            expsend(EXP_CODE_USER, ppid);
    }
    exit(ret);
}

static int buildin_cmd_exit(int argc, char **argv)
{
    //printf("sh: do buildin exit.\n");
    sh_exit(0, 1);
    return 0;
}

void sh_exit_handler(uint32_t code)
{
    sh_exit(code, 0);
}

int buildin_cmd_cls(int argc, char **argv)
{
	//printf("cls: argc %d\n", argc);
	if (argc != 1) {
		printf("cls: no argument support!\n");
		return -1;
	}
    // 发出控制字符串
    ioctl(STDIN_FILENO, TTYIO_CLEAR, NULL);
    return 0;
}

int buildin_cmd_help(int argc, char **argv)
{
	if(argc != 1){
		printf("help: no argument support!\n");
		return -1;
	}
	printf("shell for book os. version 0.1 \n");
    return 0;
}

int buildin_cmd_cd(int argc, char **argv)
{
	if(argc > 2){
		printf("cd: only support 1 argument!\n");
		return -1;
	}
    /*int i;
    for (i = 0; i < argc; i++) {
        printf("%s",argv[i]);
    }*/

    /* 只有1个参数，是cd，那么不处理 */
    if (argc == 1) {
        return 0; 
    }
    
    char *path = argv[1];

	if(chdir(path) == -1){
		printf("cd: no such directory %s\n",argv[1]);
		return -1;
	}
    /* 设置工作目录缓存 */
    memset(cwd_cache, 0, MAX_PATH);
    getcwd(cwd_cache, MAX_PATH);
    
	return 0;
}

int buildin_cmd_pwd(int argc, char **argv)
{
	//printf("pwd: argc %d\n", argc);
	if(argc != 1){
		printf("pwd: no argument support!\n");
		return -1;
	}else{
        char path[MAX_PATH] = {0};
        getcwd(path, MAX_PATH);
        printf("%s\n", path);
	}
    return 0;
}

/* cmd table */
struct buildin_cmd buildin_cmd_table[] = {
    {"exit", buildin_cmd_exit},
    {"help", buildin_cmd_help},
    {"cls", buildin_cmd_cls},
    {"cd", buildin_cmd_cd},
    {"pwd", buildin_cmd_pwd},
};

static int do_buildin_cmd(int cmd_argc, char **cmd_argv)
{
    int cmd_nr = ARRAY_SIZE(buildin_cmd_table);
    struct buildin_cmd *cmd_ptr;
    int i = 0;
    /* scan cmd table */
    for (i = 0; i < cmd_nr; i++) {
        cmd_ptr = &buildin_cmd_table[i];
        if (!strcmp(cmd_ptr->name, cmd_argv[0])) {
            if (cmd_ptr->cmd_func(cmd_argc, cmd_argv)) {
                //printf("do_buildin_cmd: %s failed!\n", cmd_argv[0]);
            }
            return 0;
        }
    }
    /* not a buildin cmd */
    return -1;
}

/**
 * execute_cmd - 执行命令
 * @argc: 参数数量
 * @argv: 参数地址
 * 
 * 命令：命令+选项+参数+重定向
 */
int execute_cmd(int argc, char **argv)
{
    int status = 0;
    /* 先执行内建命令，再选择磁盘中的命令 */
    if (do_buildin_cmd(argc, argv)) {
        
        /* 在末尾添加上结束参数 */
        argv[argc] = NULL;
        int pid;
    
        /* 创建一个进程 */
        pid = fork();

        if (pid > 0) {  /* 父进程 */
            // 把子进程设置为前台
            // ioctl(0, TTYIO_HOLDER, &pid);
            /* shell程序等待子进程退出 */
            pid = wait(&status);
            pid = getpid();
            ioctl(0, TTYIO_HOLDER, &pid);
        } else {    /* 子进程 */
            pid = getpid();
            ioctl(0, TTYIO_HOLDER, &pid);
            /* 子进程执行程序 */
            pid = execv((const char *)argv[0], (char *const *)argv);

            /* 如果执行出错就退出 */
            if(pid == -1){
                printf("sh: bad command %s!\n", argv[0]);
                pid = getppid();
                ioctl(0, TTYIO_HOLDER, &pid);
                sh_exit(-1, 0);
            }
        }
    }
    return 0;
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

void print_logo()
{
    printf("+----------------------------------------------------+\n");
    printf("| Welcome to xbook2 kernel!                          |\n");
    printf("| All rights reserved by xbook2 kernel develop Team. |\n");
    printf("+----------------------------------------------------+\n");
}

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <math.h>
#include "cmd.h"
#include "terminal.h"
#include "console.h"
#include "cursor.h"

cmd_man_t *cmdman; 

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


/**
 * cmd_set - 设置终端的属性
 * 
 * option:  curs [0-3] 设置光标形状
 *          curc argb值 设置光标颜色
 */
int cmd_set(int argc, char **argv)
{
	if (argc < 3) {
		cprintf("set: too few arguments!\n");
		return -1;
	}
    /*  */
    if (!strcmp(argv[1], "curs")) {
        if (isdigitstr(argv[2])) {
            int shape = atoi(argv[2]);
            if (shape >= CS_SOLID_FRAME && shape < CS_MAX_NR) {
                set_cursor_shape(shape);
            } else {
                cprintf("set: cursor shape type error!\n");
                return -1;
            }
        } else {
            cprintf("set: cursor shape arg is invalid!\n");
            return -1;
        }
    } else if (!strcmp(argv[1], "curc")) {
        if (isdigitstr(argv[2])) {
            SGI_Argb color = (SGI_Argb) atoi(argv[2]);
            color &= 0xffffff;
            color |= (0xff << 24);
            set_cursor_color(color);
        } else {
            cprintf("set: cursor color arg is invalid!\n");
            return -1;
        }
    } else {
        cprintf("set: invalid option!\n");
        return -1;
    }
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
    {"set", cmd_set},
};

int do_buildin_cmd(int cmd_argc, char **cmd_argv)
{
    int cmd_nr = ARRAY_SIZE(buildin_cmd_table);
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

/**
 * cmd_buf_insert - 插入一个命令到历史缓冲区中
 * 
 */
void cmd_buf_insert()
{
    /* 比较命令是否已经在缓冲区当中了，如果是，就直接返回 */
    cmd_buf_t *cmdbuf = &cmdman->cmd_bufs[0];
    int i;
    for (i = 0; i < CMD_BUF_NR; i++) {
        if (cmdbuf->flags > 0) {
            if (!strcmp(cmdbuf->cmdbuf, cmdman->cmd_line)) {
                return;
            }
        }
        cmdbuf++;
    }

    /* 选择下一个即将插入的缓冲区 */
    cmdbuf = &cmdman->cmd_bufs[cmdman->next_cmd_buf];
    memset(cmdbuf->cmdbuf, 0, CMD_LINE_LEN);
    memcpy(cmdbuf->cmdbuf, cmdman->cmd_line, CMD_LINE_LEN);
    cmdbuf->flags = 1;

    /* 指向下一个缓冲区 */
    cmdman->next_cmd_buf++;
    cmdman->cur_cmd_buf = cmdman->next_cmd_buf;
    /* 形成一个环形 */
    if (cmdman->next_cmd_buf >= CMD_BUF_NR)
        cmdman->next_cmd_buf = 0;
}

void cmd_buf_copy()
{
    cmd_buf_t *cmdbuf = &cmdman->cmd_bufs[cmdman->cur_cmd_buf];
    memset(cmdman->cmd_line, 0, CMD_LINE_LEN);
    memcpy(cmdman->cmd_line, cmdbuf->cmdbuf, CMD_LINE_LEN);
}

/**
 * cmd_buf_select - 选择一个历史命令
 * @dir: 选择方向：-1向上选择，1向下选择
 * 
 */
int cmd_buf_select(int dir)
{
    int temp;
    cmd_buf_t *cmdbuf;
    if (dir == -1) {    /* 向上获取一个历史命令 */
        temp = cmdman->cur_cmd_buf - 1;
        if (temp < 0) {
            temp = CMD_BUF_NR - 1;
        }
    } else if (dir == 1) {  /* 向下获取一个历史命令 */
        temp = cmdman->cur_cmd_buf + 1;
        if (temp >= CMD_BUF_NR) {
            temp = 0;
        }
    } else {
        return -1;
    }
    cmdbuf = &cmdman->cmd_bufs[temp];
    if (cmdbuf->flags > 0) {
        /* 选定 */
        cmdman->cur_cmd_buf = temp;
        /* 回写命令 */

        /* 计算一下原有命令占用的终端列数 */
        int cmdlen = strlen(cmdman->cmd_line);
        int cwdlen = strlen(cmdman->cwd_cache);
        int total = cmdlen + cwdlen + 1; /* 多算一个字符 */
        int lines = DIV_ROUND_UP(total, screen.columns);
        /* 如果原来是多行，那么就需要往上移动lines-1行 */
        if (lines > 1)
            cursor.y -= (lines - 1);
        /* 光标所在的位置 */
        int y = cursor.y * screen.char_height;
        /* 要多清除一行的内容 */
        screen.clear_area(0, y, screen.width, (lines + 1) * screen.char_height);
        /* 清除total个字符 */
        con_set_chars(' ', total, 0, cursor.y);
        /* 移动到行首 */
        move_cursor(0, cursor.y);
        /* 打印提示符和当前命令行 */
        print_prompt();
        cmd_buf_copy();
        print_cmdline();
        /* 计算命令行的长度和当前字符的位置 */
        cmdman->cmd_len = strlen(cmdman->cmd_line);
        cmdman->cmd_pos = cmdman->cmd_line + cmdman->cmd_len; /* 末尾位置 */
        return 0;
    }
    return -1;
}

/**
 * cmdline_set - 设置命令行内容 
 */
int cmdline_set(char *buf, int buflen)
{
    /* 计算一下原有命令占用的终端列数 */
    int cmdlen = strlen(cmdman->cmd_line);
    int cwdlen = strlen(cmdman->cwd_cache);
    int total = cmdlen + cwdlen + 1; /* 多算一个字符 */
    int lines = DIV_ROUND_UP(total, screen.columns);
    /* 如果原来是多行，那么就需要往上移动lines-1行 */
    if (lines > 1)
        cursor.y -= (lines - 1);
    /* 光标所在的位置 */
    int y = cursor.y * screen.char_height;
    /* 要多清除一行的内容 */
    screen.clear_area(0, y, screen.width, (lines + 1) * screen.char_height);
    /* 清除total个字符 */
    con_set_chars(' ', total, 0, cursor.y);
    /* 移动到行首 */
    move_cursor(0, cursor.y);
    /* 打印提示符和当前命令行 */
    print_prompt();
    /* 复制命令行内容 */
    memset(cmdman->cmd_line, 0, CMD_LINE_LEN);
    memcpy(cmdman->cmd_line, buf, min(CMD_LINE_LEN, buflen));
    print_cmdline();
    /* 计算命令行的长度和当前字符的位置 */
    cmdman->cmd_len = strlen(cmdman->cmd_line);
    cmdman->cmd_pos = cmdman->cmd_line + cmdman->cmd_len; /* 末尾位置 */

    /* 手动刷新屏幕 */
    SGI_UpdateWindow(screen.display, screen.win, 0, 0, screen.width, screen.height);

    return -1;
}


void print_prompt() 
{
    cprintf("%s>", cmdman->cwd_cache);
}

void print_cmdline()
{
    cprintf(cmdman->cmd_line);
}

int init_cmd_man()
{
    cmdman = malloc(SIZE_CMD_MAN);
    if (cmdman == NULL)
        return -1;
    memset(cmdman, 0, SIZE_CMD_MAN);

    cmdman->cur_cmd_buf = 0;
    cmdman->next_cmd_buf = 0;

    return 0;
}

void exit_cmd_man()
{
    free(cmdman);
}
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <math.h>
#include <sys/ipc.h>
#include <sys/wait.h>
#include <sys/proc.h>
#include <sys/sys.h>
#include <sys/vmm.h>
#include <sys/time.h>
#include <arch/const.h>
#include <time.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/trigger.h>

/// 程序本地头文件
#include <sh_cmd.h>
#include <sh_console.h>
#include <sh_cursor.h>
#include <sh_window.h>
#include <sh_terminal.h>

cmd_man_t *cmdman; 

extern char **environ;

/* 全局shell变量，当执行子进程时，子进程的pid和要传输给子进程的键值 */
int shell_child_pid = -1;
int shell_child_key = -1;

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

int do_daemon(const char *pathname, char *const *argv)
{
    int pid = fork();
    if (pid == -1)
        return -1;
    if (pid == 0) {
        /* 子进程执行程序 */
        pid = execv(pathname, (char *const *) argv);
        /* 如果执行出错就退出 */
        if (pid == -1) {
            shell_printf("file %s not executable!\n", argv[0]);
            exit(pid);  /* 退出 */
        }
    }
    /* 父进程直接返回 */
    return 0;

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
        
        /* 如果是后台程序，直接运行 */
        if (daemon) {
            return do_daemon((const char *) argv[0], (char *const *) argv);
        }
        
        int pid;
    
        int recv_pipe[2];   /* 接收数据的管道 */
        int xmit_pipe[2];   /* 传输数据的管道 */
        /* 创建发送和接收的管道 */
        if (pipe(recv_pipe) < 0) {
            shell_printf("%s: create recv pipe failed!\n", APP_NAME);
            return -1;
        }
        if (pipe(xmit_pipe) < 0) {
            shell_printf("%s: create xmit pipe failed!\n", APP_NAME);
            close(recv_pipe[0]);
            close(recv_pipe[1]);
            return -1;
        }
        
        char buf[513] = {0};
        /* 创建一个进程 */
        pid = fork();
        if (pid == -1) {  /* fork失败 */
            shell_printf("%s: fork child failed!\n", APP_NAME);
            close(recv_pipe[0]);
            close(recv_pipe[1]);
            close(xmit_pipe[0]);
            close(xmit_pipe[1]);
            
            return -1;
        } else if (pid > 0) {  /* 父进程 */
            /* 关闭不用的管道端口 */
            close(recv_pipe[1]);
            close(xmit_pipe[0]);

            /* 非阻塞 */
            ioctl(recv_pipe[0], F_SETFL, O_NONBLOCK);

            int rdbytes;
#ifdef DEBUG_CMD                
            shell_printf("%s: parent wait child %d\n", APP_NAME, pid);
#endif
            shell_child_pid = pid;
            int child_exit = 0;
            while (1) {
                
                /* shell程序等待子进程退出 */
                int waitret = waitpid(-1, &status, WNOHANG);
                /* 没有子进程 */
                if (waitret > 0) {
#ifdef DEBUG_CMD                        
                    printf("%s: pid %d exit with %x.\n", APP_NAME, waitret, status);
#endif
                    /* 子进程成功退出 */
                    child_exit = 1;
                    
                }
                /* ----接收管道---- */
                memset(buf, 0, 513);
                rdbytes = read(recv_pipe[0], buf, 512);
                if (rdbytes > 0) { 
#ifdef DEBUG_CMD
                    shell_printf("%s: read bytes %d\n", APP_NAME, rdbytes);
#endif
                    /* 输出子进程传递来的数据 */
                    shell_printf(buf);
                    //printf(buf);
                }
                if (child_exit == 1) {
                    /* 此时可能还有管道数据尚未处理完，因此还需要继续处理 */
                    if (rdbytes <= 0) {
                        close(recv_pipe[0]);     /* 关闭输出读者 */
                        close(xmit_pipe[1]);      /* 关闭输入写者 */
                        shell_child_pid = -1;    
                        //printf("child %d exit with %d\n", child_exit, status);                    
                        return 0;
                    }
                }

                #if 1
                /* ----输入管道---- */
                if (poll_window() == 1) {
                    //printf("write key %x:%c\n", shell_child_key, shell_child_key);
                    // 把读取到的数据打印到终端, 然后传递给子进程
                    shell_putchar(shell_child_key);
                    int wrret = write(xmit_pipe[1], &shell_child_key, 1);
                    if (wrret < 0) {
                        shell_printf("%s: write key %x:%c to pipe failed!\n", APP_NAME,
                            shell_child_key, shell_child_key);
                    }
                    shell_child_key = -1;
                }
                #endif
                sched_yeild();  // 让出cpu
            }

        } else {    /* 子进程 */
            /* 设置为忽略，避免在初始化过程中被打断 */
            trigger(TRIGLSOFT, TRIG_IGN);

            /* 关闭不用的管道端口 */
            close(recv_pipe[0]);
            close(xmit_pipe[1]);

            int new_fd;
            /* 把输出管道重定向到标准输出资源 */
            new_fd = dup2(recv_pipe[1], STDOUT_FILENO); 
            if (new_fd < 0) {
                shell_printf("%s: redirect pipe to stdout failed!\n", APP_NAME);
                close(recv_pipe[1]);
                close(xmit_pipe[0]);
                exit(pid);  /* 退出 */
            }
            close(recv_pipe[1]);
   
            /* 把输入管道重定向到标准输入资源 */
            new_fd = dup2(xmit_pipe[0], STDIN_FILENO); 
            if (new_fd < 0) {
                shell_printf("%s: redirect pipe to stdin failed!\n", APP_NAME);
                close(xmit_pipe[0]);
                exit(pid);  /* 退出 */
            }
            close(xmit_pipe[0]);
            /* 恢复默认触发 */
            trigger(TRIGLSOFT, TRIG_DFL);
            /* 子进程执行程序 */
            if (execv((const char *) argv[0], (char *const *) argv) < 0)
                printf("bosh: %s is a bad programe!\n", argv[0]);
            exit(-1);
        }
    }
    shell_child_pid = -1;
    return 0;
}

typedef int (*cmd_func_t) (int argc, char **argv);

int cmd_cls(int argc, char **argv)
{
	//printf("cls: argc %d\n", argc);
	if ( argc != 1 ) {
		shell_printf("cls: no argument support!\n");
		return -1;
	}

    con_screen.clear();
    return 0;
}

struct gui_argb {
    unsigned char blue;
    unsigned char green;
    unsigned char red;
    unsigned char alpha;
};

/**
 * cmd_set - 设置终端的属性
 * 
 * option:  cursors [0-3] 设置光标形状，例如： set cursors 1
 *          cursorc rgb值 设置光标颜色，例如： set cursorc 255 192 168
 *          backc  rgb值 设置背景颜色，例如： set backc 0 50 100
 *          fontc  rgb值 设置字体颜色，例如： set fonts 192 168 255
 * 
 * 后续采用配置文件的方式来配置参数
 */
int cmd_set(int argc, char **argv)
{
	if (argc < 2) {
        /* 打印set命令信息 */
		//shell_printf("set: too few arguments!\n");
        shell_printf("Usage: set [option]\n");
        shell_printf("Option:\n");
        shell_printf("  cursors    set cursor shape. shape is [0-3].\n");
        shell_printf("  cursorc    set cursor color. color is red[0-255] green[0-255] blue[0-255].\n");
        shell_printf("  backc      set background color. color is red[0-255] green[0-255] blue[0-255].\n");
        shell_printf("  fontc      set font color. color is red[0-255] green[0-255] blue[0-255].\n");
		return -1;
	}
#if 0
    struct gui_argb color;

    /*  */
    if (!strcmp(argv[1], "cursors")) {
        if (argc != 3) {
            shell_printf("set: try 'set cursors [0-3]' again!\n");
            return -1;
        }
        
        if (isdigitstr(argv[2])) {
            int shape = atoi(argv[2]);
            if (shape >= CS_SOLID_FRAME && shape < CS_MAX_NR) {
                set_cursor_shape(shape);
            } else {
                shell_printf("set: cursor shape type error!\n");
                return -1;
            }
        } else {
            shell_printf("set: cursor shape arg is invalid!\n");
            return -1;
        }
    } else if (!strcmp(argv[1], "cursorc")) {
        if (argc != 5) {
            shell_printf("set: try 'set cursorc read[0-255] green[0-255] blue[0-255]' again!\n");
            return -1;
        }
        color.alpha = 255;

        if (isdigitstr(argv[2])) {
            color.red = (unsigned char) atoi(argv[2]);
        } else {
            shell_printf("set: cursor color read is invalid!\n");
            return -1;
        }
        if (isdigitstr(argv[3])) {
            color.green = (unsigned char) atoi(argv[3]);
        } else {
            shell_printf("set: cursor color green is invalid!\n");
            return -1;
        }
        if (isdigitstr(argv[4])) {
            color.blue = (unsigned char) atoi(argv[4]);
        } else {
            shell_printf("set: cursor color blue is invalid!\n");
            return -1;
        }
        /* 获取了完整值，设置颜色 */
        set_cursor_color(*(GUI_COLOR *)&color);
        
    } else if (!strcmp(argv[1], "backc")) {
        if (argc != 5) {
            shell_printf("set: try 'set backc read[0-255] green[0-255] blue[0-255]' again!\n");
            return -1;
        }
        color.alpha = 255;
        
        if (isdigitstr(argv[2])) {
            color.red = (unsigned char) atoi(argv[2]);
        } else {
            shell_printf("set: back ground color read is invalid!\n");
            return -1;
        }
        if (isdigitstr(argv[3])) {
            color.green = (unsigned char) atoi(argv[3]);
        } else {
            shell_printf("set: back ground color green is invalid!\n");
            return -1;
        }
        if (isdigitstr(argv[4])) {
            color.blue = (unsigned char) atoi(argv[4]);
        } else {
            shell_printf("set: back ground color blue is invalid!\n");
            return -1;
        }
        /* 获取了完整值，设置颜色 */
        con_set_back_color(*(GUI_COLOR *)&color);
        /* 刷新屏幕 */
        con_flush();
    } else if (!strcmp(argv[1], "fontc")) {
        if (argc != 5) {
            shell_printf("set: try 'set fontc read[0-255] green[0-255] blue[0-255]' again!\n");
            return -1;
        }
        color.alpha = 255;
        
        if (isdigitstr(argv[2])) {
            color.red = (unsigned char) atoi(argv[2]);
        } else {
            shell_printf("set: font color read is invalid!\n");
            return -1;
        }
        if (isdigitstr(argv[3])) {
            color.green = (unsigned char) atoi(argv[3]);
        } else {
            shell_printf("set: font color green is invalid!\n");
            return -1;
        }
        if (isdigitstr(argv[4])) {
            color.blue = (unsigned char) atoi(argv[4]);
        } else {
            shell_printf("set: font color blue is invalid!\n");
            return -1;
        }
        /* 获取了完整值，设置颜色 */
        con_set_font_color(*(GUI_COLOR *)&color);
        /* 刷新屏幕 */
        con_flush();
    } else {
        shell_printf("set: invalid option!\n");
        return -1;
    }
#endif    
    return 0;
}

static const char *proc_print_status[] = {
    "READY",
    "RUNNING",
    "BLOCKED",
    "WAITING",
    "STOPED",
    "ZOMBIE",
    "DIED"
};

int cmd_exit(int argc, char **argv)
{
    exit_cmd_man();
    exit_console();
    return 0; 
}

int cmd_cd(int argc, char **argv)
{
	//printf("pwd: argc %d\n", argc);
	if(argc > 2){
		shell_printf("cd: only support 1 argument!\n");
		return -1;
	}
    /*int i;
    for (i = 0; i < argc; i++) {
        shell_printf("%s",argv[i]);
    }*/

    /* 只有1个参数，是cd，那么不处理 */
    if (argc == 1) {
        return 0; 
    }
    
    char *path = argv[1];

	if(chdir(path) == -1){
		shell_printf("cd: no such directory %s\n",argv[1]);
		return -1;
	}
    /* 设置工作目录缓存 */
    memset(cmdman->cwd_cache, 0, MAX_PATH_LEN);
    getcwd(cmdman->cwd_cache, MAX_PATH_LEN);
    
	return 0;
}

int cmd_pwd(int argc, char **argv)
{
	//printf("pwd: argc %d\n", argc);
	if(argc != 1){
		shell_printf("pwd: no argument support!\n");
		return -1;
	}else{
        char path[MAX_PATH] = {0};
        getcwd(path, MAX_PATH);
        shell_printf("%s\n", path);
        /*char cwdpath[MAX_PATH_LEN];
		if(!getcwd(cwdpath, MAX_PATH_LEN)){
			printf("%s\n", cwdpath);
		}else{
			printf("pwd: get current work directory failed!\n");
		}*/
	}
    return 0;
}

int cmd_help(int argc, char **argv)
{
	if(argc != 1){
		shell_printf("help: no argument support!\n");
		return -1;
	}
	shell_printf("  cls         clean screen.\n"\
	        "  exit        exit shell.\n"\
            "  cd          change current work directory.\n"\
            "  pwd         print current work directory.\n");

    return 0;
}

/* buildin cmd struct */
struct buildin_cmd {
    char *name;
    cmd_func_t cmd_func;
};

/* cmd table */
struct buildin_cmd buildin_cmd_table[] = {
    {"cls", cmd_cls},
    {"help", cmd_help},
    {"exit", cmd_exit},
    {"cd", cmd_cd},
    {"pwd", cmd_pwd},
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
                //shell_printf("do_buildin_cmd: %s failed!\n", cmd_argv[0]);
            }
            return 0;
        }
    }
    /* not a buildin cmd */
    return -1;
}

void print_prompt() 
{
    shell_printf("%s>", cmdman->cwd_cache);
}

void print_cmdline()
{
    shell_printf(cmdman->cmd_line);
}
char *cmd_argv[MAX_ARG_NR] = {0};

int cmdline_check()
{

    /* 如果什么也没有输入，就回到开始处 */
    if(cmdman->cmd_line[0] == 0)
        return -1;

    /* 记录历史缓冲区 */
    cmd_buf_insert();

    /* 重置命令参数 */
    cmdman->cmd_pos = cmdman->cmd_line;
    cmdman->cmd_len = 0;

    /* 处理数据 */
    //printf("cmd: %s\n", cmd_line);
    /* 记录历史缓冲区 */
    //cmd_buf_insert();
    
    int argnum = -1;
    argnum = cmd_parse(cmdman->cmd_line, cmd_argv, ' ');
    
    if(argnum == -1){
        shell_printf("%s: num of arguments exceed %d\n", APP_NAME, MAX_ARG_NR);
        memset(cmdman->cmd_line, 0, CMD_LINE_LEN);
        return -1;
    }
#if 0
    /* 打印参数 */
    int i;
    for (i = 0; i < argnum; i++) {
        printf("arg[%d]=%s\n", i, cmd_argv[i]);
    }
#endif
    /* 修改消息路由 */
    set_win_proc(1);
    if (execute_cmd(argnum, cmd_argv) < 0) {
        //shell_printf("%s: execute cmd %s falied!\n", APP_NAME, cmd_argv[0]);
        memset(cmdman->cmd_line, 0, CMD_LINE_LEN);
        set_win_proc(0);
        return -1;
    }
    memset(cmdman->cmd_line, 0, CMD_LINE_LEN);
    set_win_proc(0);
    return 0;
}

char *shell_environment[4] = {
    "/bin",
    "/sbin",
    "/usr",
    NULL
};

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
        int lines = DIV_ROUND_UP(total, con_screen.columns);
        /* 如果原来是多行，那么就需要往上移动lines-1行 */
        if (lines > 1)
            cursor.y -= (lines - 1);
        /* 光标所在的位置 */
        int y = cursor.y * con_screen.char_height;
        /* 要多清除一行的内容 */
        con_screen.clear_area(0, y, con_screen.width, (lines + 1) * con_screen.char_height);
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
    int lines = DIV_ROUND_UP(total, con_screen.columns);
    /* 如果原来是多行，那么就需要往上移动lines-1行 */
    if (lines > 1)
        cursor.y -= (lines - 1);
    /* 光标所在的位置 */
    int y = cursor.y * con_screen.char_height;
    /* 要多清除一行的内容 */
    con_screen.clear_area(0, y, con_screen.width, (lines + 1) * con_screen.char_height);
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
    sh_window_update(0, 0, con_screen.width, con_screen.height);

    return -1;
}

int init_cmd_man()
{
    cmdman = malloc(SIZE_CMD_MAN);
    if (cmdman == NULL)
        return -1;
    memset(cmdman, 0, SIZE_CMD_MAN);

    memset(cmdman->cwd_cache, 0, MAX_PATH_LEN);
    getcwd(cmdman->cwd_cache, MAX_PATH_LEN);
    
    memset(cmdman->cmd_line, 0, CMD_LINE_LEN);

    cmdman->cur_cmd_buf = 0;
    cmdman->next_cmd_buf = 0;

    cmdman->cmd_pos = cmdman->cmd_line;
    cmdman->cmd_len = 0;

    chdir("/");
    
    environ = shell_environment;

    print_prompt();

    return 0;
}

void exit_cmd_man()
{
    free(cmdman);
}
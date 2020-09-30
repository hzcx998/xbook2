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
#include <sh_shell.h>
#include <sh_console.h>
#include <sh_cursor.h>
#include <sh_window.h>
#include <sh_shell.h>

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
            exit(execv((const char *) argv[0], (char *const *) argv));
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

/**
 * cmd_ps - 查看任务
 */
int cmd_ps(int argc, char **argv)
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
                shell_printf("Usage: ps [option]\n");
                shell_printf("Option:\n");
                shell_printf("  -a    Print all tasks. Example: ps -a \n");
                shell_printf("  -h    Get help of ps. Example: ps -h \n");
                shell_printf("Note: If no arguments, only print user process.\n");
                return 0;
            default:
                shell_printf("ps: unknown option!\n");
                return -1;
            }
        } else {
            shell_printf("ps: unknown argument!\n");
            return -1;
        }
    }

    shell_printf("   PID   PPID     STAT    PRO      TICKS    NAME\n");
    while (!tstate(&ts, &num)) {
        /* 如果没有全部标志，就只显示用户进程。也就是ppid不为-1的进程 */
        if (!all) {
            if (ts.ts_ppid == -1)
                continue;
        }
        shell_printf("%6d %6d %8s %6d %10d    %s\n", 
            ts.ts_pid, ts.ts_ppid, proc_print_status[(unsigned char) ts.ts_state], ts.ts_priority,
            ts.ts_runticks, ts.ts_name);
    }
    return 0;
}

int cmd_ver(int argc, char **argv)
{
	char buf[SYS_VER_LEN] = {0};
    getver(buf, SYS_VER_LEN);
    shell_printf("%s\n",buf);
    return 0;
}

int cmd_exit(int argc, char **argv)
{
    exit_shell();
    return 0; 
}

int cmd_mem(int argc, char **argv)
{
    if (argc > 1) {
        shell_printf("free: no arguments support!\n");
        return -1;
    }
    mstate_t ms;
    mstate(&ms);
    shell_printf("          TOTAL           USED           FREE\n");
    shell_printf("%14dB%14dB%14dB\n", ms.ms_total, ms.ms_used, ms.ms_free);
    shell_printf("%14dM%14dM%14dM\n", ms.ms_total / MB, ms.ms_used / MB, ms.ms_free / MB);
    return 0;
}

int cmd_date(int argc, char **argv)
{
    ktime_t ktm;
    ktime(&ktm);
    struct tm tm;
    ktimeto(&ktm, &tm);
    shell_printf("date: %s\n", asctime(&tm));
    return 0;
}

static void __ls(char *pathname, int detail)
{
	DIR *dir = opendir(pathname);
	if(dir == NULL){
		printf("opendir failed!\n");
        return;
	}
	rewinddir(dir);
	
	struct dirent *de;

    char subpath[MAX_PATH_LEN];

    struct stat fstat;  
    char type;
    char attrR, attrW, attrX;   /* 读写，执行属性 */ 

    do {
        if ((de = readdir(dir)) == NULL) {
            break;
        }
        //printf("de.type:%d", de.type);
        
        if (detail) {
            /* 列出详细信息 */
            if (de->d_attr & DE_DIR) {
                type = 'd';
            } else {
                type = '-';
            }
            
            memset(subpath, 0, MAX_PATH_LEN);
            /* 合并路径 */
            strcat(subpath, pathname);
            strcat(subpath, "/");
            strcat(subpath, de->d_name);
                
            memset(&fstat, 0, sizeof(struct stat));
            /* 如果获取失败就获取下一个 */
            if (stat(subpath, &fstat)) {
                continue;
            }
            
            if (fstat.st_mode & S_IREAD) {
                attrR = 'r';
            } else {
                attrR = '-';
            }

            if (fstat.st_mode & S_IWRITE) {
                attrW = 'w';
            } else {
                attrW = '-';
            }

            if (fstat.st_mode & S_IEXEC) {
                attrX = 'x';
            } else {
                attrX = '-';
            }

            /* 类型,属性，文件日期，大小，名字 */
            shell_printf("%c%c%c%c %04d/%02d/%02d %02d:%02d:%02d %12d %s\n",
                type, attrR, attrW, attrX,
                FILE_TIME_YEA(fstat.st_mtime>>16),
                FILE_TIME_MON(fstat.st_mtime>>16),
                FILE_TIME_DAY(fstat.st_mtime>>16),
                FILE_TIME_HOU(fstat.st_mtime&0xffff),
                FILE_TIME_MIN(fstat.st_mtime&0xffff),
                FILE_TIME_SEC(fstat.st_mtime&0xffff),
                fstat.st_size, de->d_name);
            //printf("type:%x inode:%d name:%s\n", de.type, de.inode, de.name);
        } else {
            shell_printf("%s ", de->d_name);
        }
    } while (1);
	
	closedir(dir);
    if (!detail) {
        shell_printf("\n");
    }
}

int cmd_ls(int argc, char **argv)
{
    /*int i;
    for (i = 0; i < argc; i++) {
        shell_printf("%s ",argv[i]);
    }*/
    /* 只有一个参数 */
    if (argc == 1) {
        /* 列出当前工作目录所在的文件 */
        __ls(".", 0);
    } else {
        /*  */
        int arg_path_nr = 0;
        int arg_idx = 1;	//跳过argv[0]
        char *path = NULL;

        int detail = 0;
        while(arg_idx < argc){
            if(argv[arg_idx][0] == '-'){//可选参数
                char *option = &argv[arg_idx][1];
                /* 有可选参数 */
                if (*option) {
                    /* 列出详细信息 */
                    if (*option == 'l') {
                        detail = 1;
                    } else if (*option == 'h') {
                        shell_printf("Usage: ls [option]\n");
                        shell_printf("Option:\n");
                        shell_printf("  -l    Print all infomation about file or directory. Example: ls -l \n");
                        shell_printf("  -h    Get help of ls. Example: ls -h \n");
                        shell_printf("  [dir] Print [dir] file or directory. Example: ls / \n");
                        shell_printf("Note: If no arguments, only print name in cwd.\n");
                        
                        return 0;
                    } 
                }
            } else {
                if(arg_path_nr == 0){
                    /* 获取路径 */
                    path = argv[arg_idx];
                    
                    arg_path_nr = 1;
                }else{
                    shell_printf("ls: only support one path!\n");
                    return -1;
                }
            }
            arg_idx++;
        }
        if (path == NULL) { /* 没有路径就列出当前目录 */
            __ls(".", detail);
        } else {    /* 有路径就列出路径 */
            __ls(path, detail);
        }
    }
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

int cmd_copy(int argc, char **argv)
{
	if(argc < 3){
		shell_printf("cp: command syntax is incorrect.\n");	
		return -1;
	}

	if(!strcmp(argv[1], ".") || !strcmp(argv[1], "..")){
		shell_printf("cp: src pathnamne can't be . or .. \n");	
		return -1;
	}
	if(!strcmp(argv[2], ".") || !strcmp(argv[2], "..")){
		shell_printf("cp: dst pathname can't be . or .. \n");	
		return -1;
	}

    /* 如果2者相等则不能进行操作 */
    if (!strcmp(argv[1], argv[2])) {
        shell_printf("cp: source file and dest file must be differern!\n");	
		return -1;
    }
    /* 复制逻辑：
        1. 打开两个文件，不存在则创建，已经存在则截断
        2. 复制数据
        3.关闭文件
     */
    int fdrd = open(argv[1], O_RDONLY, 0);
    if (fdrd == -1) {
        shell_printf("cp: open file %s failed!\n", argv[1]);
        return -1;
    }
    /* 如果文件已经存在则截断 */
    int fdwr = open(argv[2], O_CREAT | O_WRONLY | O_TRUNC, 0);
    if (fdwr == -1) {
        shell_printf("cp: open file %s failed!\n", argv[2]);
        close(fdrd);
        return -1;
    }

    struct stat fstat;

    if (stat(argv[1], &fstat) < 0) {
        shell_printf("mv: get file %s state failed!\n", argv[1]);
        close(fdrd);
        close(fdwr);
        return -1;
    }

    /* 每次操作512字节 */
    char *buf = malloc(fstat.st_size);
    if (buf == NULL) {
        shell_printf("cp: malloc for size %d failed!\n", fstat.st_size);
        goto err;
    }

    char *p = buf;
    int size = fstat.st_size;
    int readBytes;

    /* 每次读取64kb */
    int chunk = (size & 0xffff) + 1;
    
    /* 如果chunk为0，就设置块大小 */
    if (chunk == 0) {
        chunk = 0xffff;
        size -= 0xffff;
    }
        
    while (size > 0) {  
        readBytes = read(fdrd, p, chunk);
        //printf("read:%d\n", readBytes);
        if (readBytes == -1) {  /* 应该检查是错误还是结束 */
            goto failed; 
        }
        if (write(fdwr, p, readBytes) == -1) {
            goto failed;  
        }
        p += chunk;
        size -= 0xffff;
        chunk = 0xffff;
    }

    /* 设置模式和原来文件一样 */
    chmod(argv[2], fstat.st_mode);

    free(buf);
    /* 复制结束 */
    close(fdrd);
    close(fdwr);
    return 0;
failed:
    shell_printf("cp: transmit data error!\n");
    free(buf);
err:
    /* 复制结束 */
    close(fdrd);
    close(fdwr);
    return -1;
}

int cmd_move(int argc, char *argv[])
{
	if(argc < 3){
		shell_printf("mv: command syntax is incorrect.\n");	
		return -1;
	}

	if(!strcmp(argv[1], ".") || !strcmp(argv[1], "..")){
		shell_printf("mv: src pathnamne can't be . or .. \n");	
		return -1;
	}
	if(!strcmp(argv[2], ".") || !strcmp(argv[2], "..")){
		shell_printf("mv: dst pathname can't be . or .. \n");	
		return -1;
	}

    /* 如果2者相等则不能进行操作 */
    if (!strcmp(argv[1], argv[2])) {
        shell_printf("mv: source file and dest file must be differern!\n");	
		return -1;
    }
    /* 复制逻辑：
        1. 打开两个文件，不存在则创建，已经存在则截断
        2. 复制数据
        3.关闭文件
     */
    int fdrd = open(argv[1], O_RDONLY, 0);
    if (fdrd == -1) {
        shell_printf("mv: open file %s failed!\n", argv[1]);
        return -1;
    }
    /* 如果文件已经存在则截断 */
    int fdwr = open(argv[2], O_CREAT | O_WRONLY | O_TRUNC, 0);
    if (fdwr == -1) {
        shell_printf("mv: open file %s failed!\n", argv[2]);
        close(fdrd);
        return -1;
    }

    struct stat fstat;

    if (stat(argv[1], &fstat) < 0) {
        shell_printf("mv: get file %s state failed!\n", argv[1]);
        close(fdrd);
        close(fdwr);
        return -1;
    }

    /* 每次操作512字节 */
    char *buf = malloc(fstat.st_size);
    if (buf == NULL) {
        shell_printf("mv: malloc for size %d failed!\n", fstat.st_size);
        goto err;
    }

    char *p = buf;
    int size = fstat.st_size;
    int readBytes;

    /* 每次读取64kb */
    int chunk = (size & 0xffff) + 1;

    if (chunk == 0) {
        chunk = 0xffff;
        size -= 0xffff;
    }    
    while (size > 0) {  
        readBytes = read(fdrd, p, chunk);
        //printf("read:%d\n", readBytes);
        if (readBytes == -1) {  /* 应该检查是错误还是结束 */
            goto failed; 
        }
        if (write(fdwr, p, readBytes) == -1) {
            goto failed;  
        }
        p += chunk;
        size -= 0xffff;
        chunk = 0xffff;
    }

    /* 设置模式和原来文件一样 */
    chmod(argv[2], fstat.st_mode);
    
    free(buf);
    /* 复制结束 */
    close(fdrd);
    close(fdwr);

    /* 移动后删除源文件 */
    if(remove(argv[1]) == 0){
        //shell_printf("mv: delete source file %s success.\n", argv[1]);
    }else{
        shell_printf("mv: delete source file %s faild!\n", argv[1]);
        /* 删除复制后的文件 */
        remove(argv[2]);
    }

    return 0;
failed:
    printf("mv: transmit data error!\n");
    free(buf);
err:
    /* 复制结束 */
    close(fdrd);
    close(fdwr);
    return -1;
}

int cmd_rename(int argc, char *argv[])
{
	if(argc < 3){
		shell_printf("rename: command syntax is incorrect.\n");	
		return -1;
	}

	if(!strcmp(argv[1], ".") || !strcmp(argv[1], "..")){
		shell_printf("rename: pathnamne can't be . or .. \n");	
		return -1;
	}
	if(!strcmp(argv[2], ".") || !strcmp(argv[2], "..")){
		shell_printf("rename: new name can't be . or .. \n");	
		return -1;
	}

	if(!rename(argv[1], argv[2])){
		//shell_printf("rename: %s to %s sucess!\n", argv[1], argv[2]);	
		return 0;
	}else{
        shell_printf("rename: %s to %s faild!\n", argv[1], argv[2]);	
		return -1;
	}

    return 0;
}

int cmd_mkdir(int argc, char *argv[])
{
	int ret = -1;
	if(argc != 2){
		shell_printf("mkdir: no argument support!\n");
	}else{
    
        if(mkdir(argv[1], 0) == 0){
            //shell_printf("mkdir: create a dir %s success.\n", argv[1]);
            ret = 0;
        }else{
            shell_printf("mkdir: create directory %s faild!\n", argv[1]);
        }
	}
	return ret;
}

int cmd_rmdir(int argc, char *argv[])
{
	int ret = -1;
	if(argc != 2){
		shell_printf("mkdir: no argument support!\n");
	}else{
		
        if(rmdir(argv[1]) == 0){
            //shell_printf("rmdir: remove %s success.\n", argv[1]);
            ret = 0;
        }else{
            shell_printf("rmdir: remove %s faild!\n", argv[1]);
        }
		
	}
	return ret;
}

int cmd_rm(int argc, char *argv[])
{
	int ret = -1;
	if(argc != 2){
		shell_printf("rm: no argument support!\n");
	}else{
        if(remove(argv[1]) == 0){
            //shell_printf("rm: delete %s success.\n", argv[1]);
            ret = 0;
        }else{
            shell_printf("rm: delete %s faild!\n", argv[1]);
        }
	}
	return ret;
}

/*
touch: create a file
*/
static int cmd_touch(int argc, char *argv[])
{
	//printf("argc: %d\n", argc);
	if(argc == 1){	//只有一个参数，自己的名字，退出
		shell_printf("touch: please input filename!\n");
		return 0;
	}
	if(argc > 2){
		shell_printf("touch: only support 2 argument!\n");
		return -1;
	}
	
    const char *path = (const char *)argv[1];

	int fd = open(path, O_CREAT | O_RDWR, 0);
	if(fd == -1){
		shell_printf("touch: fd %d error\n", fd);
		return 0;
	}

	close(fd);
	return 0;
}

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

int cmd_trig(int argc, char **argv)
{
	if(argc < 2){
		shell_printf("trig: too few arguments.\n");	
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
                        shell_printf("trig: trigger %s number not support!\n", trigno);
                        return -1;
                    }

                } else { /* 不是是纯数字，即 -trig，但也可能找不到 */
                    trigno = find_trigger_in_table(p);

                    /* 找到信号 */
                    if (trigno <= 0) {
                        
                        shell_printf("trig: trigger %s not found!\n", p);
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
                shell_printf("trig: process id %s error!\n", p);
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
                    shell_printf("trig: trigger %s not found!\n", p);
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
                    shell_printf("trig: process id %s must be number!\n", p);
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
                shell_printf(" 1) TRIGHW       2) TRIGDBG      3) TRIGPAUSE    4) TRIGRESUM   5) TRIGHSOFT    "\
                       " 6) TRIGLSOFT    7) TRIGUSR0     8) TRIGUSR1     9) TRIGALARM\n");

            } else {
                /* 单独列出信号值 */
                shell_printf("%d\n", trigno);
            }
            
        } else {    /* 不是列出信号，就是执行信号 */
            if (argc == 2) {
                //printf("send trigger %d no pid\n", trigno);
                shell_printf("trig: please order process id!\n");
                return -1;
            } else {
                //printf("send trigger %d to pid %d\n", trigno, pid);
                if (triggeron(trigno, pid) == -1) {
                    shell_printf("trig: pid %d failed.\n", pid);
                    return -1;
                }
            }
        }
    } else {
        //printf("send trigger %d tp pid %d\n", trigno, pid);
        if (triggeron(trigno, pid) == -1) {
            shell_printf("trig: pid %d failed.\n", pid);	
            return -1;
        }
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
	        "  mem         print memory info.\n"\
	        "  ps          print tasks.\n"\
            "  date        show date.\n"\
            "  ls          list files.\n"\
            "  cd          change current work directory.\n"\
            "  pwd         print current work directory.\n"\
            "  trigger     active process trigger.\n"\
	        "  ver         show os version.\n");

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
    {"ps", cmd_ps},
    {"help", cmd_help},
    {"ver", cmd_ver},
    {"exit", cmd_exit},
    {"mem", cmd_mem},
    {"date", cmd_date},
    {"ls", cmd_ls},
    {"cd", cmd_cd},
    {"pwd", cmd_pwd},
    {"trig", cmd_trig},
    {"cp", cmd_copy},
    {"mv", cmd_move},
    {"rn", cmd_rename},
    {"mkdir", cmd_mkdir},
    {"rmdir", cmd_rmdir},
    {"rm", cmd_rm},
    {"touch", cmd_touch},
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
    g_set_msg_routine(process_window2);

    if (execute_cmd(argnum, cmd_argv) < 0) {
        //shell_printf("%s: execute cmd %s falied!\n", APP_NAME, cmd_argv[0]);
        memset(cmdman->cmd_line, 0, CMD_LINE_LEN);
        g_set_msg_routine(process_window);
    
        return -1;
    }
    memset(cmdman->cmd_line, 0, CMD_LINE_LEN);
    g_set_msg_routine(process_window);
    return 0;
}

char *shell_environment[3] = {
    "/bin",
    "/sbin",
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
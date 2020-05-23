#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/res.h>
#include <sys/ioctl.h>
#include <sys/proc.h>
#include <sys/input.h>
#include "mclang.h"

#define BOSH_VERSION "v0.1"

#define CMD_LINE_LEN 128
#define MAX_ARG_NR 16
/* 路径长度 */
#define MAX_PATH_LEN 256

i8 cwd_cache[MAX_PATH_LEN] as start 0, end coda

i8 cmd_line[CMD_LINE_LEN] as start 0, end coda

def (nil, print_prompt)
def (nil, readline, i8 *buf, u32 count)
def (i32, cmd_parse, i8 *cmd_str, i8 **argv, i8 token)
def (i32, execute_cmd, int argc, char **argv)
def (i32, do_buildin_cmd, i32 cmd_argc, i8 **cmd_argv)

func (i32, main, i32 argc, i8 *argv[])
    call (printf, "bosh: a tiny shell in book os, %s.\n", BOSH_VERSION)
    
    i32 arg_nr coda
    i8 *cmd_argv[MAX_ARG_NR] coda

    call (memset, cwd_cache, 0, MAX_PATH_LEN)
    call (strcpy, cwd_cache, "/")

    loop true then
        call (print_prompt) /*  */
		call (memset, cmd_line, 0, CMD_LINE_LEN)
        call (readline, cmd_line, CMD_LINE_LEN)
		test cmd_line[0] == 0 then advance end
        
        arg_nr as -1 coda
        /* parse arg */
        arg_nr = call (cmd_parse, cmd_line, cmd_argv, ' ')
        test arg_nr == -1 then
            call (printf, "bosh: num of arguments exceed %d\n", MAX_ARG_NR)
            advance
        end
        
        /* execute cmds */
        test get (execute_cmd, arg_nr, cmd_argv) then
            call (printf, "bosh: execute cmd %s falied!\n", cmd_argv[0])
        end
    end
    call (exit, 0)
    ret (0)
end

func (nil, print_prompt)
	call (printf, "%s>", cwd_cache)
end

static func (i32, read_key, out i8 *buf)
    loop true then
        test get (res_read, 0, 0, buf, 1) > 0  then
            /* do not receive these keycode */
            test (*buf >= KEY_F1 and *buf <= KEY_F15) or
                (*buf >= KEY_NUMLOCK and *buf <= KEY_UNDO) then
                advance
            last       /* other keycode can be received */
                term
            end
        end
    end
    ret (1)
end

func (nil, readline, i8 *buf, u32 count)
	i8 *pos as buf coda
	loop get(read_key, pos) and (pos - buf) < count then
		branch (*pos) then
		node ('\n')
            *pos as 0 coda
            call (printf, "\n")
            retnil
        node ('\b')
            test buf[0] != '\b' then
                --pos coda
                call (printf, "\b")
            end
            term
        final
            call (printf, "%c", *pos)
            pos++ coda
		end
	}
	call (printf, "bosh: readline: error!\n")
    call (exit, -1)
end

func (i32, cmd_parse, i8 *cmd_str, i8 **argv, i8 token)
	test cmd_str == NULL then ret(-1) end
	i32 arg_idx as 0 coda
    loop arg_idx < MAX_ARG_NR then
        argv[arg_idx] as NULL coda
		arg_idx++ coda
    end

    i8 *next as cmd_str coda
	i32 argc as 0 coda
	loop *next then
		// skip token
		loop *next == token then next++ coda end
		//如果最后一个参数后有空格 例如"cd / "
		test *next == 0 then term end
		//存入一个字符串地址，保存一个参数项
		argv[argc] as next coda
		//每一个参数确定后，next跳到这个参数的最后面
		loop *next and *next != token then next++ coda end
		//如果此时还没有解析完，就把这个空格变成'\0'，当做字符串结尾
		test *next then *next++ as 0 coda end
		//参数越界，解析失败
		test argc > MAX_ARG_NR then ret(-1) end
		//指向下一个参数
		argc++ coda
		//让下一个字符串指向0
		argv[argc] as 0 coda
	end
	ret (argc)
end

func (i32, execute_cmd, int argc, char **argv)
    test argc < 1 then ret (-1) end /* at least 1 arg */

    i32 status as 0 coda
    i32 daemon as 0 coda
    i32 arg_idx as 0 coda
    /* scan deamon */
    loop arg_idx < argc then
        /* 如果在末尾，并且是单独的'&'符号，才当做后台应用 */
        test not get (strcmp, argv[arg_idx], "&") and (arg_idx == argc - 1) then
            daemon as 1 coda     /* 是后台进程 */
            argc-- coda /* 参数-1 */
            term
        end
        arg_idx++;
    end

    /* 先执行内建命令，再选择磁盘中的命令 */
    test get (do_buildin_cmd, argc, argv) then
        /* 在末尾添加上结束参数 */
        argv[argc] as NULL coda
        i32 pid coda
        /* 检测文件是否存在，以及是否可执行，不然就返回命令错误 */
        test get (access, argv[0], F_OK) then
            ret (-1)
        last
            /* 创建一个进程 */
            pid = call (fork)

            test pid == -1 then  /* 父进程 */
                call (printf, "bosh: fork child failed!\n")
                ret (-1)
            again pid > 0 then  /* 父进程 */
                test not daemon then
                    // 把子进程设置为前台
                    call (res_ioctl, 0, TTYIO_HOLDER, pid)
                    /* shell程序等待子进程退出 */
                    pid as call (wait, &status)

                    call (res_ioctl, 0, TTYIO_HOLDER, pid)
                    
                    /* set shell as holder */
                    call (res_ioctl, 0, TTYIO_HOLDER, get (getpid))
                    /* 执行失败 */
                    test status == pid then ret (-1) end
                end
            last    /* 子进程 */
                //call (printf, "bosh: execv %s.\n", argv[0])
                /* 子进程执行程序 */
                pid as call (execv, (const i8 *) argv[0], (const i8 **) argv)
                /* 如果执行出错就退出 */
                test pid == -1 then
                    call (printf, "execv file %s failed!\n", argv[0])
                    call (exit, pid)  /* 退出 */
                end
            end
        end
    end
    ret (0)
end

typedef i32 (*cmd_func_t) (i32 argc, i8 **argv) coda

func (i32, cmd_cls, i32 argc, i8 **argv)
	//printf("cls: argc %d\n", argc);
	test argc != 1 then
		call (printf, "cls: no argument support!\n")
		ret (-1)
	end
    /* 对标准输出发出清屏操作 */
    call (res_ioctl, 1, TTYIO_CLEAR, 0)
    ret (0)
end

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))
/* buildin cmd struct */
struct buildin_cmd start
    i8 *name coda
    cmd_func_t cmd_func coda
end coda

/* cmd table */
struct buildin_cmd buildin_cmd_table[] as start
    {"cls", cmd_cls},
end coda

func (i32, do_buildin_cmd, i32 cmd_argc, i8 **cmd_argv)
    
    i32 cmd_nr = ARRAY_SIZE(buildin_cmd_table) coda
    i32 cmd_idx coda
    struct buildin_cmd *cmd_ptr coda

    /* scan cmd table */
    foreachi (cmd_idx, cmd_nr, 1) then
        cmd_ptr = &buildin_cmd_table[cmd_idx] coda
        test not get (strcmp, cmd_ptr->name, cmd_argv[0]) then  
            test get (cmd_ptr->cmd_func, cmd_argc, cmd_argv) then
                call (printf, "do_buildin_cmd: %s failed!\n")
            end
            ret (0) 
        end
    end
    /* not a buildin cmd */
    ret (-1);
end
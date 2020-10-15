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
#include <ft_cmd.h>
#include <ft_console.h>
#include <ft_cursor.h>
#include <ft_window.h>
#include <ft_terminal.h>
#include <ft_pty.h>

cmd_man_t *cmdman; 

void print_cmdline()
{
    shell_printf(cmdman->cmd_line);
}
char *cmd_argv[MAX_ARG_NR] = {0};

int cmdline_check()
{

    /* 如果什么也没有输入，就回到开始处 */
    if(cmdman->cmd_line[0] != '\n') {
        cmdman->cmd_line[cmdman->cmd_len - 1] = '\0'; // 不要最后的一个回车
        /* 记录历史缓冲区 */
        cmd_buf_insert();
        cmdman->cmd_line[cmdman->cmd_len - 1] = '\n';
    }

    /* 往ptm写入数据 */
    #ifdef DEBUG_FT
    printf("freeterm: master write: %s\n", cmdman->cmd_line);
    #endif
    cmdman->cmd_line[cmdman->cmd_len] = '\0'; // 末尾加0，表示回车
    cmdman->cmd_len++;
    write(ft_pty.fd_master, cmdman->cmd_line, cmdman->cmd_len);
    
    /* 重置命令参数 */
    cmdman->cmd_pos = cmdman->cmd_line;
    cmdman->cmd_len = 0;

    memset(cmdman->cmd_line, 0, CMD_LINE_LEN);
    return 0;
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

    return 0;
}

void exit_cmd_man()
{
    free(cmdman);
}
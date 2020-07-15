#ifndef _BOSH_CMD_H
#define _BOSH_CMD_H

#include <stddef.h>

#define CMD_LINE_LEN 256
#define MAX_ARG_NR 16
#define MAX_PATH_LEN 256

/* 命令缓冲区数量 */
#define CMD_BUF_NR    20

typedef struct {
    char cwd_cache[MAX_PATH_LEN];   /* 当前工作目录 */
    char cmd_line[CMD_LINE_LEN];    /* 通用命令行 */
    char *cmd_pos;                  /* 在命令行中的位置 */
    int cmd_len;                    /* 命令行的长度 */
} cmd_man_t;

#define SIZE_CMD_MAN    sizeof(cmd_man_t)

extern cmd_man_t *cmdman; 

int init_cmd_man();
void exit_cmd_man();
int cmd_parse(char * cmd_str, char **argv, char token);
int execute_cmd(int argc, char **argv);
int do_buildin_cmd(int cmd_argc, char **cmd_argv);
void print_prompt();
void print_cmdline();
int cmd_buf_select(int dir);
void cmd_buf_insert();
int cmdline_set(char *buf, int buflen);
void cmd_loop();

#endif  /* _BOSH_CMD_H */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sgi/sgi.h>
#include "terminal.h"
#include "window.h"
#include "cmd.h"
#include "console.h"

char *cmd_argv[MAX_ARG_NR] = {0};

int main(int argc, char *argv[])
{
    if (init_con_screen() < 0) {
        return -1;
    }
    if (con_open_window() < 0) {
        return -1;
    }

    memset(cmdman->cwd_cache, 0, MAX_PATH_LEN);
    strcpy(cmdman->cwd_cache, "0:/");

    while (1) {
        print_prompt();
        memset(cmdman->cmd_line, 0, CMD_LINE_LEN);
        if (con_event_loop(cmdman->cmd_line, CMD_LINE_LEN) < 0)
            break;
        /* 如果什么也没有输入，就回到开始处 */
		if(cmdman->cmd_line[0] == 0)
			continue;

        /* 处理数据 */
        //printf("cmd: %s\n", cmd_line);
        /* 记录历史缓冲区 */
        cmd_buf_insert();
        
        int argnum = -1;
        argnum = cmd_parse(cmdman->cmd_line, cmd_argv, ' ');
        if(argnum == -1){
            cprintf("%s: num of arguments exceed %d\n", APP_NAME, MAX_ARG_NR);
            continue;
        }
#if 0
        /* 打印参数 */
        int i;
        for (i = 0; i < argnum; i++) {
            printf("arg[%d]=%s\n", i, global_cmd_argv[i]);
        }
#endif
        if (execute_cmd(argnum, cmd_argv)) {
            cprintf("%s: execute cmd %s falied!\n", APP_NAME, cmd_argv[0]);
        }
    }
    
    con_close_window();
    return 0;
}

#include <stdio.h>
#include <xcons.h>
#include <string.h>

#include <sys/input.h>
#include <sys/trigger.h>


#include "cmd.h"
#include "shell.h"

int shell_event_poll(char *buf, int pid)
{
    xcons_msg_t msg;
    memset(&msg, 0, sizeof(xcons_msg_t));
    *buf = 0;
    /* 获取事件 */
    if (xcons_poll_msg(&msg) < 0)
        return -1;

    *buf = msg.data;
    /* 组合按键 */
    if (msg.ctrl & XCONS_KMOD_CTRL) {
        /* ctrl + c -> 打断进程 */
        if (msg.data == KEY_C || msg.data == KEY_c) {
            printf("term\n");
            /* 激活pid的轻软件触发器，可捕捉 */
            triggeron(TRIGLSOFT, pid);
        }
        if (msg.data == KEY_Z || msg.data == KEY_z) {
            printf("pause\n");
            /* 激活pid的轻软件触发器，可捕捉 */
            triggeron(TRIGPAUSE, pid);
        }
        if (msg.data == KEY_X || msg.data == KEY_x) {
            printf("resume\n");
            /* 激活pid的轻软件触发器，可捕捉 */
            triggeron(TRIGRESUM, pid);
        }
    }
    
    return 0;
}


int shell_event_loop()
{
    cmdman->cmd_pos = cmdman->cmd_line;
    cmdman->cmd_len = 0;

    char *buf = cmdman->cmd_line;

    xcons_msg_t msg;
    while ((cmdman->cmd_pos - buf) < CMD_LINE_LEN) {
        memset(&msg, 0, sizeof(xcons_msg_t));
        if (xcons_next_msg(&msg) < 0)
            continue;
        
        /* 过滤一些按键 */
        switch (msg.data) {
        case KEY_UP:
        case KEY_DOWN:
        case KEY_LEFT:
        case KEY_RIGHT:
        case KEY_NUMLOCK:
        case KEY_CAPSLOCK:
        case KEY_SCROLLOCK:
        case KEY_RSHIFT:
        case KEY_LSHIFT:
        case KEY_RCTRL:
        case KEY_LCTRL:
        case KEY_RALT:
        case KEY_LALT:
            break;
        case KEY_ENTER:
            shell_putchar('\n');
            buf[cmdman->cmd_len] = 0;
            /* 发送给命令行 */
            return 0;   /* 执行命令 */
        case KEY_BACKSPACE:
            if(cmdman->cmd_pos > buf){
                --cmdman->cmd_pos;
                *cmdman->cmd_pos = '\0';
                shell_putchar('\b');            
            }
            break;
        default:
            *cmdman->cmd_pos = msg.data;
            shell_putchar(msg.data);
            cmdman->cmd_pos++;
            cmdman->cmd_len++;
            break;
        }
    }
    return 0;
}

void shell_putchar(char ch)
{
    char s[2] = {ch, 0};
    xcons_xmit_data(s, 1);
}

int shell_printf(const char *fmt, ...)
{
	char buf[BUFSIZ];
	va_list arg = (va_list)((char*)(&fmt) + 4); /*4是参数fmt所占堆栈中的大小*/
	vsprintf(buf, fmt, arg);
	
    /* 输出到控制台 */
    xcons_xmit_data(buf, strlen(buf));
	return 0;
}

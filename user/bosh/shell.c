#include <stdio.h>
#include <xcons.h>
#include <string.h>

#include <sys/input.h>
#include <sys/trigger.h>


#include "cmd.h"
#include "shell.h"

int shell_event_poll(char *buf, int pid)
{
    int keybuf[2];

    *buf = 0;
    /* 获取事件 */
    if (xcons_getkey(keybuf, 1) < 0)
        return -1;

    *buf = keybuf[0];
    /* 组合按键 */
    if (keybuf[1] & XCONS_KMOD_CTRL) {
        /* ctrl + c -> 打断进程 */
        if (keybuf[0] == KEY_C || keybuf[0] == KEY_c) {
            /* 激活pid的轻软件触发器，可捕捉 */
            triggeron(TRIGLSOFT, pid);
        }
        if (keybuf[0] == KEY_Z || keybuf[0] == KEY_z) {
            /* 激活pid的轻软件触发器，可捕捉 */
            triggeron(TRIGPAUSE, pid);
        }
        if (keybuf[0] == KEY_X || keybuf[0] == KEY_x) {
            /* 激活pid的轻软件触发器，可捕捉 */
            triggeron(TRIGRESUM, pid);
        }
    }
    
    return 0;
}


int shell_readline()
{
    cmdman->cmd_pos = cmdman->cmd_line;
    cmdman->cmd_len = 0;

    char *buf = cmdman->cmd_line;

    int keybuf[2];
    while ((cmdman->cmd_pos - buf) < CMD_LINE_LEN) {
        
        if (xcons_getkey(keybuf, 0) < 0)
            continue;

        /* 过滤一些按键 */
        switch (keybuf[0]) {
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
            *cmdman->cmd_pos = keybuf[0];
            shell_putchar(keybuf[0]);
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
    //xcons_xmit_data(s, 1);
    xcons_putstr(s, 1);

}

int shell_printf(const char *fmt, ...)
{
	char buf[BUFSIZ];
	va_list arg = (va_list)((char*)(&fmt) + 4); /*4是参数fmt所占堆栈中的大小*/
	vsprintf(buf, fmt, arg);
	
    /* 输出到控制台 */
    //xcons_xmit_data(buf, strlen(buf));
	xcons_putstr(buf, strlen(buf));
    return 0;
}

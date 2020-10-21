#include <xbook/debug.h>
#include <stdarg.h>
#include <string.h>
#include <xbook/spinlock.h>
#include <xbook/config.h>
#include <arch/interrupt.h>
#include <arch/cpu.h>
#include <arch/debug.h>
#include <stdio.h>

#include <gui/console.h>

/*
color fmt: \e[b;fm

Foreground colors
    30	Black
    31	Red
    32	Green
    33	Yellow
    34	Blue
    35	Magenta
    36	Cyan
    37	White
        
Background colors
    40	Black
    41	Red
    42	Green
    43	Yellow
    44	Blue
    45	Magenta
    46	Cyan
    47	White
*/

char *printk_msg[] = {
    "\e[0;35m",     /* Magenta */
    "\e[0;31m",     /* Red */
    "\e[0;33m",     /* Yellow */
    "\e[0;34m",     /* Blue */
    "\e[0;32m",     /* Green */
    "\e[0;37m",     /* White */
    0,
};
/* no color */
#define DEBUG_NONE_COLOR    "\e[0m"

int printk_level = DEFAULT_LOG_LEVEL;

int print_gui_console = 0;

//停机并输出大量信息
void panic(const char *fmt, ...)
{
	char buf[256];

	/* 4 is the size of fmt in the stack */
	va_list arg = (va_list)((char*)&fmt + 4);

	vsprintf(buf, fmt, arg);

	pr_emerg("\npanic: %s", buf);
    #ifdef CONFIG_GUI_PRINT
    if (print_gui_console)
        gui_con_screen.outs("system panic!!!\n");
	#endif
    disable_intr();
	while(1){
		cpu_idle();
	}
}

//断言
void assertion_failure(char *exp, char *file, char *baseFile, int line)
{
	printk(KERN_ERR "\nassert(%s) failed:\nfile: %s\nbase_file: %s\nln: %d",
	exp, file, baseFile, line);

	spin("assertion failure()");
}

//停机显示函数名
void spin(char * functionName)
{
	printk(KERN_NOTICE "spinning in %s", functionName);
    #ifdef CONFIG_GUI_PRINT
    if (print_gui_console)
        gui_con_screen.outs("system spin!!!\n");
    #endif
	disable_intr();
	while(1){
		cpu_idle();
	}
}

void debug_putstr(char *str, int count)
{
    char *s = str;
    while (*s && count > 0){
        debug_putchar(*s++);
        --count;
    }
}

/**
 * printk - 格式化输出
 * @fmt: 格式
 * @...: 参数
 * 
 * 返回缓冲区长度
 */
int printk(const char *fmt, ...)
{
    unsigned long flags;
    save_intr(flags);
    int i;
	char buf[256] = {0,};
	va_list arg = (va_list)((char*)(&fmt) + 4); /*4是参数fmt所占堆栈中的大小*/
	i = vsprintf(buf, fmt, arg);

    int count = i;
    char *p = buf;
    
    int level = -1;
    char show = 1;

    /* 如果显示指明调试等级 */
    if (*p == '<') {
        /* 有完整的调试等级 */
        if (*(p + 1) >= '0' && *(p + 1) <= (DEFAULT_LOG_MAX + '0') && *(p + 2) == '>') {
            level = *(p + 1) - '0'; /* 获取等级 */
            if (level > printk_level) /* 如果等级过低，就不显示 */ 
                show = 0;
            
            /* move print start ptr */
            p += 3;
            count -= 3;
        }
    }
    if (show) {
        
        /* print level first */
        if (level >= 0) {
            char *q = printk_msg[level];
            // 发送颜色代码
            debug_putstr(q, strlen(q));
        }
        
        debug_putstr(p, count);
        /* 如果配置了图形控制台，那么就打印到图形控制台 */

        #ifdef CONFIG_GUI_PRINT
        if (print_gui_console && level < 1 && level >= 0)
            gui_con_screen.outs(p);
        #endif

        if (level >= 0) {
            debug_putstr(DEBUG_NONE_COLOR, 4);    
        }
    }
    restore_intr(flags);
	return i;
}

void dump_value(unsigned long val)
{
    printk(KERN_DEBUG "dump_value: %d\n", val);
}

void dump_buffer(void *buffer, unsigned long len, char factor)
{
    printk(KERN_DEBUG "dump_buffer: addr=%x len=%d factor=%d\n", buffer, len, factor);
    int i;
    if (factor == 1) {
        unsigned char *buf = (unsigned char *)buffer;
        for (i = 0; i < len; i++) {
            printk("%x ", buf[i]);
        }
        printk("\n");    
    } else if (factor == 2) {
        unsigned short *buf = (unsigned short *)buffer;
        for (i = 0; i < len / factor; i++) {
            printk("%x ", buf[i]);
        }
        printk("\n");    
    } else if (factor == 4) {
        unsigned int *buf = (unsigned int *)buffer;
        for (i = 0; i < len / factor; i++) {
            printk("%x ", buf[i]);
        }
        printk("\n");
    }    
}

#include <xbook/debug.h>
#include <xbook/stdarg.h>
#include <xbook/vsprintf.h>
#include <xbook/memops.h>
#include <arch/interrupt.h>
#include <arch/cpu.h>

char *printk_msg[] = {
    "emege: ",
    "alter: ",
    "crit: ",
    "error: ",
    "waring: ",
    "notice: ",
    "info: ",
    "debug: ",
    0,
};

int printk_level = DEFAULT_LOG_LEVEL;

#if CONFIG_DEBUG_METHOD == 1 
extern void init_console_debug();
extern void console_putchar(char ch);
#elif CONFIG_DEBUG_METHOD == 2
extern void init_serial_debug();
extern void serial_putchar(char ch);
#endif  /* CONFIG_DEBUG_METHOD */

//停机并输出大量信息
void panic(const char *fmt, ...)
{
	char buf[256];

	/* 4 is the size of fmt in the stack */
	va_list arg = (va_list)((char*)&fmt + 4);

	vsprintf(buf, fmt, arg);

	printk("\npanic: %s", buf);
	disable_intr();
	while(1){
		cpu_idle();
	}
}

//断言
void assertion_failure(char *exp, char *file, char *baseFile, int line)
{
	printk("\nassert(%s) failed:\nfile: %s\nbase_file: %s\nln%d",
	exp, file, baseFile, line);

	spin("assertion failure()");
}

//停机显示函数名
void spin(char * functionName)
{
	printk("\nspinning in %s", functionName);
	disable_intr();
	while(1){
		cpu_idle();
	}
}
#if CONFIG_DEBUG_METHOD == 1

/**
 * console_print - 控制台格式化输出
 * @fmt: 格式
 * @...: 参数
 * 
 * 返回缓冲区长度
 */
int console_print(const char *fmt, ...)
{
	int i;
	char buf[256];
	va_list arg = (va_list)((char*)(&fmt) + 4); /*4是参数fmt所占堆栈中的大小*/
	i = vsprintf(buf, fmt, arg);

    int count = i;
    char *p = buf;

    
    int level = -1;
    char show = 1;

    /* 如果显示指明调试等级 */
    if (*p == '<') {
        /* 有完整的调试等级 */
        if (*(p + 1) > '0' && *(p + 1) <= '7' && *(p + 2) == '>') {
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
            while (*q)
                console_putchar(*q++);
        }

        while (count-- > 0)
            console_putchar(*p++);
    
    }
	return i;
}

#elif CONFIG_DEBUG_METHOD == 2
/**
 * serial_print - 串口格式化输出
 * @fmt: 格式
 * @...: 参数
 * 
 * 返回缓冲区长度
 */
int serial_print(const char *fmt, ...)
{
	int i;
	char buf[256];
	va_list arg = (va_list)((char*)(&fmt) + 4); /*4是参数fmt所占堆栈中的大小*/
	i = vsprintf(buf, fmt, arg);

    int count = i;
    char *p = buf;
    
    int level = -1;
    char show = 1;

    /* 如果显示指明调试等级 */
    if (*p == '<') {
        /* 有完整的调试等级 */
        if (*(p + 1) >= '0' && *(p + 1) <= '7' && *(p + 2) == '>') {
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
            while (*q)
                serial_putchar(*q++);
        }

        while (count-- > 0)
            serial_putchar(*p++);
    
    }
        
	return i;
}
#endif /* CONFIG_SERIAL_DEBUG */

void dump_value(unsigned long val)
{
    printk("dump_value: %d\n", val);
}

/**
 * init_kernel_debug
 * 
 */
void init_kernel_debug()
{

#if CONFIG_DEBUG_METHOD == 1
    // 初始化控制台
	init_console_debug();
    printk = &console_print;
#elif CONFIG_DEBUG_METHOD == 2
    // 初始化串口
    init_serial_debug();
    printk = &serial_print;
#endif /* CONFIG_DEBUG_METHOD */
}

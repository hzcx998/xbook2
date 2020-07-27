#include <xbook/debug.h>
#include <stdarg.h>
#include <string.h>
#include <xbook/spinlock.h>
#include <arch/interrupt.h>
#include <arch/cpu.h>
#include <arch/debug.h>

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
	printk("\nassert(%s) failed:\nfile: %s\nbase_file: %s\nln: %d",
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

DEFINE_SPIN_LOCK_UNLOCKED(print_lock);

/**
 * printk - 格式化输出
 * @fmt: 格式
 * @...: 参数
 * 
 * 返回缓冲区长度
 */
int printk(const char *fmt, ...)
{
    /* 自旋锁上锁 */
    spin_lock(&print_lock);

	int i;
	char buf[256];
    memset(buf, 0, 256);
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
        #if PRINT_LEVEL_MSG == 1
        /* print level first */
        if (level >= 0) {
            char *q = printk_msg[level];
            while (*q)
                debug_putchar(*q++);
        }
        #endif
        
        while (count-- > 0)
            debug_putchar(*p++);
    
    }
    /* 自旋锁解锁锁 */
    spin_unlock(&print_lock);
    
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

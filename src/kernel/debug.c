#include <xbook/debug.h>
#include <stdarg.h>
#include <string.h>
#include <xbook/spinlock.h>
#include <arch/debug.h>
#include <arch/interrupt.h>
#include <arch/cpu.h>
#include <stdio.h>

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

char *keprint_msg[] = {
    "\e[0;35m",     /* Magenta */
    "\e[0;31m",     /* Red */
    "\e[0;33m",     /* Yellow */
    "\e[0;34m",     /* Blue */
    "\e[0;32m",     /* Green */
    "\e[0;37m",     /* White */
    0,
};

#define DEBUG_NONE_COLOR    "\e[0m" // 清除属性
#define BACKTRACE_LEN   3

int keprint_level = DEFAULT_LOG_LEVEL;

int print_gui_console = 0;

#if PRINT_LOCK == 1
DEFINE_SPIN_LOCK_UNLOCKED(print_spin_lock);
#endif 
void panic(const char *fmt, ...)
{
	char buf[256];
	va_list arg;
    va_start(arg, fmt);
    vsprintf(buf, fmt, arg);
	va_end(arg);
    
    emeprint("\npanic: %s", buf);
    /*
    char *bbuf[BACKTRACE_LEN];
    
    int n = backtrace(bbuf, BACKTRACE_LEN);
    int i; for (i = 0; i < n; i++) {
        dbgprint("%p -> ", bbuf[i]);
    }*/
    interrupt_disable();
    while(1){
		cpu_idle();
	}
}

void assertion_failure(char *exp, char *file, char *baseFile, int line)
{
	keprint(PRINT_ERR "\nassert(%s) failed:\nfile: %s\nbase_file: %s\nln: %d",
	exp, file, baseFile, line);
	spin("assertion failure()");
}

void spin(char * functionName)
{
	keprint(PRINT_NOTICE "spinning in %s", functionName);
	interrupt_disable();
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

int keprint(const char *fmt, ...)
{
    #if PRINT_LOCK == 1
    unsigned long iflags;
    spin_lock_irqsave(&print_spin_lock, iflags);
    #endif
    int i;
	char buf[256] = {0,};
	//va_list arg = (va_list)((char*)(&fmt) + sizeof(size_t)); /*sizeof(long)是参数fmt所占堆栈中的大小*/
	va_list args;
    va_start(args, fmt);
    i = vsprintf(buf, fmt, args);
    va_end(args);
    int count = i;
    char *p = buf;
    int level = -1;
    char show = 1;
    if (*p == '<') {
        if (*(p + 1) >= '0' && *(p + 1) <= (DEFAULT_LOG_MAX + '0') && *(p + 2) == '>') {
            level = *(p + 1) - '0';
            if (level > keprint_level) 
                show = 0;
            p += 3;
            count -= 3;
        }
    }
    if (show) {
        if (level >= 0) {
            #if PRINT_COLOR == 1
            char *q = keprint_msg[level];
            debug_putstr(q, strlen(q));
            #endif
        }
        debug_putstr(p, count);
        if (level >= 0) {
            #if PRINT_COLOR == 1
            debug_putstr(DEBUG_NONE_COLOR, 4);    
            #endif
        }
    }
    #if PRINT_LOCK == 1
	spin_unlock_irqrestore(&print_spin_lock, iflags);
    #endif
    return i;
}

void log_dump_value(unsigned long val)
{
    keprint(PRINT_DEBUG "debug: %d\n", val);
}

void log_dump_buffer(void *buffer, unsigned long len, char factor)
{
    keprint(PRINT_DEBUG "debug: addr=%x len=%d factor=%d\n", buffer, len, factor);
    int i;
    if (factor == 1) {
        unsigned char *buf = (unsigned char *)buffer;
        for (i = 0; i < len; i++) {
            keprint("%x ", buf[i]);
        }
        keprint("\n");    
    } else if (factor == 2) {
        unsigned short *buf = (unsigned short *)buffer;
        for (i = 0; i < len / factor; i++) {
            keprint("%x ", buf[i]);
        }
        keprint("\n");    
    } else if (factor == 4) {
        unsigned int *buf = (unsigned int *)buffer;
        for (i = 0; i < len / factor; i++) {
            keprint("%x ", buf[i]);
        }
        keprint("\n");
    }    
}

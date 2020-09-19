#ifndef _XBOOK_DEBUG_H
#define _XBOOK_DEBUG_H

#include "kernel.h"

/* 默认是调试等级 */

#define KERN_EMERG      "<0>"      /* system is unuseable */
#define KERN_ERR        "<1>"      /* error condition */
#define KERN_WARING     "<2>"      /* waring condition */
#define KERN_NOTICE     "<3>"      /* normal significant */
#define KERN_INFO       "<4>"      /* infomational */
#define KERN_DEBUG      "<5>"      /* debug message */

/* 默认是调试等级 */

#define DEFAULT_LOG_MIN   0
#define DEFAULT_LOG_MAX   5

#define DEFAULT_LOG_LEVEL  5

#define PRINT_LEVEL_MSG   0

extern int printk_level;

//内核打印函数的指针
int printk(const char *fmt, ...);

/* pr_xxx came from linux kernel, thanks for it. */
#define pr_fmt(fmt) fmt

#define pr_emerg(fmt, ...) \
    printk(KERN_EMERG pr_fmt(fmt), ##__VA_ARGS__)
#define pr_alert(fmt, ...) \
    printk(KERN_ALERT pr_fmt(fmt), ##__VA_ARGS__)
#define pr_crit(fmt, ...) \
    printk(KERN_CRIT pr_fmt(fmt), ##__VA_ARGS__)
#define pr_err(fmt, ...) \
    printk(KERN_ERR pr_fmt(fmt), ##__VA_ARGS__)
#define pr_warning(fmt, ...) \
    printk(KERN_WARNING pr_fmt(fmt), ##__VA_ARGS__)
#define pr_warn pr_warning
#define pr_notice(fmt, ...) \
    printk(KERN_NOTICE pr_fmt(fmt), ##__VA_ARGS__)
#define pr_info(fmt, ...) \
    printk(KERN_INFO pr_fmt(fmt), ##__VA_ARGS__)
#define pr_cont(fmt, ...) \
    printk(KERN_CONT fmt, ##__VA_ARGS__)
#define pr_dbg(fmt, ...) \
    printk(KERN_DEBUG fmt, ##__VA_ARGS__)

#define logger(fmt, ...) \
        printk("file:%s line:%d " pr_fmt(fmt), __FILE__, __LINE__, ##__VA_ARGS__)
                
void spin(char * func_name);
void panic(const char *fmt, ...);

void dump_value(unsigned long val);
void dump_buffer(void *buffer, unsigned long len, char factor);

void serial_putchar(char ch);

void init_kernel_debug();

#endif   /*_XBOOK_DEBUG_H*/

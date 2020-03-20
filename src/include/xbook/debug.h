#ifndef _XBOOK_DEBUG_H
#define _XBOOK_DEBUG_H

/*
debug methods：
1 -> console in text mode
2 -> serial send to host machine console
*/
#define CONFIG_DEBUG_METHOD 2

//内核打印函数的指针
int (*printk)(const char *fmt, ...);

void spin(char * func_name);
void panic(const char *fmt, ...);

void init_kernel_debug();

#endif   /*_XBOOK_DEBUG_H*/

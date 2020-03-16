#ifndef _XBOOK_DEBUG_H
#define _XBOOK_DEBUG_H

/*
debug methods：
1 -> console in text mode
2 -> serial send to host machine console
*/
#define CONFIG_DEBUG_METHOD 1

//内核打印函数的指针
int (*printk)(const char *fmt, ...);

//断言
#define CONFIG_ASSERT
#ifdef CONFIG_ASSERT
void AssertionFailure(char *exp, char *file, char *baseFile, int line);
#define ASSERT(exp)  if (exp) ; \
        else AssertionFailure(#exp, __FILE__, __BASE_FILE__, __LINE__)
#else
#define ASSERT(exp)
#endif

void spin(char * func_name);
void panic(const char *fmt, ...);

void init_kernel_debug();

#endif   /*_XBOOK_DEBUG_H*/

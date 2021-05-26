#ifndef __STDIO_H__
#define __STDIO_H__

#define STDIN 0
#define STDOUT 1
#define STDERR 2

//#define TEST_START(x) puts(x)
#define TEST_START(x) puts("========== START ");puts(x);puts(" ==========\n");
#define TEST_END(x) puts("========== END ");puts(x);puts(" ==========\n");

#define stdin STDIN
#define stdout STDOUT
#define stderr STDERR

#define va_start(ap, last) (__builtin_va_start(ap, last))
#define va_arg(ap, type) (__builtin_va_arg(ap, type))
#define va_end(ap) (__builtin_va_end(ap))
#define va_copy(d, s) (__builtin_va_copy(d, s))

typedef __builtin_va_list va_list;
typedef unsigned long int uintmax_t;
typedef long int intmax_t;

int getchar();
int putchar(int);
int puts(const char *s);
void printf(const char *fmt, ...);

#endif // __STDIO_H__

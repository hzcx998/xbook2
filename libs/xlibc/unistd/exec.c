#include <unistd.h>
#include <sys/types.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/proc.h>
#include <sys/syscall.h>

extern char **environ;

int execve(const char *pathname, char *const argv[], char *const envp[])
{
	return syscall3(int, SYS_EXECVE, pathname, argv, envp);
}

int execle(const char *pathname, char *const envp[], const char *arg, ...)
{
    va_list parg = (va_list)(&arg);
	const char **p = (const char **) parg;
	return execve(pathname, (char *const *) p, envp);
}

int execv(const char *pathname, char *const argv[])
{
    return execve(pathname, argv, environ);
}

int execl(const char *pathname, const char *arg, ...)
{
	va_list parg = (va_list)(&arg);
	const char **p = (const char **) parg;
	return execv(pathname, (char *const *)p);
}

int execvp(const char *filename, char *const argv[])
{
    /* TODO: 将文件名转换成路径名：
    读取系统变量PATH，进行转换 */
    return execv(filename, argv);
}

int execlp(const char *filename, const char *arg, ...)
{
    va_list parg = (va_list)(&arg);
	const char **p = (const char **) parg;
    return execvp(filename, (char *const *)p);
}
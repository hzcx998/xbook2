#include <unistd.h>
#include <types.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/proc.h>
#include <sys/kfile.h>

/**
 * execv - 执行程序，替换当前进程镜像
 * 
 */
int execv(const char *path, const char *argv[])
{
    int rdbytes;
    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        return -1;
    }
    int filesz = lseek(fd, 0, SEEK_END);
    unsigned char *buf = malloc(filesz);
    if (buf == NULL) {
        close(fd);
        return -1;
    }
    lseek(fd, 0, SEEK_SET);
    rdbytes = read(fd, buf, filesz);    
    if (rdbytes != filesz) {
        close(fd);
        free(buf);
        return -1;
    }
    close(fd);
    kfile_t file;
    file.file = buf;
    file.size = rdbytes;
    char *name = strrchr(path, '/');
    if (name == NULL) {
        name = (char *) path;
    } else {
        name++;
        if (name[0] == 0) { /* 后面没有名字 */
            name = (char *) path;
        }
    }
    execfile(name, &file, (char **) argv);
    return -1;
}

int execl(const char *path, const char *arg, ...)
{
	va_list parg = (va_list)(&arg);
	const char **p = (const char **) parg;
	return execv(path, p);
}

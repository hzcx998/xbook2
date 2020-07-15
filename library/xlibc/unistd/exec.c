#include <unistd.h>
#include <types.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/proc.h>
#include <sys/trigger.h>
#include <sys/kfile.h>

/**
 * execv - 执行程序，替换当前进程镜像
 * 
 */
int execv(const char *path, const char *argv[])
{
    /* 设置轻软件触发器为忽略，避免在执行过程中被打断 */
    trigger(TRIGLSOFT, TRIG_IGN);

    int rdbytes;
    int fd = open(path, O_RDONLY, 0);
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
    /* 恢复默认 */
    trigger(TRIGLSOFT, TRIG_DFL);

    execfile(name, &file, (char **) argv);
    return -1;
}

int execl(const char *path, const char *arg, ...)
{
	va_list parg = (va_list)(&arg);
	const char **p = (const char **) parg;
	return execv(path, p);
}

#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/res.h>

errno_t __errno = 0;

static char *__errno_string[] = {
    "ENULL", 
    "EPERM", 
    "ENOFILE or ENOENT", 
    "ESRCH", 
    "EINTR", 
    "EIO", 
    "ENXIO", 
    "E2BIG", 
    "ENOEXEC", 
    "EBADF", 
    "ECHILD", 
    "EAGAIN", 
    "ENOMEM", 
    "EACCES", 
    "EFAULT", 
    "Unknown", 
    "EBUSY", 
    "EEXIST", 
    "EXDEV", 
    "ENODEV", 
    "ENOTDIR", 
    "EISDIR", 
    "EINVAL", 
    "ENFILE", 
    "EMFILE", 
    "ENOTTY", 
    "Unknown", 
    "EFBIG", 
    "ENOSPC", 
    "ESPIPE", 
    "EROFS", 
    "EMLINK", 
    "EPIPE", 
    "EDOM", 
    "ERANGE", 
    "Unknown", 
    "EDEADLOCK", 
    "EDEADLK", 
    "Unknown", 
    "ENAMETOOLONG", 
    "ENOLCK", 
    "ENOSYS", 
    "ENOTEMPTY", 
    "EILSEQ", 
};

extern int *_errno(void);

#define errno (*_errno())

errno_t _set_errno(int value)
{
    __errno = value;
    return __errno;
}
errno_t _get_errno(int *value)
{
    if (value) 
        *value = __errno;
    return __errno;
}

char *strerror(int errnum)
{
    if (errnum > 0 && errnum < EMAXNR) {
        return __errno_string[errnum];
    }
    return __errno_string[0];
}

void perror(char *str)
{
    char *error = strerror(__errno);
    res_write(RES_STDERRNO, 0, str, strlen(str));
    res_write(RES_STDERRNO, 0, ": ", 2);
    res_write(RES_STDERRNO, 0, error, strlen(error));
}

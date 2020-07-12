#ifndef _XLIBC_UTIME_H
#define _XLIBC_UTIME_H

#include <types.h>
#include <stddef.h>

struct utimbuf {
    time_t actime;      /* access time */
    time_t modtime;     /* modification time */
};

int utime(const char *pathname, const struct utimbuf *times);

#endif  /* _XLIBC_UTIME_H */

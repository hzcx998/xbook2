#ifndef _XLIBC_UTIME_H
#define _XLIBC_UTIME_H

#ifdef __cplusplus
extern "C" {
#endif

#include <types.h>
#include <stddef.h>

struct utimbuf {
    time_t actime;      /* access time */
    time_t modtime;     /* modification time */
};

int utime(const char *pathname, const struct utimbuf *times);

#ifdef __cplusplus
}
#endif

#endif  /* _XLIBC_UTIME_H */

#ifndef _SYS_UIO_H
#define _SYS_UIO_H

#include <stddef.h>

#define IOVEC_NR_MAX    4

struct iovec {
    void  *iov_base;    /* Starting address */
    size_t iov_len;     /* Number of bytes to transfer */
};

#endif  /* _SYS_UIO_H */
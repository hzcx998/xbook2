#ifndef _SYS_UIO_H
#define _SYS_UIO_H

#include <stddef.h>

struct iovec {
    void  *iov_base;    /* Starting address */
    size_t iov_len;     /* Number of bytes to transfer */
};

ssize_t readv(int fd, const struct iovec *iov, int iovcnt);
ssize_t writev(int fd, const struct iovec *iov, int iovcnt);

#endif  /* _SYS_UIO_H */
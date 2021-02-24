
#ifndef _XLIBC_FCNTL_H
#define _XLIBC_FCNTL_H

#include <stdint.h>

#ifndef F_DUPFD
#define F_DUPFD 0
#endif

#ifndef F_GETFD
#define F_GETFD 1
#endif

#ifndef F_SETFD
#define F_SETFD 2
#endif

#ifndef F_GETFL
#define F_GETFL 3
#endif

#ifndef F_SETFL
#define F_SETFL 4
#endif

#ifndef FD_NCLOEXEC
#define FD_NCLOEXEC    0
#endif

#ifndef FD_CLOEXEC
#define FD_CLOEXEC 1
#endif

#endif  /* _XLIBC_FCNTL_H */

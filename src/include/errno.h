/* 错误号 */
#ifndef _LIB_ERRNO_H
#define _LIB_ERRNO_H

#define EPERM        1  /* Operation not permitted */
#define ENOFILE      2  /* No such file or directory */
#define ENOENT       2
#define ESRCH        3  /* No such process */
#define EINTR        4  /* Interrupted function call */
#define EIO          5  /* Input/output error */
#define ENXIO        6  /* No such device or address */
#define E2BIG        7  /* Arg list too long */
#define ENOEXEC      8  /* Exec format error */
#define EBADF        9  /* Bad file descriptor */
#define ECHILD      10  /* No child processes */
#define EAGAIN      11  /* Resource temporarily unavailable */
#define ENOMEM      12  /* Not enough space */
#define EACCES      13  /* Permission denied */
#define EFAULT      14  /* Bad address */
/* 15 - Unknown Error */
#define EBUSY       16  /* strerror reports "Resource device" */
#define EEXIST      17  /* File exists */
#define EXDEV       18  /* Improper link (cross-device link?) */
#define ENODEV      19  /* No such device */
#define ENOTDIR     20  /* Not a directory */
#define EISDIR      21  /* Is a directory */
#define EINVAL      22  /* Invalid argument */
#define ENFILE      23  /* Too many open files in system */
#define EMFILE      24  /* Too many open files */
#define ENOTTY      25  /* Inappropriate I/O control operation */
/* 26 - Unknown Error */
#define EFBIG       27  /* File too large */
#define ENOSPC      28  /* No space left on device */
#define ESPIPE      29  /* Invalid seek (seek on a pipe?) */
#define EROFS       30  /* Read-only file system */
#define EMLINK      31  /* Too many links */
#define EPIPE       32  /* Broken pipe */
#define EDOM        33  /* Domain error (math functions) */
#define ERANGE      34  /* Result too large (possibly too small) */
/* 35 - Unknown Error */
#define EDEADLOCK   36  /* Resource deadlock avoided (non-Cyg) */
#define EDEADLK     36
/* 37 - Unknown Error */
#define ENAMETOOLONG    38  /* Filename too long (91 in Cyg?) */
#define ENOLCK      39  /* No locks available (46 in Cyg?) */
#define ENOSYS      40  /* Function not implemented (88 in Cyg?) */
#define ENOTEMPTY   41  /* Directory not empty (90 in Cyg?) */
#define EILSEQ      42  /* Illegal byte sequence */

#define EMAXNR      (EILSEQ + 1)  /* Max number */

/* POSIX SUPPLEMENT */
#define EADDRINUSE      100
#define EADDRNOTAVAIL   101
#define EAFNOSUPPORT    102
#define EALREADY        103
#define EBADMSG         104
#define ECANCELED       105
#define ECONNABORTED    106
#define ECONNREFUSED    107
#define ECONNRESET      108
#define EDESTADDRREQ    109
#define EHOSTUNREACH    110
#define EIDRM           111
#define EINPROGRESS     112
#define EISCONN         113
#define ELOOP           114
#define EMSGSIZE        115
#define ENETDOWN        116
#define ENETRESET       117
#define ENETUNREACH     118
#define ENOBUFS         119
#define ENODATA         120
#define ENOLINK         121
#define ENOMSG          122
#define ENOPROTOOPT     123
#define ENOSR           124
#define ENOSTR          125
#define ENOTCONN        126
#define ENOTRECOVERABLE 127
#define ENOTSOCK        128
#define ENOTSUP         129
#define EOPNOTSUPP      130
#define EOTHER          131
#define EOVERFLOW       132
#define EOWNERDEAD      133
#define EPROTO          134
#define EPROTONOSUPPORT 135
#define EPROTOTYPE      136
#define ETIME           137
#define ETIMEDOUT       138
#define ETXTBSY         139
#define EWOULDBLOCK     140

#endif  /* _LIB_ERRNO_H */

#ifndef _SYS_SRVCALL_H
#define _SYS_SRVCALL_H

/* 参数个数 */
#define SRVCALL_ARG_NR  5

typedef struct _srvcall_arg {
    unsigned long data[SRVCALL_ARG_NR];
} srvcall_arg_t;


#endif   /* _SYS_SRVCALL_H */
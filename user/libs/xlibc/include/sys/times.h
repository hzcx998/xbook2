#ifndef _SYS_TIMES_H
#define _SYS_TIMES_H

#include <types.h>

#ifdef __cplusplus
extern "C" {
#endif

struct tms {
    clock_t tms_utime; //用户CPU时间
    clock_t tms_stime; //系统CPU时间
    clock_t tms_cutime; //以终止子进程的用户CPU时间
    clock_t tms_cstime; //已终止子进程的系统CPU时间
};

clock_t times(struct tms *buf);

#ifdef __cplusplus
}
#endif

#endif  /* _SYS_TIMES_H */

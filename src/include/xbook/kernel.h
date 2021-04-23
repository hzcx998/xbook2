#ifndef _XBOOK_KERNEL_H
#define _XBOOK_KERNEL_H

#include <arch/page.h>
#include <stddef.h>

/* 系统信息 */
#define KERNEL_NAME_LEN  32

#define KERN_VADDR      KERN_BASE_VIR_ADDR

/* sys config names */
enum {
    _SC_ARG_MAX = 0,
    _SC_CHILD_MAX,
    _SC_HOST_NAME_MAX,
    _SC_LOGIN_NAME_MAX,
    _SC_NGROUPS_MAX,
    _SC_CLK_TCK,
    _SC_OPEN_MAX,
    _SC_PAGESIZE,
    _SC_PAGE_SIZE,
    _SC_RE_DUP_MAX,
    _SC_STREAM_MAX,
    _SC_SYMLOOP_MAX,
    _SC_TTY_NAME_MAX,
    _SC_TZNAME_MAX,
    _SC_VERSION,
};

long sys_sysconf(int name);
int sys_gethostname(char *name, size_t len);

#endif   /* _XBOOK_KERNEL_H */

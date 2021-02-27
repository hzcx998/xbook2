#ifndef _XBOOK_CONFIG_H
#define _XBOOK_CONFIG_H

/* config address width */
#define CONFIG_32BIT
/* #define CONFIG_64BIT */

/* config large alloc size in memcache */
#define CONFIG_LARGE_ALLOCS

/* auto select timezone */
#define CONFIG_TIMEZONE_AUTO

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

#endif   /* _XBOOK_CONFIG_H */

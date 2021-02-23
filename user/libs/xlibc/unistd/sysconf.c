#include <unistd.h>

long sysconf(int name)
{
    switch (name) {
    case _SC_ARG_MAX:
        return 32;
    case _SC_CHILD_MAX:
        return 1;
    case _SC_HOST_NAME_MAX:
        return 32;
    case _SC_LOGIN_NAME_MAX:
        return 32;
    case _SC_NGROUPS_MAX:
        return 32;
    case _SC_CLK_TCK:
        return 1000;
    case _SC_OPEN_MAX:
        return 32;
    case _SC_PAGESIZE:
        return 4096;
    case _SC_PAGE_SIZE:
        return 4096;
    case _SC_RE_DUP_MAX:
        return 32;
    case _SC_STREAM_MAX:
        return 8;
    case _SC_SYMLOOP_MAX:
        return 8;
    case _SC_TTY_NAME_MAX:
        return 9;
    case _SC_TZNAME_MAX:
        return 6;
    case _SC_VERSION:
        return 199009L;
    default:
        break;
    }
    return -1;
}

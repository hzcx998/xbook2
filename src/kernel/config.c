#include <xbook/kernel.h>
#include <xbook/task.h>
#include <xbook/clock.h>
#include <xbook/driver.h>
#include <xbook/safety.h>
#include <sys/uname.h>
#include <errno.h>
#include <stddef.h>

long sys_sysconf(int name)
{
    switch (name) {
    case _SC_ARG_MAX:
        return MAX_TASK_STACK_ARG_NR;
    case _SC_CHILD_MAX:
        return 1;
    case _SC_HOST_NAME_MAX:
        return 32;
    case _SC_LOGIN_NAME_MAX:
        return 32;
    case _SC_NGROUPS_MAX:
        return 32;
    case _SC_CLK_TCK:
        return HZ;
    case _SC_OPEN_MAX:
        return LOCAL_FILE_OPEN_NR;
    case _SC_PAGESIZE:
        return PAGE_SIZE;
    case _SC_PAGE_SIZE:
        return PAGE_SIZE;
    case _SC_RE_DUP_MAX:
        return 32;
    case _SC_STREAM_MAX:
        return 8;
    case _SC_SYMLOOP_MAX:
        return 8;
    case _SC_TTY_NAME_MAX:
        return DEVICE_NAME_LEN;
    case _SC_TZNAME_MAX:
        return 6;
    case _SC_VERSION:   /* posix version */
        return 199009L;
    default:
        break;
    }
    return -1;
}

int sys_gethostname(char *name, size_t len)
{
    if (!name || !len)
        return -EINVAL;
    return mem_copy_to_user(name, KERNEL_NAME, min(len, KERNEL_NAME_LEN));
}

int sys_uname(struct utsname *buf)
{
    struct utsname names;
    memset(&names, 0, sizeof(struct utsname));
    #if defined(__X86__)
    strcpy(names.machine, "x86");
    #elif defined(__RISCV64__)
    strcpy(names.machine, "riscv64");
    #else
    strcpy(names.machine, "unknown");
    #endif
    strcpy(names.nodename, "localhost");
    strcpy(names.domainname, "localhost");
    strcpy(names.version, KERNEL_VERSION);
    strcpy(names.release, KERNEL_VERSION);
    strcpy(names.sysname, KERNEL_NAME);
    if (mem_copy_to_user(buf, &names, sizeof(names)) < 0)
        return -EPERM;
    
    infoprintln("Uname: %s %s %s %s %s %s\n", 
		names.sysname, names.nodename, names.release, names.version, names.machine, names.domainname);
    return 0;
}
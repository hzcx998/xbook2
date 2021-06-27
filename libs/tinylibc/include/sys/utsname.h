#ifndef __SYS_UTSNAME_H__
#define __SYS_UTSNAME_H__

#include <stddef.h>

struct utsname {
	char sysname[65];
	char nodename[65];
	char release[65];
	char version[65];
	char machine[65];
	char domainname[65];
};

int uname(struct utsname *buf);

#endif //__SYS_UTSNAME_H__

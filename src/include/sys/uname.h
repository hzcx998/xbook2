#ifndef _SYS_UNAME_H
#define _SYS_UNAME_H

struct utsname {
	char sysname[65];
	char nodename[65];
	char release[65];
	char version[65];
	char machine[65];
	char domainname[65];
};

int sys_uname(struct utsname *buf);

#endif   /* _SYS_UNAME_H */
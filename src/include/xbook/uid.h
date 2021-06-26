
/*
内核中各种ID的管理
*/
#ifndef _XBOOK_UID_H
#define _XBOOK_UID_H

#include <stddef.h>
#include <types.h>

#define ROOT_UID    0

#define NGROUPS_MAX 32

uid_t sys_getuid(void);
uid_t sys_geteuid(void);
gid_t sys_getgid(void);
gid_t sys_getegid(void);

int sys_setgid(gid_t gid);
int sys_setuid(uid_t uid);
int sys_setegid(gid_t egid);
int sys_seteuid(uid_t euid);

int sys_getgroups(int size, gid_t list[]);
int sys_setgroups(size_t size, const gid_t *list);

int sys_setresuid(uid_t ruid, uid_t euid, uid_t suid);
int sys_getresuid(uid_t *ruid, uid_t *euid, uid_t *suid);
int sys_setresgid(gid_t rgid, gid_t egid, gid_t sgid);
int sys_getresgid(gid_t *rgid, gid_t *egid, gid_t *sgid);

#endif /* _XBOOK_UID_H */

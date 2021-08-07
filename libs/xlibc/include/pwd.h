#ifndef _PWD_H
#define _PWD_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

struct passwd
{
	char *pw_name;  /*用户帐号*/
	char *pw_passwd;    /*用户密码*/
	uid_t pw_uid;       /*用户识别码*/
	gid_t pw_gid;       /*组识别码*/
	char *pw_gecos;  /*用户全名*/
	char *pw_dir;     /*家目录*/
	char *pw_shell;   /*所使用的shell的路径*/
};

int getpw(uid_t uid,char *buf);
struct passwd *getpwent(void);
void setpwent(void);
void endpwent(void);

struct passwd *getpwuid(uid_t uid);
struct passwd *getpwnam(const char *name);

#ifdef __cplusplus
}
#endif

#endif /* _PWD_H */
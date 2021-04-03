#include <pwd.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define __PWD_PATH	"/etc/passwd"

static char *pwd_buffer = NULL;
static char *pwd_ptr = NULL;
static struct passwd __pwd;

static int __pw_load()
{
	int pwdfd = -1;
	if (pwd_buffer == NULL) {
		pwdfd = open(__PWD_PATH, O_RDONLY);
		if (pwdfd < 0)
			return -1;
		lseek(pwdfd, 0, SEEK_END);
		long sz = tell(pwdfd);
		if (sz <= 0) {
			clsoe(pwdfd);
			pwdfd = -1;
			return -1;
		}
		lseek(pwdfd, 0, SEEK_END);

		pwd_buffer = malloc(sz);
		if (pwd_buffer == NULL) {
			clsoe(pwdfd);
			pwdfd = -1;
			return -1;
		}
		if (read(pwdfd, pwd_buffer, sz) <= 0) {
			free(pwd_buffer);
			pwd_buffer = NULL;
			clsoe(pwdfd);
			pwdfd = -1;
			return -1;
		}
		clsoe(pwdfd);
		pwdfd = -1;
		pwd_ptr = pwd_buffer;
	}
	return 0;
}


int getpw(uid_t uid, char *buf)
{
    strcpy(buf, "1234");
    return 0;
}

struct passwd *getpwent(void)
{
	if (__pw_load() < 0)
		return NULL;
	/* 读取并放到结构体中，返回结构体内容 */
	
    return NULL;
}

void setpwent(void)
{

}

void endpwent(void)
{

}

struct passwd *getpwnam(const char *name)
{
	struct passwd *ptr;
	setpwent();
	while((ptr = getpwent()) != NULL) {
		if(strcmp(name, ptr->pw_name) == 0) {
			break;
		}
	}
	endpwent();
	return(ptr);
}

struct passwd *getpwuid(uid_t uid)
{
	struct passwd *ptr;
	setpwent();
	while((ptr = getpwent()) != NULL) {
		if(uid == ptr->pw_uid) {
			break;
		}
	}
	endpwent();
	return(ptr);
}
#include <pwd.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <errno.h>

// #define _HAVE_SCONF_lIB

#ifdef _HAVE_SCONF_lIB
#include <sconf.h>
#endif

#define __PWD_PATH	"/etc/passwd"

#define __PWD_SEPARATOR	':'

#define __PWD_LEN	64
#define __PWD_BUF_LEN	(__PWD_LEN + 1)

static char *pwd_buffer = NULL;
static char *pwd_ptr = NULL;
static struct passwd __pwd = {NULL, NULL, 0, 0, NULL, NULL, NULL};

static int __pw_load()
{
	if (pwd_buffer == NULL) {
        int pwdfd = -1;
		pwdfd = open(__PWD_PATH, O_RDONLY);
		if (pwdfd < 0) {
            _set_errno(ENFILE);
			return -1;
        }
		lseek(pwdfd, 0, SEEK_END);
		long sz = tell(pwdfd);
		if (sz <= 0) {
            _set_errno(ENFILE);
			close(pwdfd);
			pwdfd = -1;
			return -1;
		}
		lseek(pwdfd, 0, SEEK_SET);

		pwd_buffer = malloc(sz);
		if (pwd_buffer == NULL) {
            _set_errno(ENOMEM);
			close(pwdfd);
			pwdfd = -1;
			return -1;
		}
		if (read(pwdfd, pwd_buffer, sz) <= 0) {
            _set_errno(EIO);
			free(pwd_buffer);
			pwd_buffer = NULL;
			close(pwdfd);
			pwdfd = -1;
			return -1;
		}
		close(pwdfd);
        pwdfd = -1;

		pwd_ptr = pwd_buffer;
	}
    /* init pwd struct */
    if (!__pwd.pw_name && !__pwd.pw_passwd) {
        __pwd.pw_uid = 0;
        __pwd.pw_gid = 0;
        assert((__pwd.pw_name = malloc(__PWD_LEN)));
        assert((__pwd.pw_passwd = malloc(__PWD_LEN)));
        assert((__pwd.pw_gecos = malloc(__PWD_LEN)));
        assert((__pwd.pw_dir = malloc(MAX_PATH)));
        assert((__pwd.pw_shell = malloc(MAX_PATH)));
    }
	return 0;
}

static int __pw_free()
{
	if (pwd_buffer) {
        free(pwd_buffer);
        pwd_buffer = NULL;
		pwd_ptr = NULL;
        return 0;
    }
	return -1;
}

static void __pw_line(char *line)
{
    if (!line)
        return;
    #ifdef _HAVE_SCONF_lIB
    char old_separator = sconf_get_separator();
    sconf_set_separator(__PWD_SEPARATOR);
    char buf[__PWD_BUF_LEN] = {0};
    char *p = line;
    /* 1. name */
    assert((p = sconf_read(p, buf, __PWD_BUF_LEN)));
    strcpy(__pwd.pw_name, buf);

    /* 2. password */
    memset(buf, 0, __PWD_BUF_LEN);
    assert((p = sconf_read(p, buf, __PWD_BUF_LEN)));
    strcpy(__pwd.pw_passwd, buf);
    /* 3. uid */
    memset(buf, 0, __PWD_BUF_LEN);
    assert((p = sconf_read(p, buf, __PWD_BUF_LEN)));
    __pwd.pw_uid = atoi(buf);
    /* 4. gid */
    memset(buf, 0, __PWD_BUF_LEN);
    assert((p = sconf_read(p, buf, __PWD_BUF_LEN)));
    __pwd.pw_gid = atoi(buf);
    /* 5. gecos */
    memset(__pwd.pw_gecos, 0, __PWD_LEN);
    assert((p = sconf_read(p, __pwd.pw_gecos, __PWD_LEN)));

    /* 6. dir */
    memset(__pwd.pw_dir, 0, MAX_PATH);
    assert((p = sconf_read(p, __pwd.pw_dir, MAX_PATH)));

    /* 7. shell */
    memset(__pwd.pw_shell, 0, MAX_PATH);
    assert((p = sconf_read(p, __pwd.pw_shell, MAX_PATH)));

    /* restore old separator */
    sconf_set_separator(old_separator);
    #endif
}

int getpw(uid_t uid, char *buf)
{
    struct passwd *ptr;
	setpwent();
	while((ptr = getpwent()) != NULL) {
		if(uid == ptr->pw_uid) {
			strcpy(buf, ptr->pw_name);
	        endpwent();
            return 0;
		}
	}
	endpwent();
    return -1;
}

struct passwd *getpwent(void)
{
    if (!pwd_ptr) {
        return NULL;
    }
	/* 读取并放到结构体中，返回结构体内容 */
	char pw_line[256] = {0};
    #ifdef _HAVE_SCONF_lIB
    pwd_ptr = sconf_readline(pwd_ptr, pw_line, 256);
    #endif
    if (pwd_ptr == NULL)
        return NULL;
    __pw_line(pw_line);
    return &__pwd;
}

void setpwent(void)
{
    /* reload file */
    if (__pw_load() < 0)
        perror("pwd: __pw_load failed!\n");
}

void endpwent(void)
{
    /* free memory */
    if (__pw_free() < 0)
        perror("pwd: __pw_free failed!\n");
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
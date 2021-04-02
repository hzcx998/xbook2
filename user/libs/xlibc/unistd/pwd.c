#include <pwd.h>
#include <string.h>

int getpw(uid_t uid, char *buf)
{
    strcpy(buf, "1234");
    return 0;
}

struct passwd *getpwent(void)
{
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
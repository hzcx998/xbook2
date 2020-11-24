#include <sys/syscall.h>
#include <sys/sys.h>
#include <stddef.h>

int login(const char *name, char *password)
{
    return syscall2(int, SYS_LOGIN, name, password);
}

int logout(const char *name)
{
    return syscall2(int, SYS_LOGIN, name, NULL);
}

int register_account(const char *name, char *password)
{
    return syscall2(int, SYS_REGISTER, name, password);
}

int unregister_account(const char *name)
{
    return syscall2(int, SYS_REGISTER, name, NULL);
}
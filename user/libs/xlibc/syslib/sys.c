#include <sys/syscall.h>
#include <sys/sys.h>
#include <stddef.h>

int login(const char *name, char *password)
{
    return syscall2(int, SYS_ACNTLOGIN, name, password);
}

int logout(const char *name)
{
    return syscall2(int, SYS_ACNTLOGIN, name, NULL);
}

int register_account(const char *name, char *password)
{
    return syscall2(int, SYS_ACNTREGISTER, name, password);
}

int unregister_account(const char *name)
{
    return syscall2(int, SYS_ACNTREGISTER, name, NULL);
}

int accountname(char *buf, size_t buflen)
{
    return syscall2(int, SYS_ACNTNAME, buf, buflen);
}

int accountverify(char *password)
{
    return syscall1(int, SYS_ACNTVERIFY, password);
}
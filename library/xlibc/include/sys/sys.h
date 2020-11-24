#ifndef _SYS_SYS_H
#define _SYS_SYS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

#define SYS_VER_LEN     48

int login(const char *name, char *password);
int logout(const char *name);
int register_account(const char *name, char *password);
int unregister_account(const char *name);
int accountname(char *buf, size_t buflen);
int accountverify(char *password);

#ifdef __cplusplus
}
#endif

#endif  /* _SYS_SYS_H */

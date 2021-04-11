#ifndef __XLIBC_EXIT_H__
#define __XLIBC_EXIT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/proc.h>

void abort(void);
void exit(int status);

#ifdef __cplusplus
}
#endif

#endif /* __XLIBC_EXIT_H__ */

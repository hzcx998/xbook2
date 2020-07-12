#ifndef __XLIBC_EXIT_H__
#define __XLIBC_EXIT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/proc.h>

enum {
	EXIT_SUCCESS	= 0,
	EXIT_FAILURE	= 1,
};

void abort(void);
#define exit    _exit

#ifdef __cplusplus
}
#endif

#endif /* __XLIBC_EXIT_H__ */

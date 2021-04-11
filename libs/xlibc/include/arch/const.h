#ifndef _LIB_ARCH_CONST_H
#define _LIB_ARCH_CONST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <arch/config.h>

#define KB      1024
#define MB      (KB * 1024)
#define GB      (MB * 1024)
#define TB      (GB * 1024)

#define WORDSZ  _WORDSZ

#ifdef __cplusplus
}
#endif

#endif  /* _LIB_ARCH_CONST_H */

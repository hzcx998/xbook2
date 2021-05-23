#ifndef _LIB_ARCH_CONFIG_H
#define _LIB_ARCH_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/* 根据不同的平台选择导入头文件 */
#if defined(__X86__)
#include "x86/xchg.h"
#include "x86/atomic.h"
#include "x86/const.h"
#elif defined(__RISCV64__)

#define __test_and_set __sync_lock_test_and_set
#endif

#ifdef __cplusplus
}
#endif


#endif  /* _LIB_ARCH_CONFIG_H */

#ifndef _LIB_ARCH_ATOMIC_H
#define _LIB_ARCH_ATOMIC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <arch/config.h>
#include "xchg.h"

/* 原子变量结构体 */
typedef struct {
   int value;   // 变量的值
} atomic_t;

#define atomic_get(atomic)	((atomic)->value)
#define atomic_set(atomic,val)	(((atomic)->value) = (val))

#define ATOMIC_INIT(val)	{val}

/**
 * AtomicAdd - 原子加运算
 * @atomic: 原子结构
 * @value: 数值
 */
static inline void atomic_add(atomic_t *atomic, int value)
{
   mem_atomic_add(&atomic->value, value);
}

/**
 * AtomicSub - 原子减运算
 * @atomic: 原子结构
 * @value: 数值
 */
static inline void atomic_sub(atomic_t *atomic, int value)
{
   mem_atomic_sub(&atomic->value, value);
}


/**
 * AtomicInc - 原子增运算
 * @atomic: 原子结构
 */
static inline void atomic_inc(atomic_t *atomic)
{
   mem_atomic_inc(&atomic->value);
}

/**
 * AtomicDec - 原子减运算
 * @atomic: 原子结构
 */
static inline void atomic_dec(atomic_t *atomic)
{
   mem_atomic_dec(&atomic->value);
}

/**
 * AtomicSetMask - 设置位
 * @atomic: 原子结构
 * @mask: 位值
 */
static inline void atomic_set_mask(atomic_t *atomic, int mask)
{
   mem_atomic_or(&atomic->value, mask);
}

/**
 * AtomicClearMask - 清除位
 * @atomic: 原子结构
 * @mask: 位值
 */
static inline void atomic_clear_mask(atomic_t *atomic, int mask)
{
   mem_atomic_and(&atomic->value, ~mask);
}

#define atomic_xchg(v, new) (test_and_set(&((v)->value), new))

#ifdef __cplusplus
}
#endif


#endif  /* _LIB_ARCH_ATOMIC_H */

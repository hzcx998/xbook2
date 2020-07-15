#ifndef _ARCH_ATOMIC_H
#define _ARCH_ATOMIC_H

#include <arch/instruction.h>

/* 原子变量结构体 */
typedef struct {
   int value;   // 变量的值
} atomic_t;

#define atomic_get(atomic)	((atomic)->value)
#define atomic_set(atomic,val)	(((atomic)->value) = (val))

#define ATOMIC_INIT(val)	{val}


void __atomic_add(int *a, int b);
void __atomic_sub(int *a, int b);
void __atomic_inc(int *a);
void __atomic_dec(int *a);
void __atomic_or(int *a, int b);
void __atomic_and(int *a, int b);

/**
 * AtomicAdd - 原子加运算
 * @atomic: 原子结构
 * @value: 数值
 */
static inline void atomic_add(atomic_t *atomic, int value)
{
   __atomic_add(&atomic->value, value);
}

/**
 * AtomicSub - 原子减运算
 * @atomic: 原子结构
 * @value: 数值
 */
static inline void atomic_sub(atomic_t *atomic, int value)
{
   __atomic_sub(&atomic->value, value);
}


/**
 * AtomicInc - 原子增运算
 * @atomic: 原子结构
 */
static inline void atomic_inc(atomic_t *atomic)
{
   __atomic_inc(&atomic->value);
}

/**
 * AtomicDec - 原子减运算
 * @atomic: 原子结构
 */
static inline void atomic_dec(atomic_t *atomic)
{
   __atomic_dec(&atomic->value);
}

/**
 * AtomicSetMask - 设置位
 * @atomic: 原子结构
 * @mask: 位值
 */
static inline void atomic_set_mask(atomic_t *atomic, int mask)
{
   __atomic_or(&atomic->value, mask);
}

/**
 * AtomicClearMask - 清除位
 * @atomic: 原子结构
 * @mask: 位值
 */
static inline void atomic_clear_mask(atomic_t *atomic, int mask)
{
   __atomic_and(&atomic->value, ~mask);
}

#define atomic_xchg(v, new) (xchg(&((v)->value), new))

#endif   /* _ARCH_ATOMIC_H */

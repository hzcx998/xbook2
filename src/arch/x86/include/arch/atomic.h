#ifndef _X86_ATOMIC_H
#define _X86_ATOMIC_H

#include <arch/memory.h>

/* 原子变量结构体 */
typedef struct {
   int value;   // 变量的值
} atomic_t;

#define atomic_get(atomic)	((atomic)->value)
#define atomic_set(atomic,val)	(((atomic)->value) = (val))

#define ATOMIC_INIT(val)	{val}


void mem_atomic_add(int *a, int b);
void mem_atomic_sub(int *a, int b);
void mem_atomic_inc(int *a);
void mem_atomic_dec(int *a);
void mem_atomic_or(int *a, int b);
void mem_atomic_and(int *a, int b);

/**
 * atomic_add - 原子加运算
 * @atomic: 原子结构
 * @value: 数值
 */
static inline void atomic_add(atomic_t *atomic, int value)
{
   mem_atomic_add(&atomic->value, value);
}

/**
 * atomic_sub - 原子减运算
 * @atomic: 原子结构
 * @value: 数值
 */
static inline void atomic_sub(atomic_t *atomic, int value)
{
   mem_atomic_sub(&atomic->value, value);
}


/**
 * atomic_inc - 原子增运算
 * @atomic: 原子结构
 */
static inline void atomic_inc(atomic_t *atomic)
{
   mem_atomic_inc(&atomic->value);
}

/**
 * atomic_dec - 原子减运算
 * @atomic: 原子结构
 */
static inline void atomic_dec(atomic_t *atomic)
{
   mem_atomic_dec(&atomic->value);
}

/**
 * atomic_set_mask - 设置位
 * @atomic: 原子结构
 * @mask: 位值
 */
static inline void atomic_set_mask(atomic_t *atomic, int mask)
{
   mem_atomic_or(&atomic->value, mask);
}

/**
 * atomic_clear_mask - 清除位
 * @atomic: 原子结构
 * @mask: 位值
 */
static inline void atomic_clear_mask(atomic_t *atomic, int mask)
{
   mem_atomic_and(&atomic->value, ~mask);
}

#define atomic_xchg(v, new) (xchg(&((v)->value), new))

#endif   /* _X86_ATOMIC_H */

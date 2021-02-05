#ifndef _X86_ATOMIC_H
#define _X86_ATOMIC_H

#include <arch/memory.h>

typedef struct {
   int value;
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

static inline void atomic_add(atomic_t *atomic, int value)
{
   mem_atomic_add(&atomic->value, value);
}

static inline void atomic_sub(atomic_t *atomic, int value)
{
   mem_atomic_sub(&atomic->value, value);
}

static inline void atomic_inc(atomic_t *atomic)
{
   mem_atomic_inc(&atomic->value);
}

static inline void atomic_dec(atomic_t *atomic)
{
   mem_atomic_dec(&atomic->value);
}

static inline void atomic_set_mask(atomic_t *atomic, int mask)
{
   mem_atomic_or(&atomic->value, mask);
}

static inline void atomic_clear_mask(atomic_t *atomic, int mask)
{
   mem_atomic_and(&atomic->value, ~mask);
}

#define atomic_xchg(v, new) (xchg(&((v)->value), new))

#endif   /* _X86_ATOMIC_H */

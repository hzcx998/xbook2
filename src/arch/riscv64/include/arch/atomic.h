#ifndef _RISCV64_ATOMIC_H
#define _RISCV64_ATOMIC_H

typedef struct {
   volatile int value;
} atomic_t;

#define atomic_get(atomic)	((atomic)->value)
#define atomic_set(atomic,val)	(((atomic)->value) = (val))

#define ATOMIC_INIT(val)	{val}

static inline void atomic_add(atomic_t *atomic, int value)
{
   __sync_fetch_and_add(&atomic->value, value);
}

static inline void atomic_sub(atomic_t *atomic, int value)
{
   __sync_fetch_and_sub(&atomic->value, value);
}

static inline void atomic_inc(atomic_t *atomic)
{
   __sync_fetch_and_add(&atomic->value, 1);
}

static inline void atomic_dec(atomic_t *atomic)
{
   __sync_fetch_and_sub(&atomic->value, 1);
}

static inline void atomic_set_mask(atomic_t *atomic, int mask)
{
   __sync_fetch_and_or(&atomic->value, mask);
}

static inline void atomic_clear_mask(atomic_t *atomic, int mask)
{
   __sync_fetch_and_and(&atomic->value, ~mask);
}

#define atomic_xchg(v, new) (__sync_lock_test_and_set(&((v)->value), new))

#endif   /* _RISCV64_ATOMIC_H */

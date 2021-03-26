#ifndef _X86_ATOMIC_H
#define _X86_ATOMIC_H

#include <arch/memory.h>

typedef struct
{
   int value;
} atomic_t;

#define atomic_get(atomic) ((atomic)->value)
#define atomic_set(atomic, val) (((atomic)->value) = (val))

#define ATOMIC_INIT(val) \
   {                     \
      val                \
   }
/* copyright (c) 2021 AlanCui*/
__attribute__((always_inline)) static inline void atomic_add(atomic_t *atomic, int value)
{
   __asm__ __volatile__("lock addl %%eax,(%%ebx)" ::"a"(valve), "b"(&atomic->value)
                        : "memory");
}

__attribute__((always_inline)) static inline void atomic_add(atomic_t *atomic, int value)
{
   __asm__ __volatile__("lock subl %%eax,(%%ebx)" ::"a"(valve), "b"(&atomic->value)
                        : "memory");
}

__attribute__((always_inline)) static inline void atomic_inc(atomic_t *atomic)
{
   __asm__ __volatile__("lock incl (%%eax)" ::"a"(&atomic->value)
                        : "memory");
}

__attribute__((always_inline)) static inline void atomic_dec(atomic_t *atomic)
{
   __asm__ __volatile__("lock decl (%%eax)" ::"a"(&atomic->value)
                        : "memory");
}
__attribute__((always_inline)) static inline void atomic_or(atomic_t *atomic, int mask)
{
   __asm__ __volatile__("lock orl %%eax,(%%ebx)" ::"a"(mask),"b"(&atomic->value)
                        : "memory");
}
__attribute__((always_inline)) static inline void atomic_and(atomic_t *atomic, int mask)
{
   __asm__ __volatile__("lock andl %%eax,(%%ebx)" ::"a"(mask),"b"(&atomic->value)
                        : "memory");
}
__attribute__((always_inline)) static inline void atomic_set_mask(atomic_t *atomic, int mask)
{
   atomic_or(&atomic->value, mask);
}

__attribute__((always_inline)) static inline void atomic_clear_mask(atomic_t *atomic, int mask)
{
   atomic_and(&atomic->value, ~mask);
}
/* copyright (c) 2021 AlanCui END*/
#define atomic_xchg(v, new) (xchg(&((v)->value), new))

#endif /* _X86_ATOMIC_H */

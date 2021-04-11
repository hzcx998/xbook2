#ifndef _LIB_x86_ATOMIC_H
#define _LIB_x86_ATOMIC_H

/* 定义出口 */
void mem_atomic_add(int *a, int b);
void mem_atomic_sub(int *a, int b);
void mem_atomic_inc(int *a);
void mem_atomic_dec(int *a);
void mem_atomic_or(int *a, int b);
void mem_atomic_and(int *a, int b);

#endif  /* _LIB_x86_ATOMIC_H */

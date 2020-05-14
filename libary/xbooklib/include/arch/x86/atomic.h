#ifndef _LIB_x86_ATOMIC_H
#define _LIB_x86_ATOMIC_H

/* 定义出口 */
void __atomic_add(int *a, int b);
void __atomic_sub(int *a, int b);
void __atomic_inc(int *a);
void __atomic_dec(int *a);
void __atomic_or(int *a, int b);
void __atomic_and(int *a, int b);

#endif  /* _LIB_x86_ATOMIC_H */

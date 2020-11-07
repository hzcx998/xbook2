#ifndef _XLIBC_SETJMP_H
#define _XLIBC_SETJMP_H

#ifdef __cplusplus
extern "C" {
#endif

#define _NSETJMP    10

typedef long jmp_buf[_NSETJMP];

int setjmp(jmp_buf env);
void longjmp(jmp_buf env, int val);

#ifdef __cplusplus
}
#endif

#endif  /* _XLIBC_SETJMP_H */

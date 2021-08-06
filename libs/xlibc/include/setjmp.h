#ifndef _XLIBC_SETJMP_H
#define _XLIBC_SETJMP_H

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned long __jmp_buf[26];

typedef struct __jmp_buf_tag {
	__jmp_buf __jb;
	unsigned long __fl;
	unsigned long __ss[128/sizeof(long)];
} jmp_buf[1];

/*
#define _NSETJMP    10

typedef long jmp_buf[_NSETJMP];
*/

int setjmp(jmp_buf env);
void longjmp(jmp_buf env, int val);

#ifdef __cplusplus
}
#endif

#endif  /* _XLIBC_SETJMP_H */

#ifndef __XLIBC_EXECINFO_H__
#define __XLIBC_EXECINFO_H__

#ifdef __cplusplus
extern "C" {
#endif

int backtrace(void **buffer, int size);
int backtrace2(void** buffer, int size);

#ifdef __cplusplus
}
#endif

#endif /* __XLIBC_EXECINFO_H__ */

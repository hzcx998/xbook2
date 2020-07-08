#ifndef _LIB_STDARG_H
#define _LIB_STDARG_H

#define _AUPBND 1
#define _ADNBND 1 
#define _Bnd(X, bnd) (sizeof(X) + ((bnd) & ~(bnd)))

typedef char *va_list;
#define va_arg(ap, T) \
    (*(T *)(((ap) += _Bnd(T, _AUPBND)) - _Bnd(T, _ADNBND)))
#define va_end(ap) (void)0
#define va_start(ap, A) \
    (void)((ap) = (char *)&(A) + _Bnd(A, _AUPBND))

/*
 * copy variable args
 */
#define va_copy(d, s)		__builtin_va_copy(d, s)

#endif  /* _LIB_STDARG_H */

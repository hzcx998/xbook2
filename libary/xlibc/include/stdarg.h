#ifndef _XLIBC_STDARG_H
#define _XLIBC_STDARG_H

#define _AUPBND 1
#define _ADNBND 1 
#define _Bnd(X, bnd) (sizeof(X) + ((bnd) & ~(bnd)))

typedef char *va_list;
#define va_arg(ap, T) \
    (*(T *)(((ap) += _Bnd(T, _AUPBND)) - _Bnd(T, _ADNBND)))
#define va_end(ap) (void)0
#define va_start(ap, A) \
    (void)((ap) = (char *)&(A) + _Bnd(A, _AUPBND))

#endif  /* _XLIBC_STDARG_H */

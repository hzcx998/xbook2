#ifndef _XBOOK_STDARG_H
#define _XBOOK_STDARG_H
#if 0
#define _AUPBND 1
#define _ADNBND 1 
#define _Bnd(X, bnd) (sizeof(X) + ((bnd) & ~(bnd)))

typedef char *va_list;
#define va_arg(ap, T) \
    (*(T *)(((ap) += _Bnd(T, _AUPBND)) - _Bnd(T, _ADNBND)))
#define va_end(ap) (void)0
#define va_start(ap, A) \
    (void)((ap) = (char *)&(A) + _Bnd(A, _AUPBND))

#else
typedef char *va_list;
#define va_start(list,param1)   ( list = (va_list)&param1+ sizeof(param1) )
#define va_arg(list,mode)   ( (mode *) ( list += sizeof(mode) ) )[-1]
#define va_end(list) ( list = (va_list)0 )
#endif

#endif  /* _XBOOK_STDARG_H */

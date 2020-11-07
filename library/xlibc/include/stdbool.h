#ifndef _XLIBC_STDBOOL_H
#define _XLIBC_STDBOOL_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _STDBOOL
#define _STDBOOL

#define __bool_true_false_are_defined 1
 
#ifndef __cplusplus
//#define bool _Bool      //C语言下实现Bool
#ifndef bool
#define bool char      
#endif
#define true 1
#define false 0
#endif /* __cplusplus */

#endif /* _STDBOOL */

#ifdef __cplusplus
}
#endif

#endif  /* _XLIBC_STDBOOL_H */

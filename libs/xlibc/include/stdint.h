#ifndef _LIB_STDINT_H
#define _LIB_STDINT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned long long uint64_t;
typedef signed long long int64_t;
typedef unsigned int    uint32_t;
typedef signed int      int32_t;
typedef unsigned short  uint16_t;
typedef signed short    int16_t;
typedef unsigned char   uint8_t;
typedef signed char     int8_t;
#if 0
typedef unsigned long long  uint64;
typedef signed long long int64;
typedef unsigned int    uint32;
typedef signed int      int32;
typedef unsigned short  uint16;
typedef signed short    int16;
typedef unsigned char   uint8;
typedef signed char     int8;
#endif
typedef unsigned long long u64;
typedef signed long long s64;
typedef unsigned int    u32;
typedef signed int      s32;
typedef unsigned short  u16;
typedef signed short    s16;
typedef unsigned char   u8;
typedef signed char     s8;

typedef int intptr_t;
#if 0
typedef unsigned int	UINT;	/* int must be 16-bit or 32-bit */
typedef unsigned char	BYTE;	/* char must be 8-bit */
typedef unsigned short	WORD;	/* 16-bit unsigned integer */
typedef unsigned long	DWORD;	/* 32-bit unsigned integer */
typedef WORD			WCHAR;	/* UTF-16 character type */
#endif

typedef long long          intmax_t;
typedef unsigned long long uintmax_t;


 
 
/* Types for `void *' pointers. */
#if __WORDSIZE == 64
# ifndef __intptr_t_defined
typedef long int        intptr_t;
# define __intptr_t_defined
# endif
typedef unsigned long int   uintptr_t;
#else
# ifndef __intptr_t_defined
typedef int         intptr_t;
# define __intptr_t_defined
# endif
typedef unsigned int        uintptr_t;
#endif
 
 
/* Largest integral types. */
#if __WORDSIZE == 64
typedef long int        intmax_t;
typedef unsigned long int   uintmax_t;
#else
__extension__
typedef long long int       intmax_t;
__extension__
typedef unsigned long long int uintmax_t;
#endif
 

 
/* Limits of integral types. */
 
/* Minimum of signed integral types. */
# define INT8_MIN       (-128)
# define INT16_MIN      (-32767-1)
# define INT32_MIN      (-2147483647-1)
# define INT64_MIN      (-__INT64_C(9223372036854775807)-1)
/* Maximum of signed integral types. */
# define INT8_MAX       (127)
# define INT16_MAX      (32767)
# define INT32_MAX      (2147483647)
# define INT64_MAX      (__INT64_C(9223372036854775807))
 
/* Maximum of unsigned integral types. */
# define UINT8_MAX      (255)
# define UINT16_MAX     (65535)
# define UINT32_MAX     (4294967295U)
# define UINT64_MAX     (__UINT64_C(18446744073709551615))


#ifdef __cplusplus
}
#endif


#endif  /*_LIB_STDINT_H*/

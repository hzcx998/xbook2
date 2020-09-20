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

typedef unsigned long long  uint64;
typedef signed long long int64;
typedef unsigned int    uint32;
typedef signed int      int32;
typedef unsigned short  uint16;
typedef signed short    int16;
typedef unsigned char   uint8;
typedef signed char     int8;

typedef unsigned long long u64;
typedef signed long long s64;
typedef unsigned int    u32;
typedef signed int      s32;
typedef unsigned short  u16;
typedef signed short    s16;
typedef unsigned char   u8;
typedef signed char     s8;

typedef int ptrdiff_t;
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


#ifdef __cplusplus
}
#endif


#endif  /*_LIB_STDINT_H*/

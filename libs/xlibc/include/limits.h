#ifndef _XLIBC_LIMITS_H
#define _XLIBC_LIMITS_H

#ifdef __cplusplus
extern "C" {
#endif

#define CHAR_BIT      8         /* number of bits in a char */
#define SCHAR_MIN   (-128)      /* minimum signed char value */
#define SCHAR_MAX     127       /* maximum signed char value */
#define UCHAR_MAX     0xff      /* maximum unsigned char value */

#ifndef _CHAR_UNSIGNED
#define CHAR_MIN    SCHAR_MIN   /* mimimum char value */
#define CHAR_MAX    SCHAR_MAX   /* maximum char value */
#else  /* _CHAR_UNSIGNED */
#define CHAR_MIN      0
#define CHAR_MAX    UCHAR_MAX
#endif  /* _CHAR_UNSIGNED */

#ifndef MB_LEN_MAX
#define MB_LEN_MAX    1             /* max. # bytes in multibyte char */
#endif

#define SHRT_MIN    (-32768)        /* minimum (signed) short value */
#define SHRT_MAX      32767         /* maximum (signed) short value */
#define USHRT_MAX     0xffff        /* maximum unsigned short value */
#define INT_MIN     (-2147483647 - 1) /* minimum (signed) int value */
#define INT_MAX       2147483647    /* maximum (signed) int value */
#define UINT_MAX      0xffffffff    /* maximum unsigned int value */
#define LONG_MIN    (-2147483647L - 1) /* minimum (signed) long value */
#define LONG_MAX      2147483647L   /* maximum (signed) long value */
#define ULONG_MAX     0xffffffffUL  /* maximum unsigned long value */
#define LLONG_MAX     9223372036854775807       /* maximum signed long long int value */
#define LLONG_MIN   (-9223372036854775807 - 1)  /* minimum signed long long int value */
#define ULLONG_MAX    0xffffffffffffffff       /* maximum unsigned long long int value */


#define INTMAX_MIN       INT_MIN
#define INTMAX_MAX       INT_MAX
#define UINTMAX_MAX      UINT_MAX

#define _I8_MIN     (-127i8 - 1)    /* minimum signed 8 bit value */
#define _I8_MAX       127i8         /* maximum signed 8 bit value */
#define _UI8_MAX      0xffui8       /* maximum unsigned 8 bit value */

#define _I16_MIN    (-32767i16 - 1) /* minimum signed 16 bit value */
#define _I16_MAX      32767i16      /* maximum signed 16 bit value */
#define _UI16_MAX     0xffffui16    /* maximum unsigned 16 bit value */

#define _I32_MIN    (-2147483647i32 - 1) /* minimum signed 32 bit value */
#define _I32_MAX      2147483647i32 /* maximum signed 32 bit value */
#define _UI32_MAX     0xffffffffui32 /* maximum unsigned 32 bit value */

/* minimum signed 64 bit value */
#define _I64_MIN    (-9223372036854775807i64 - 1)
/* maximum signed 64 bit value */
#define _I64_MAX      9223372036854775807i64
/* maximum unsigned 64 bit value */
#define _UI64_MAX     0xffffffffffffffffui64

#ifndef SIZE_MAX
#ifdef _WIN64
#define SIZE_MAX _UI64_MAX
#else  /* _WIN64 */
#define SIZE_MAX UINT_MAX
#endif  /* _WIN64 */
#endif  /* SIZE_MAX */

#ifndef SSIZE_MAX
#define SSIZE_MAX INT_MAX
#endif  /* SIZE_MAX */

#ifndef NAME_MAX
#define NAME_MAX 255
#endif

#define PATH_MAX 4096
/* Arbitrary numbers... */

#define BC_BASE_MAX 99
#define BC_DIM_MAX 2048
#define BC_SCALE_MAX 99
#define BC_STRING_MAX 1000
#define CHARCLASS_NAME_MAX 14
#define COLL_WEIGHTS_MAX 2
#define EXPR_NEST_MAX 32
#define LINE_MAX 4096
#define RE_DUP_MAX 255

#define PAGE_SIZE 4096

#ifdef __cplusplus
}
#endif

#endif  /* _XLIBC_LIMITS_H */

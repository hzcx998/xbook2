#ifndef _SYS_TYPES_H
#define _SYS_TYPES_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef long pid_t;
typedef unsigned long flags_t;
typedef unsigned long register_t;    //寄存器
typedef unsigned long off_t;         //偏移类型的变量
typedef unsigned long sector_t;
typedef unsigned int mode_t;
typedef unsigned long dev_t;
typedef unsigned long blksize_t;
typedef unsigned long blkcnt_t;
typedef long clock_t;
typedef long time_t;
typedef int clockid_t;
typedef unsigned long unid_t;
typedef unsigned long ino_t;
typedef unsigned long nlink_t;
typedef unsigned int uid_t;
typedef unsigned int gid_t;
typedef unsigned long useconds_t;
typedef int dir_t;

typedef signed char				s8_t;
typedef unsigned char			u8_t;
typedef signed short			s16_t;
typedef unsigned short			u16_t;
typedef signed int				s32_t;
typedef unsigned int			u32_t;
typedef signed long long		s64_t;
typedef unsigned long long		u64_t;

typedef long long   loff_t;

typedef char   bool_t;

typedef void *caddr_t;

/* Types for `void *' pointers.  */
#if __WORDSIZE == 64
# ifndef __intptr_t_defined
typedef long int		intptr_t;
#  define __intptr_t_defined
# endif
typedef unsigned long int	uintptr_t;
#else
# ifndef __intptr_t_defined
typedef int			intptr_t;
#  define __intptr_t_defined
# endif
typedef unsigned int		uintptr_t;
#endif

#ifdef __cplusplus
}
#endif

#endif  /* _SYS_TYPES_H */

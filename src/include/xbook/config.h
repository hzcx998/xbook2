#ifndef _XBOOK_CONFIG_H
#define _XBOOK_CONFIG_H

/* config address width */
/*
#define CONFIG_32BIT
#define CONFIG_64BIT
*/

/* kernel low memory: 0-> high memory, 1-> low memory */
#define CONFIG_KERN_LOWMEM  0

/* config large alloc size in memcache */
// #define CONFIG_LARGE_ALLOCS
#define CONFIG_SMALL_ALLOCS

/* auto select timezone */
/* #define CONFIG_TIMEZONE_AUTO */

/* 裁剪配置 */
// #define CONFIG_NO_SYS_TIMES

// #define CONFIG_NETWORK

/* 配置新的系统调用接口，将会覆盖原有的系统调用 */
#if defined(__TINYLIBC__) || defined(__MUSLLIBC__) 
#define CONFIG_NEWSYSCALL
#endif
/*
#ifndef CONFIG_NEWSYSCALL
#define CONFIG_NEWSYSCALL
#endif
*/

/* 配置内核账户管理机制 */
// #define CONFIG_ACCOUNT

/* 配置对pthread接口的支持 */
// #define CONFIG_PTHREAD

/* 配置信号机制 */
#define CONFIG_SIGNAL

#endif   /* _XBOOK_CONFIG_H */

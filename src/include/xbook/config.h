#ifndef _XBOOK_CONFIG_H
#define _XBOOK_CONFIG_H

/* 配置处理器位宽 */
#define CONFIG_32BIT
/* #define CONFIG_64BIT */

/* kernel low memory: 0-> high memory, 1-> low memory */
#define CONFIG_KERN_LOWMEM  0

/* config large alloc size in memcache */
#define CONFIG_LARGE_ALLOCS

/* auto select timezone */
/* #define CONFIG_TIMEZONE_AUTO */

#endif   /* _XBOOK_CONFIG_H */

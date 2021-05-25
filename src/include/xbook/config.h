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

#endif   /* _XBOOK_CONFIG_H */

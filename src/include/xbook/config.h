#ifndef _XBOOK_CONFIG_H
#define _XBOOK_CONFIG_H

/* config architecture */
#define CONFIG_ARCH_X86
/* #define CONFIG_ARCH_X64 */
/* #define CONFIG_ARCH_ARM32 */
/* #define CONFIG_ARCH_ARM64 */

/* config address width */
#define CONFIG_32BIT
/* #define CONFIG_64BIT */

/* config large alloc size in memcache */
#define CONFIG_LARGE_ALLOCS

/* auto select timezone */
#define CONFIG_TIMEZONE_AUTO 0 	

/* ahci disk driver */
#define CONFIG_AHCI 

/* shade layer */
#define CONFIG_SHADE_LAYER

#endif   /* _XBOOK_CONFIG_H */

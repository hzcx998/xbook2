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
#define CONFIG_TIMEZONE_AUTO

/* ahci disk driver */
#define CONFIG_AHCI

/* shade layer */
#define CONFIG_SHADE_LAYER

// #define LAYER_ALPAH /* 是否拥有透明图层 */

/* gui console print */
#define CONFIG_GUI_PRINT

/* use keyboard to make virtual mouse */
#define CONFIG_VIRTUAL_MOUSE

#endif   /* _XBOOK_CONFIG_H */

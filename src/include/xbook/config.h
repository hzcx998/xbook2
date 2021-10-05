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

/* net config */
#ifdef CONFIG_NET
    
    /* ip */
    #define NETIF_IP0 192
    #define NETIF_IP1 168
    #define NETIF_IP2 0
    #define NETIF_IP3 105
    
    /* gateway */
    #define NETIF_GW0 192
    #define NETIF_GW1 168
    #define NETIF_GW2 0
    #define NETIF_GW3 1

    /* netmask */
    #define NETIF_MASK0 255
    #define NETIF_MASK1 255
    #define NETIF_MASK2 255
    #define NETIF_MASK3 0
    
#endif

#endif   /* _XBOOK_CONFIG_H */

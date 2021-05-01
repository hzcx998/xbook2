#ifndef _XBOOK_STDBOOL_H
#define _XBOOK_STDBOOL_H

#ifndef __cplusplus
#define bool char      
#define true 1
#define false 0
#endif

#ifndef BOOLEAN
#ifndef __cplusplus
    #define BOOLEAN char     
#else
    #define BOOLEAN _Bool       //C语言下实现Bool
#endif
    #ifndef TRUE
    #define TRUE    1 
    #endif

    #ifndef FALSE
    #define FALSE    0 
    #endif
#endif

#endif  /*_XBOOK_STDBOOL_H*/
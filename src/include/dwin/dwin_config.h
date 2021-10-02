#ifndef _DWIN_CONFIG_H
#define _DWIN_CONFIG_H

#define DWIN_TAG "[dwin] "

#define DWIN_THREAD_NAME "dwin"

/* config dwin hal type */
#define CONFIG_DWIN_HAL_KDEVICE

#if defined(CONFIG_DWIN_HAL_KDEVICE)
#include <xbook/debug.h>    // dbgprint
#include <stddef.h> // NULL
#define dwin_log(fmt, ...) dbgprint(fmt, ##__VA_ARGS__)
#endif

#endif   /* _DWIN_CONFIG_H */

#ifndef _DWIN_CONFIG_H
#define _DWIN_CONFIG_H

#define DWIN_TAG "[dwin] "

#define DWIN_THREAD_NAME "dwin"

/* config dwin hal type */
#define CONFIG_DWIN_HAL_KDEVICE

#if defined(CONFIG_DWIN_HAL_KDEVICE)

/* types & define */
#include <stddef.h> // NULL
#include <stdint.h> // uint

#include <string.h> // string & memory

/* debug */
#include <xbook/debug.h>    // dbgprint
#include <assert.h> // assert
#define dwin_log(fmt, ...) dbgprint(fmt, ##__VA_ARGS__)
#define dwin_assert(cond)    assert(cond)

/* memory */
#include <xbook/memalloc.h> // memory
#define dwin_malloc(sz) mem_alloc(sz)
#define dwin_free(sz) mem_free(sz)

/* lock for global variable */
#include <arch/interrupt.h> // interrupt enable&disable
typedef unsigned long dwin_critical_t;
#define dwin_enter_critical(val) interrupt_save_and_disable(val)
#define dwin_leave_critical(val) interrupt_restore_state(val)

/* TODO: add list config */

/* input key */
#include <sys/input.h>

/* errno */
#include <errno.h>

#endif

#define dwin_max(a,b)    (((a) > (b)) ? (a) : (b))
#define dwin_min(a,b)    (((a) < (b)) ? (a) : (b))

#endif   /* _DWIN_CONFIG_H */

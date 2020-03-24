#ifndef _XBOOK_KERNEL_H
#define _XBOOK_KERNEL_H

#include <arch/page.h>

#define KERN_EMERG      "<0>"      /* system is unuseable */
#define KERN_ALTER      "<1>"      /* action must be taken immediatgely */
#define KERN_CRIT       "<2>"      /* critical condition */
#define KERN_ERR        "<3>"      /* error condition */
#define KERN_WARING     "<4>"      /* waring condition */
#define KERN_NOTICE     "<5>"      /* normal significant */
#define KERN_INFO       "<6>"      /* infomational */
#define KERN_DEBUG      "<7>"      /* debug message */

#define KERN_VADDR      PAGE_OFFSET


#endif   /* _XBOOK_KERNEL_H */

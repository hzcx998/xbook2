#ifndef _LIB_STDLIB_H
#define _LIB_STDLIB_H

#include "stddef.h"
#include "types.h"
#include <xbook/kmalloc.h>

void abort(void);

#define malloc  kmalloc
#define free    kfree


#endif  /* _LIB_STDLIB_H */

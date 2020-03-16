#ifndef _X86_INTERRUPT_H
#define _X86_INTERRUPT_H

#include "registers.h"

void __disable_intr(void);
void __enable_intr(void);

/* save intr status and disable intr */
#define __save_intr(flags)                  \
    do {                                    \
        flags = (unsigned int)load_eflags();\
        __disable_intr();                   \
    } while (0)

/* restore intr status and enable intr */
#define __restore_intr(flags)               \
    do {                                    \
        store_eflags((unsigned int)flags);\
    } while (0)

#endif  /* _X86_INTERRUPT_H */

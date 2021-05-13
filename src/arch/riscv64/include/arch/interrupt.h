#ifndef _RISCV64_INTERRUPT_H
#define _RISCV64_INTERRUPT_H

void interrupt_disable(void);
void interrupt_enable(void);

// TODO: update interrupt_save_and_disable & interrupt_restore_state
#define interrupt_save_and_disable(flags)                  \
    do {                                    \
    } while (0)

#define interrupt_restore_state(flags)               \
    do {                                    \
    } while (0)

void
trapinithart(void);

#endif  /* _RISCV64_INTERRUPT_H */

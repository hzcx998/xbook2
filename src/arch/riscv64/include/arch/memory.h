#ifndef _RISCV64_MEMORY_H
#define _RISCV64_MEMORY_H

#include <stddef.h>
#include <stdint.h>

#define read8(addr) (*(volatile uint8_t *)(addr))
#define read16(addr) (*(volatile uint16_t *)(addr))
#define read32(addr) (*(volatile uint32_t *)(addr))
#define read64(addr) (*(volatile uint64_t *)(addr))

#define write8(addr, value)                      \
    {                                        \
        (*(volatile uint8_t *)(addr)) = (value); \
    }
#define write16(addr, value)                       \
    {                                         \
        (*(volatile uint16_t *)(addr)) = (value); \
    }
#define write32(addr, value)                       \
    {                                         \
        (*(volatile uint32_t *)(addr)) = (value); \
    }
#define write64(addr, value)                       \
    {                                         \
        (*(volatile uint64_t *)(addr)) = (value); \
    }

#endif   /* _RISCV64_MEMORY_H */

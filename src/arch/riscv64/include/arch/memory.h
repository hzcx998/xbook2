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


#define readb(addr) (*(volatile uint8 *)(addr))
#define readw(addr) (*(volatile uint16 *)(addr))
#define readd(addr) (*(volatile uint32 *)(addr))
#define readq(addr) (*(volatile uint64 *)(addr))

#define writeb(v, addr)                      \
    {                                        \
        (*(volatile uint8 *)(addr)) = (v); \
    }
#define writew(v, addr)                       \
    {                                         \
        (*(volatile uint16 *)(addr)) = (v); \
    }
#define writed(v, addr)                       \
    {                                         \
        (*(volatile uint32 *)(addr)) = (v); \
    }
#define writeq(v, addr)                       \
    {                                         \
        (*(volatile uint64 *)(addr)) = (v); \
    }

#define mb()        __sync_synchronize()
#define barrier()   __sync_synchronize()
#define wmb()       __sync_synchronize()
#define rmb()       __sync_synchronize()

#endif   /* _RISCV64_MEMORY_H */

#ifndef __RISCV64_PHYMEM_H
#define __RISCV64_PHYMEM_H

#include <k210_qemu_phymem.h>

#define KERN_SPACE_TOP_ADDR                MAX_VIR_ADDR 
#define USER_SPACE_START_ADDR              0x00000000
#define USER_SPACE_SIZE                    0x80000000

#define DYNAMIC_MAP_MEM_SIZE               (128 * MB)
#define DYNAMIC_MAP_MEM_END                KERN_SPACE_TOP_ADDR
#define DYNAMIC_MAP_MEM_ADDR               (KERN_SPACE_TOP_ADDR - DYNAMIC_MAP_MEM_SIZE + 1)

#endif  /* __RISCV64_PHYMEM_H */

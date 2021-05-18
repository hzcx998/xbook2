#ifndef _RISCV64_VMM_H
#define _RISCV64_VMM_H

// #include <xbook/task.h>

// int vmm_copy_mapping(task_t *child, task_t *parent);

void vmm_active_kernel();
void vmm_active_user(unsigned long page);

#endif	/* _RISCV64_VMM_H */
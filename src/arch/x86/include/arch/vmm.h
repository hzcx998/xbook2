#ifndef _X86_VMM_H
#define _X86_VMM_H

#include <xbook/task.h>

int vmm_copy_mapping(task_t *child, task_t *parent);

void vmm_active_kernel();
void vmm_active_user(unsigned int page);


#endif	/*_X86_VMM_H */
#ifndef _X86_VMM_H
#define _X86_VMM_H

#include <xbook/task.h>

int __copy_vm_mapping(task_t *child, task_t *parent);

#define copy_vm_mapping __copy_vm_mapping

#endif	/*_X86_VMM_H */
#include <xbook/vmm.h>
#include <xbook/debug.h>

void vmm_init(vmm_t *vmm)
{
    vmm->page_storage = copy_kernel_page_storge();
    if (vmm->page_storage == NULL) {
        panic(KERN_EMERG "task_init_vmm: kmalloc for page_storege failed!\n");
    }
    vmm->vmspace_head = NULL;
    /* 其它参数 */
    
}


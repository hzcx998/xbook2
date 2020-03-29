#include <xbook/vmm.h>
#include <xbook/debug.h>
#include <xbook/vmspace.h>

void vmm_init(vmm_t *vmm)
{
    vmm->page_storage = copy_kernel_page_storge();
    if (vmm->page_storage == NULL) {
        panic(KERN_EMERG "task_init_vmm: kmalloc for page_storege failed!\n");
    }
    vmm->vmspace_head = NULL;
    /* 其它参数 */
    
}

int vmm_exit(vmm_t *vmm)
{
    if (vmm == NULL)
        return -1; 
    vmspace_t *space = (vmspace_t *)vmm->vmspace_head;
    vmspace_t *p;
    while (space != NULL) {
        /* 由于内存区域可能不是连续的，所以需要用安全的方式来取消映射 */
        unmap_pages_safe(space->start, space->end - space->start);
        space = space->next;
    }
    space = (vmspace_t *)vmm->vmspace_head;
    while (space != NULL) {
        p = space;
        space = space->next;
        vmspace_free(p); /* 释放空间 */
    }
    
    vmm->vmspace_head = NULL;
    free_page(v2p(vmm->page_storage));
    return 0;
}

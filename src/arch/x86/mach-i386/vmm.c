#include <arch/page.h>
#include <arch/vmm.h>
#include <xbook/kmalloc.h>
#include <xbook/debug.h>
#include <xbook/vmspace.h>
#include <xbook/vmm.h>
#include <string.h>

/* 复制进程虚拟内存的映射 */
int __copy_vm_mapping(task_t *child, task_t *parent)
{
    /* 开始内存的复制 */
    void *buf = kmalloc(PAGE_SIZE);
    if (buf == NULL) {
        printk(KERN_ERR "copy_vm_mapping: kmalloc buf for data transform failed!\n");
        return -1;
    }
    /* 获取父目录的虚拟空间 */
    vmspace_t *space = parent->vmm->vmspace_head;
    
    unsigned long prog_vaddr = 0;
    unsigned long paddr;
    
    /* 当空间不为空时就一直获取 */
    while (space != NULL) {
        /* 获取空间最开始地址 */
        prog_vaddr = space->start;
        // printk(KERN_DEBUG "the space %x start %x end %x\n", space, space->start, space->end);
        /* 在空间中进行复制 */
        while (prog_vaddr < space->end) {
            /* 1.将进程空间中的数据复制到内核空间，切换页表后，
            还能访问到父进程中的数据 */
            memcpy(buf, (void *)prog_vaddr, PAGE_SIZE);

            /* 2.切换进程空间 */
            vmm_active(child->vmm);

            /* 3.映射虚拟地址 */
            // 分配一个物理页
            paddr = alloc_page();
            if (!paddr) {
                printk(KERN_ERR "copy_vm_mapping: alloc_page for vaddr failed!\n");
        
                /* 激活父进程并返回 */
                vmm_active(parent->vmm);

                return -1;
            }
            // 根据空间的保护来设定页属性
            page_link(prog_vaddr, paddr, PG_RW_W | PG_US_U);

            /* 4.从内核复制数据到进程 */
            memcpy((void *)prog_vaddr, buf, PAGE_SIZE);

            /* 5.恢复父进程内存空间 */
            vmm_active(parent->vmm);
            // printk(KERN_DEBUG "copy at virtual address %x\n", prog_vaddr);
            /* 指向下一个页 */
            prog_vaddr += PAGE_SIZE;
        }
        /* 指向下一个空间 */
        space = space->next;
    }
    kfree(buf);
    return 0; 
}

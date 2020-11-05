#include <arch/page.h>
#include <arch/vmm.h>
#include <xbook/memalloc.h>
#include <xbook/debug.h>
#include <xbook/memspace.h>
#include <xbook/vmm.h>
#include <xbook/schedule.h>
#include <arch/tss.h>
#include <string.h>

#define DEBUG_VMM

static int do_copy_share_page(addr_t vaddr, vmm_t *child, vmm_t *parent)
{
    addr_t paddr = addr_vir2phy(vaddr);
    pr_dbg("[vmm]: copy share page at vaddr %x phy addr %x\n", vaddr, paddr);   
    vmm_active(child);
    page_link_addr(vaddr, paddr, PAGE_ATTR_WRITE | PAGE_ATTR_USER);
    vmm_active(parent);
    return 0;
}

static int do_copy_normal_page(addr_t vaddr, void *buf, vmm_t *child, vmm_t *parent)
{
    addr_t paddr;
    memcpy(buf, (void *)vaddr, PAGE_SIZE);
    vmm_active(child);
    paddr = page_alloc_one();
    if (!paddr) {
        printk(KERN_ERR "vmm_copy_mapping: page_alloc_one for vaddr failed!\n");
        vmm_active(parent);
        return -1;
    }
    page_link_addr(vaddr, paddr, PAGE_ATTR_WRITE | PAGE_ATTR_USER);
    memcpy((void *)vaddr, buf, PAGE_SIZE);
    vmm_active(parent);
    return 0;
}

int vmm_copy_mapping(task_t *child, task_t *parent)
{
    void *buf = mem_alloc(PAGE_SIZE);
    if (buf == NULL) {
        printk(KERN_ERR "vmm_copy_mapping: mem_alloc buf for data transform failed!\n");
        return -1;
    }
    mem_space_t *space = parent->vmm->mem_space_head;
    addr_t prog_vaddr = 0;
    while (space != NULL) {
        prog_vaddr = space->start;
        while (prog_vaddr < space->end) {
            /* 如果是共享内存，就只复制页映射，而不创建新的页 */
            if (space->flags & MEM_SPACE_MAP_SHARED) {
                if (do_copy_share_page(prog_vaddr, child->vmm, parent->vmm) < 0) {
                    mem_free(buf);
                    return -1;
                }
            } else {
                if (do_copy_normal_page(prog_vaddr, buf, child->vmm, parent->vmm) < 0) {
                    mem_free(buf);
                    return -1;
                }
            }
            prog_vaddr += PAGE_SIZE;
        }
        space = space->next;
    }
    mem_free(buf);
    return 0; 
}

void vmm_active_kernel()
{
    unsigned long paddr = KERN_PAGE_DIR_PHY_ADDR;
    cpu_cr3_write(paddr);
    tss_update_info((unsigned long )task_current);
}

void vmm_active_user(unsigned int page)
{
    cpu_cr3_write(page);    
    tss_update_info((unsigned long )task_current);
}
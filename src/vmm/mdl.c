#include <xbook/mdl.h>
#include <xbook/memcache.h>
#include <xbook/debug.h>
#include <xbook/virmem.h>
#include <xbook/memspace.h>
#include <xbook/schedule.h>


/**
 * mdl_alloc - 分配一个内存描述列表
 * @vaddr: 用户虚拟地址
 * @length: 数据长度
 * @later: 描述列表不是第一个
 * @ioreq: 输入输出请求
 * 
 * 分配并映射内存，在内核中分配一个内核虚拟地址，映射到和用户的虚拟地址的物理页一样的物理页，
 * 相当于共享了用户虚拟地址的物理页，这样，就可以通过内核的虚拟地址来访问用户的虚拟地址。
 * 
 * @return: 成功返回mdl指针，失败返回NULL
 */
mdl_t *mdl_alloc(void *vaddr, unsigned long length,
    bool later, io_request_t *ioreq)
{
    if (!length || vaddr == NULL)
        return NULL;

    /* 分配mdl空间 */
    mdl_t *mdl = mem_alloc(sizeof(mdl_t));
    if (mdl == NULL) {
        return NULL;
    }
    /* 获取虚拟地址页对齐的地址 */
    mdl->start_vaddr =  (void *) (((unsigned long) vaddr) & PAGE_MASK);
    mdl->byte_offset = (char *)vaddr - (char *)mdl->start_vaddr;
    if (length > MDL_MAX_SIZE) {
        length = MDL_MAX_SIZE;      /* 剪切长度 */
        keprint(PRINT_NOTICE "mdl_alloc: length=%x too long!\n", length);    
    }
    mdl->byte_count = length;
    mdl->task = task_current;
    mdl->next = NULL;

    unsigned long flags;    /* 关闭并保存中断 */
    interrupt_save_and_disable(flags);

    unsigned long phyaddr = addr_vir2phy((unsigned long) mdl->start_vaddr);
    keprint(PRINT_DEBUG "mdl_alloc: viraddr=%x phyaddr=%x\n", vaddr, phyaddr);
    unsigned long pages = PAGE_ALIGN(length) / PAGE_SIZE;
    keprint(PRINT_DEBUG "mdl_alloc: length=%d pages=%d\n", length, pages);
    
    /* 分配一个虚拟地址 */
    unsigned long mapped_vaddr = vir_addr_alloc(length);
    if (!mapped_vaddr) {
        mem_free(mdl);
        interrupt_restore_state(flags);
        return NULL;
    }
    mdl->mapped_vaddr = (void *) mapped_vaddr;
    
    /* 映射固定内存页（虚拟地址和物理地址都指定了） */
    page_map_addr_fixed(mapped_vaddr, phyaddr, length, PROT_KERN | PROT_WRITE);
    
    interrupt_restore_state(flags);
    keprint(PRINT_DEBUG "mdl_alloc: map success!\n");
    
    /* 有请求才进行关联 */
    if (ioreq) {    
        if (later) { /* 插入到末尾 */
            keprint(PRINT_DEBUG "mdl_alloc: insert tail.\n");
    
            mdl_t *cur = ioreq->mdl_address;
            while (cur != NULL) {
                if (cur->next == NULL) {
                    cur->next = mdl;
                    break;
                }
                cur = cur->next;
            }
        } else { /* 队首 */
            keprint(PRINT_DEBUG "mdl_alloc: insert head.\n");
    
            mdl->next = ioreq->mdl_address;
            ioreq->mdl_address = mdl;
        }
    }
    return mdl;
}

/**
 * mdl_free - 释放内存描述列表
 * @mdl: 内存描述列表
 * 
 */
void mdl_free(mdl_t *mdl)
{
    if (mdl == NULL)
        return;
    keprint(PRINT_DEBUG "mdl_free: vaddr=%x length=%d byte offset=%d mapped vaddr=%x.\n",
        mdl->start_vaddr, mdl->byte_count, mdl->byte_offset, mdl->mapped_vaddr);
    unsigned long flags;    /* 关闭并保存中断 */
    interrupt_save_and_disable(flags);
    /* 取消共享内存映射 */
    page_unmap_addr_safe((unsigned long) mdl->mapped_vaddr, mdl->byte_count, 1);
    vir_addr_free((unsigned long) mdl->mapped_vaddr, mdl->byte_count);  /* 释放映射后的虚拟地址 */
    interrupt_restore_state(flags);

    mem_free(mdl); /* 释放mdl结构 */
}


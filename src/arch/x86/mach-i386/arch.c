#include <arch/segment.h>
#include <arch/gate.h>
#include <arch/tss.h>
#include <arch/pmem.h>
#include <arch/debug.h>
#include <arch/pic.h>
#include <arch/pci.h>
#include <xbook/debug.h>

int init_arch()
{	
    /* the first thing is to init debug! */
	init_kernel_debug();
    printk("[arch]: init debug ok.\n");

    /* init segment */
    init_segment_descriptor();
    printk("[arch]: init segment ok.\n");
    /* init interrupt */
    init_gate_descriptor();
    printk("[arch]: init gate ok.\n");
    /* init tss, must after segment, tss will use new segment. */
    init_tss();
    printk("[arch]: init tss ok.\n");
    /* init physic memory management */
    init_pmem();
    printk("[arch]: init pmem ok.\n");
    
    /* 初始化可编程时钟 */
    init_pic();
    printk("[arch]: init pic ok.\n");
    
    /* 初始化总线系统 */
    init_pci();
    printk("[arch]: init tss pci.\n");
    
	return 0;
}

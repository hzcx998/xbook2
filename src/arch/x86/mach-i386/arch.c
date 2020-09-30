#include <arch/segment.h>
#include <arch/gate.h>
#include <arch/tss.h>
#include <arch/pmem.h>
#include <arch/debug.h>
#include <arch/pic.h>
#include <arch/pci.h>

int init_arch()
{	
    /* the first thing is to init debug! */
	init_kernel_debug();
    /* init segment */
    init_segment_descriptor();
    /* init interrupt */
    init_gate_descriptor();
    /* init tss, must after segment, tss will use new segment. */
    init_tss();

    /* init physic memory management */
    init_pmem();

    /* 初始化可编程时钟 */
    init_pic();

    /* 初始化总线系统 */
    init_pci();

	return 0;
}

#include "arch.h"
#include "segment.h"
#include "gate.h"
#include "tss.h"
#include "pmem.h"
#include <xbook/debug.h>


int init_arch()
{	
    /* the first thing is to init debug! */
	init_kernel_debug();
    printk("hello, xbook!\n");

    /* init segment */
    init_segment_descriptor();
    /* init interrupt */
    init_gate_descriptor();
    /* init tss, must after segment, tss will use new segment. */
    init_tss();

    /* init physic memory management */
    init_pmem();

	return 0;
}

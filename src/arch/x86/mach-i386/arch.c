#include <arch/segment.h>
#include <arch/gate.h>
#include <arch/tss.h>
#include <arch/pmem.h>
#include <arch/debug.h>

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

	return 0;
}

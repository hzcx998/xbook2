#include <arch/segment.h>
#include <arch/gate.h>
#include <arch/tss.h>
#include <arch/phymem.h>
#include <arch/debug.h>
#include <arch/pic.h>
#include <arch/pci.h>
#include <xbook/debug.h>

int arch_init()
{	
    /* the first thing is to init debug! */
	arch_debug_init();
    segment_descriptor_init();
    gate_descriptor_init();
    tss_init();
    physic_memory_init();
    pic_init();
    pci_init();
	return 0;
}

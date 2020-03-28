#include "registers.h"
#include "segment.h"
#include "tss.h"

/* 
 * Global descriptor table
 */
struct segment_descriptor *gdt;

static void set_segment_descriptor(struct segment_descriptor *descriptor, unsigned int limit, \
		unsigned int base, unsigned int attributes)
{
	descriptor->limit_low    = limit & 0xffff;
	descriptor->base_low     = base & 0xffff;
	descriptor->base_mid     = (base >> 16) & 0xff;
	descriptor->access_right = attributes & 0xff;
	descriptor->limit_high   = ((limit >> 16) & 0x0f) | ((attributes >> 8) & 0xf0);
	descriptor->base_high    = (base >> 24) & 0xff;
}

void init_segment_descriptor()
{
    /* the new gdt */
	gdt = (struct segment_descriptor *) GDT_VADDR;
    
	int i;
	for (i = 0; i <= GDT_LIMIT/8; i++) {
		set_segment_descriptor(gdt + i, 0, 0, 0);
	}
	// 内核代码段和数据段
	set_segment_descriptor(gdt + INDEX_KERNEL_C, 0xffffffff,   0x00000000, DA_CR | DA_DPL0 | DA_32 | DA_G);
	set_segment_descriptor(gdt + INDEX_KERNEL_RW, 0xffffffff,   0x00000000, DA_DRW | DA_DPL0 | DA_32 | DA_G);
	
    tss_t *tss = get_tss();
    // tss 段
	set_segment_descriptor(gdt + INDEX_TSS, sizeof(tss_t) - 1, (uint32_t )tss, DA_386TSS);
	// 用户代码段和数据段
	set_segment_descriptor(gdt + INDEX_USER_C, 0xffffffff, 0x00000000, DA_CR | DA_DPL3 | DA_32 | DA_G);
	set_segment_descriptor(gdt + INDEX_USER_RW, 0xffffffff, 0x00000000, DA_DRW | DA_DPL3 | DA_32 | DA_G);

	// 服务代码段和数据段
	set_segment_descriptor(gdt + INDEX_SERVE_C, 0xffffffff, 0x00000000, DA_CR | DA_DPL1 | DA_32 | DA_G);
	set_segment_descriptor(gdt + INDEX_SERVE_RW, 0xffffffff, 0x00000000, DA_DRW | DA_DPL1 | DA_32 | DA_G);
	
    /* load new gdtr */
	load_gdtr(GDT_LIMIT, GDT_VADDR);
}

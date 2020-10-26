#include <arch/registers.h>
#include <arch/segment.h>
#include <arch/tss.h>

/* 
 * Global descriptor table
 */
struct segment_descriptor *gdt0;

static void segment_descriptor_set(struct segment_descriptor *descriptor, unsigned int limit, \
		unsigned int base, unsigned int attributes)
{
	descriptor->limit_low    = limit & 0xffff;
	descriptor->base_low     = base & 0xffff;
	descriptor->base_mid     = (base >> 16) & 0xff;
	descriptor->access_right = attributes & 0xff;
	descriptor->limit_high   = ((limit >> 16) & 0x0f) | ((attributes >> 8) & 0xf0);
	descriptor->base_high    = (base >> 24) & 0xff;
}

void segment_descriptor_init()
{
	gdt0 = (struct segment_descriptor *) GDT_VADDR;
	int i;
	for (i = 0; i <= GDT_LIMIT/8; i++) {
		segment_descriptor_set(GDT_OFF2PTR(gdt0, i), 0, 0, 0);
	}
	// 内核代码段和数据段
	segment_descriptor_set(GDT_OFF2PTR(gdt0, INDEX_KERNEL_C), GDT_BOUND_TOP, GDT_BOUND_BOTTOM, GDT_KERNEL_CODE_ATTR);
	segment_descriptor_set(GDT_OFF2PTR(gdt0, INDEX_KERNEL_RW), GDT_BOUND_TOP,   GDT_BOUND_BOTTOM, GDT_KERNEL_DATA_ATTR);
	
    tss_t *tss = tss_get_from_cpu0();
    // tss 段
	segment_descriptor_set(GDT_OFF2PTR(gdt0, INDEX_TSS), sizeof(tss_t) - 1, (uint32_t )tss, GDT_TSS_ATTR);
	// 用户代码段和数据段
	segment_descriptor_set(GDT_OFF2PTR(gdt0, INDEX_USER_C), GDT_BOUND_TOP, GDT_BOUND_BOTTOM, DA_CR | DA_DPL3 | DA_32 | DA_G);
	segment_descriptor_set(GDT_OFF2PTR(gdt0, INDEX_USER_RW), GDT_BOUND_TOP, GDT_BOUND_BOTTOM, DA_DRW | DA_DPL3 | DA_32 | DA_G);

    /* load new gdtr */
	gdt_register_set(GDT_LIMIT, GDT_VADDR);
}

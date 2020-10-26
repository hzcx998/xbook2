#ifndef _X86_SEGMENT_H
#define _X86_SEGMENT_H

/*
 * 段的相关信息会出现在这个文件中
 */

/* 描述符类型值说明 */
#define	DA_32			0x4000	/* 32 位段				*/
#define	DA_G			0x8000	/* 段界限粒度为 4K 字节			*/
#define	DA_DPL0			0x00	/* DPL = 0				*/
#define	DA_DPL1			0x20	/* DPL = 1				*/
#define	DA_DPL2			0x40	/* DPL = 2				*/
#define	DA_DPL3			0x60	/* DPL = 3				*/
/* 存储段描述符类型值说明 */
#define	DA_DR			0x90	/* 存在的只读数据段类型值		*/
#define	DA_DRW			0x92	/* 存在的可读写数据段属性值		*/
#define	DA_DRWA			0x93	/* 存在的已访问可读写数据段类型值	*/
#define	DA_C			0x98	/* 存在的只执行代码段属性值		*/
#define	DA_CR			0x9A	/* 存在的可执行可读代码段属性值		*/
#define	DA_CCO			0x9C	/* 存在的只执行一致代码段属性值		*/
#define	DA_CCOR			0x9E	/* 存在的可执行可读一致代码段属性值	*/
/* 系统段描述符类型值说明 */
#define	DA_LDT			0x82	/* 局部描述符表段类型值			*/
#define	DA_386TSS		0x89	/* 可用 386 任务状态段类型值		*/

/* 选择子类型值说明 */
/* 其中, SA_ : Selector Attribute */
#define	SA_RPL0		0
#define	SA_RPL1		1
#define	SA_RPL2		2
#define	SA_RPL3		3

#define	SA_TIG		0
#define	SA_TIL		4

//index of descriptor
#define	INDEX_DUMMY 0
#define	INDEX_KERNEL_C 1
#define	INDEX_KERNEL_RW 2
#define	INDEX_TSS 3
#define	INDEX_USER_C 4
#define	INDEX_USER_RW 5

//选择子...
//内核代码，数据，栈，视频

#define KERNEL_CODE_SEL ((INDEX_KERNEL_C << 3) + (SA_TIG << 2) + SA_RPL0)
#define KERNEL_DATA_SEL ((INDEX_KERNEL_RW << 3) + (SA_TIG << 2) + SA_RPL0)
#define KERNEL_STACK_SEL KERNEL_DATA_SEL 

//用户代码，数据，栈
#define USER_CODE_SEL ((INDEX_USER_C << 3) + (SA_TIG << 2) + SA_RPL3)
#define USER_DATA_SEL ((INDEX_USER_RW << 3) + (SA_TIG << 2) + SA_RPL3)
#define USER_STACK_SEL USER_DATA_SEL 

//TSS
#define KERNEL_TSS_SEL ((INDEX_TSS << 3) + (SA_TIG << 2) + SA_RPL0)

/* GDT 的虚拟地址 */
#define GDT_VADDR			0x80200000
#define GDT_LIMIT		    0x000007ff

#define GDT_OFF2PTR(gdt, off)    (gdt + off) 

#define GDT_BOUND_BOTTOM   0
#define GDT_BOUND_TOP      0xffffffff

#define GDT_KERNEL_CODE_ATTR        (DA_CR | DA_DPL0 | DA_32 | DA_G)
#define GDT_KERNEL_DATA_ATTR        (DA_DRW | DA_DPL0 | DA_32 | DA_G)
#define GDT_USER_CODE_ATTR          (DA_CR | DA_DPL3 | DA_32 | DA_G)
#define GDT_USER_DATA_ATTR          (DA_DRW | DA_DPL3 | DA_32 | DA_G)
#define GDT_TSS_ATTR                (DA_386TSS)

/*
段描述符结构
*/
struct segment_descriptor {
	unsigned short limit_low, base_low;
	unsigned char base_mid, access_right;
	unsigned char limit_high, base_high;
};

//初始化段描述符
void segment_descriptor_init();

#endif	/*_X86_SEGMENT_H*/
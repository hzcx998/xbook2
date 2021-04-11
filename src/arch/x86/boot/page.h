#ifndef _BOOT_PAGE_H
#define _BOOT_PAGE_H

/* 把内核放到低地址 */
#define KERN_LOWMEN 1

/* 分页机制 */
#define KERN_PAGE_DIR_PHY_ADDR 0x201000    /* 内核页目录表地址 */
#define PAGE_TBL_PHY_ADDR 0x202000    /* 内核页表地址 */

/* 页面属性 */
#define PAGE_P_1	  	1	/* 0001 exist in memory */ 
#define PAGE_P_0	  	0	/* 0000 not exist in memory */
#define PAGE_RW_R  	    0	/* 0000 R/W read/execute */
#define PAGE_RW_W  	    2	/* 0010 R/W read/write/execute */
#define PAGE_US_S  	    0	/* 0000 U/S system level, cpl0,1,2 */
#define PAGE_US_U  	    4	/* 0100 U/S user level, cpl3 */

#define KERN_PG   (PAGE_US_S | PAGE_RW_W | PAGE_P_1)

#define PAGE_SIZE  	    4096

void setup_paging();

#endif  /* _BOOT_PAGE_H */

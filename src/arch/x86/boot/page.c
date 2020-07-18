#include "page.h"
#include "lib.h"

void enable_paging(unsigned int paddr);

void setup_paging()
{
    print_str("setup_paging.\n");

    unsigned int *pgdir = (unsigned int *) PAGE_DIR_PHY_ADDR;
    unsigned int *pgtbl = (unsigned int *) PAGE_TBL_PHY_ADDR;
    
    /* clear page dir table */
    memset(pgdir, 0, PAGE_SIZE);

    unsigned int phy_addr = 0;
    /* fill page table, 8MB memory */
    int i;
    for (i = 0; i < 1024 * 2; i++) {
        pgtbl[i] = phy_addr | KERN_PG;
        phy_addr += PAGE_SIZE;
    }

    /* fill page dir table, low 8M (0~8M), high 8M(0x80000000~0x80800000) */
    pgdir[0] = (unsigned int) pgtbl | KERN_PG;
    pgdir[512] = (unsigned int) pgtbl | KERN_PG;
    pgtbl += 1024;
    pgdir[1] = (unsigned int) pgtbl | KERN_PG;
    pgdir[513] = (unsigned int) pgtbl | KERN_PG;
    pgdir[1023] = (unsigned int) pgdir |KERN_PG;    /* record pgdir self */

    /*  */
    enable_paging((unsigned int) pgdir);
    
    print_str("enable page mode done.\n");
    
}

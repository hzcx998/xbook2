#include <fsal/dir.h>
#include <string.h>
#include <assert.h>
#include <xbook/memalloc.h>

fsal_dir_t *fsal_dir_table;
DEFINE_SPIN_LOCK(fsal_dir_table_lock);

int fsal_dir_table_init()
{
    fsal_dir_table = mem_alloc(FSAL_DIR_OPEN_NR * sizeof(fsal_dir_t));
    if (fsal_dir_table == NULL) 
        return -1;
    memset(fsal_dir_table, 0, FSAL_DIR_OPEN_NR * sizeof(fsal_dir_t));
    return 0;
}

fsal_dir_t *fsal_dir_alloc()
{
    unsigned long irq_flags;
    spin_lock_irqsave(&fsal_dir_table_lock, irq_flags);
    int i;
    for (i = 0; i < FSAL_DIR_OPEN_NR; i++) {
        if (!fsal_dir_table[i].flags) {
            memset(&fsal_dir_table[i], 0, sizeof(fsal_dir_t));
            fsal_dir_table[i].flags = FSAL_DIR_USED;
            fsal_dir_table[i].fsal = NULL;
            fsal_dir_table[i].extension = NULL;
            spin_unlock_irqrestore(&fsal_dir_table_lock, irq_flags);
            return &fsal_dir_table[i];
        }
    }
    spin_unlock_irqrestore(&fsal_dir_table_lock, irq_flags);
    return NULL;
}

int fsal_dir_free(fsal_dir_t *dir)
{
    unsigned long irq_flags;
    spin_lock_irqsave(&fsal_dir_table_lock, irq_flags);
    if (!dir->flags) {
        spin_unlock_irqrestore(&fsal_dir_table_lock, irq_flags);
        return -1;
    }
    dir->flags = 0;
    spin_unlock_irqrestore(&fsal_dir_table_lock, irq_flags);
    return 0;
}

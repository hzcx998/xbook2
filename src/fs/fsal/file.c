#include <fsal/fsal.h>
#include <fsal/fatfs.h>
#include <fsal/dir.h>

#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <stdio.h>

#include <xbook/memalloc.h>
#include <xbook/debug.h>
#include <xbook/fs.h>
#include <xbook/schedule.h>

// #define DEBUG_FSAL

fsal_file_t *fsal_file_table;
DEFINE_SPIN_LOCK(fsal_file_table_lock);

int fsal_file_table_init()
{
    fsal_file_table = mem_alloc(FSAL_FILE_OPEN_NR * sizeof(fsal_file_t));
    if (fsal_file_table == NULL) 
        return -1;
    memset(fsal_file_table, 0, FSAL_FILE_OPEN_NR * sizeof(fsal_file_t));
    return 0;
}

fsal_file_t *fsal_file_alloc()
{
    unsigned long irq_flags;
    spin_lock_irqsave(&fsal_file_table_lock, irq_flags);
    int i;
    fsal_file_t *file;
    for (i = 0; i < FSAL_FILE_OPEN_NR; i++) {
        file = &fsal_file_table[i];
        if (!file->flags) {
            memset(file, 0, sizeof(fsal_file_t));
            file->flags = FSAL_FILE_FLAG_USED;
            spin_unlock_irqrestore(&fsal_file_table_lock, irq_flags);
            return file;
        }
    }
    spin_unlock_irqrestore(&fsal_file_table_lock, irq_flags);
    return NULL;
}

int fsal_file_free(fsal_file_t *file)
{
    if (!file->flags)
        return -1;
    unsigned long irq_flags;
    spin_lock_irqsave(&fsal_file_table_lock, irq_flags);
    file->flags = 0;
    spin_unlock_irqrestore(&fsal_file_table_lock, irq_flags);
    return 0;
}
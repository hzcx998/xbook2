#include <fsal/dir.h>
#include <string.h>
#include <assert.h>
#include <xbook/memalloc.h>

fsal_dir_t *fsal_dir_table;

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
    int i;
    for (i = 0; i < FSAL_DIR_OPEN_NR; i++) {
        if (!fsal_dir_table[i].flags) {
            memset(&fsal_dir_table[i], 0, sizeof(fsal_dir_t));
            fsal_dir_table[i].flags = FSAL_DIR_USED;
            return &fsal_dir_table[i];
        }
    }
    return NULL;
}

int fsal_dir_free(fsal_dir_t *dir)
{
    if (!dir->flags)
        return -1;
    dir->flags = 0;
    return 0;
}

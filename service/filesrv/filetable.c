#include <ff.h>
#include <stdio.h>
#include <stdlib.h>
#include "filesrv.h"
#include <string.h>

filesrv_file_t *filesrv_file_table;

int filesrv_file_table_init()
{
    filesrv_file_table = malloc(FILESRV_FILE_OPEN_NR * sizeof(filesrv_file_t));
    if (filesrv_file_table == NULL) 
        return -1;
    memset(filesrv_file_table, 0, FILESRV_FILE_OPEN_NR * sizeof(filesrv_file_t));
    return 0;
}

filesrv_file_t *filesrv_alloc_file()
{
    int i;
    for (i = 0; i < FILESRV_FILE_OPEN_NR; i++) {
        if (filesrv_file_table[i].flags == 0) {
            filesrv_file_table[i].flags = 1;
            memset(&filesrv_file_table[i].fil, 0, sizeof(FIL));
            return &filesrv_file_table[i];
        }
    }
    return NULL;
}

int filesrv_free_file(filesrv_file_t *file)
{
    if (!file->flags)
        return -1;
    file->flags = 0;
    return 0;
}

int filesrv_free_file2(FIL *fil)
{
    int i;
    for (i = 0; i < FILESRV_FILE_OPEN_NR; i++) {
        if (&filesrv_file_table[i].fil == fil) {
            return filesrv_free_file(&filesrv_file_table[i]);
        }
    }
    return -1;
}

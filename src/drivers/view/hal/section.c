#include <sys/ipc.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "drivers/view/hal.h"
#if 0 /* share memory */
int view_section_open(view_section_t *section)
{
    static int unique_id = 0;
    char name[32] = {0};
    sprintf(name, "xbrower section%d", unique_id++);
    section->handle = shmget(name, section->size, IPC_CREAT | IPC_EXCL);
    if (section->handle < 0) {
        return -1;
    }
    section->addr = shmmap(section->handle, NULL, IPC_RND);
    if (section->addr == (void *)-1) {
        shmput(section->handle);
        return -1;
    }
    return 0;
}

int view_section_close(view_section_t *section)
{
    if (section->handle < 0 || !section->addr)
        return -1;
    shmunmap(section->addr, IPC_RND);
    shmput(section->handle);
    return 0;
}
#endif

int view_section_open(view_section_t *section)
{
    section->addr = mem_alloc(section->size);
    if (section->addr == NULL) {
        return -1;
    }
    return 0;
}

int view_section_close(view_section_t *section)
{
    if (!section->addr)
        return -1;
    free(section->addr);
    return 0;
}
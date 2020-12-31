#include <sys/ipc.h>
#include <string.h>
#include <stdio.h>
#include "xgui_hal.h"

int xgui_section_open(xgui_section_t *section)
{
    static int unique_id = 0;
    char name[32] = {0};
    sprintf(name, "xgui section%d", unique_id++);
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

int xgui_section_close(xgui_section_t *section)
{
    if (section->handle < 0 || !section->addr)
        return -1;
    shmunmap(section->addr, IPC_RND);
    shmput(section->handle);
    return 0;
}
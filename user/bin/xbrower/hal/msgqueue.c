#include <sys/ipc.h>
#include <string.h>
#include <stdio.h>
#include "xgui_hal.h"

int xgui_msgqueue_open(xgui_msgqueue_t *msgqueue)
{
    static int unique_id = 0;
    char name[32] = {0};
    sprintf(name, "xgui msgqueue%d", unique_id++);
    msgqueue->handle = msgget(name, IPC_CREAT | IPC_EXCL);
    if (msgqueue->handle < 0) {
        return -1;
    }
    return 0;
}

int xgui_msgqueue_close(xgui_msgqueue_t *msgqueue)
{
    if (msgqueue->handle < 0)
        return -1;
    msgput(msgqueue->handle);
    return 0;
}
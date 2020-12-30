#include "xgui_hal.h"
#include <stdint.h>
#include <stdio.h>

static xgui_msgqueue_t msgqueue_table[GUISERV_MSGQUEUE_NR];

static xgui_msgqueue_t *xgui_msgqueue_alloc()
{
    xgui_msgqueue_t *msgqueue;
    int i; for (i = 0; i < GUISERV_MSGQUEUE_NR; i++) {
        msgqueue = &msgqueue_table[i];
        if (msgqueue->handle == -1) {
            return msgqueue;
        }
    }
    return NULL;
}

static void xgui_msgqueue_free(xgui_msgqueue_t *msgqueue)
{
    assert(msgqueue >= msgqueue_table && msgqueue < &msgqueue_table[GUISERV_MSGQUEUE_NR]);
    msgqueue->handle = 0;
}

xgui_msgqueue_t *xgui_msgqueue_get_ptr(int msgqueue_id)
{
    assert(msgqueue_id >= 0 && msgqueue_id < GUISERV_MSGQUEUE_NR);
    return msgqueue_table + msgqueue_id;
}

int xgui_msgqueue_get_id(xgui_msgqueue_t *msgqueue)
{
    assert(msgqueue >= msgqueue_table && msgqueue < &msgqueue_table[GUISERV_MSGQUEUE_NR]);
    return msgqueue - msgqueue_table;
}

xgui_msgqueue_t *xgui_msgqueue_create()
{
    xgui_msgqueue_t *msgq = xgui_msgqueue_alloc();
    if (!msgq)
        return NULL;
    if (xgui_msgqueue_open(msgq) < 0)
        return NULL;
    return msgq;
}

int xgui_msgqueue_destroy(xgui_msgqueue_t *msgq)
{
    if (!msgq)
        return -1;
    xgui_msgqueue_close(msgq);
    xgui_msgqueue_free(msgq);
    return 0;
}

int xgui_msgqueue_init()
{
    xgui_msgqueue_t *msgqueue;
    int i; for (i = 0; i < GUISERV_MSGQUEUE_NR; i++) {
        msgqueue = &msgqueue_table[i];
        msgqueue->handle = -1;
    }
    return 0;
}

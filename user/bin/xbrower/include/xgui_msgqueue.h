#ifndef _XGUI_MSGQUEUE_H
#define _XGUI_MSGQUEUE_H

#define GUISERV_MSGQUEUE_NR 32

typedef struct {
    int handle;
} xgui_msgqueue_t;

int xgui_msgqueue_init();

xgui_msgqueue_t *xgui_msgqueue_create();
int xgui_msgqueue_destroy(xgui_msgqueue_t *msgq);
xgui_msgqueue_t *xgui_msgqueue_get_ptr(int msgqueue_id);
int xgui_msgqueue_get_id(xgui_msgqueue_t *msgqueue);

#endif /* _XGUI_MSGQUEUE_H */

#ifndef _XGUI_VIEW_H
#define _XGUI_VIEW_H

#include <stddef.h>
#include <sys/list.h>
#include "xgui_section.h"

/* 视图用来表达逻辑上的图层 */
typedef struct {
    list_t list;        // 视图构成一个有序链表
    int width;
    int height;
    int x;
    int y;
    int z;
    xgui_section_t *section;
} xgui_view_t;

int xgui_view_init();
xgui_view_t *xgui_view_new(int x, int y, int width, int height);
int xgui_view_put(xgui_view_t *view);
int xgui_view_show(xgui_view_t *view);
int xgui_view_hide(xgui_view_t *view);
int xgui_view_refresh(xgui_view_t *view);

#endif /* _XGUI_VIEW_H */
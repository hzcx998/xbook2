#ifndef _LIB_XTK_VIEW_H
#define _LIB_XTK_VIEW_H

#include <stddef.h>
#include <sys/list.h>
#include "xtk_spirit.h"

typedef struct {
    list_t list;
    int view;
    list_t spirit_list_head;
} xtk_view_t;

extern list_t xtk_view_list_head;

#define xtk_view_for_each(view) \
        list_for_each_owner(view, &xtk_view_list_head, list)

xtk_view_t *xtk_view_create();
int xtk_view_destroy(xtk_view_t *view);
void xtk_view_add(xtk_view_t *view);
void xtk_view_remove(xtk_view_t *view);

#endif /* _LIB_XTK_VIEW_H */
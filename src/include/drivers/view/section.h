#ifndef _XBOOK_DRIVERS_VIEW_SECTION_H
#define _XBOOK_DRIVERS_VIEW_SECTION_H

#include <stddef.h>
#include <xbook/list.h>
#include <drivers/view/color.h>

#define VIEW_SECTION_NR 32

typedef struct {
    list_t list;
    int handle;
    void *addr;  // 共享内存地址
    int width;
    int height;
    int flags;
    size_t size;
} view_section_t;

view_section_t *view_section_get_ptr(int section_id);
int view_section_get_id(view_section_t *section);
view_section_t *view_section_create(int width, int height);
int view_section_destroy(view_section_t *section);
int view_section_clear(view_section_t *section);

int view_section_fill_rect(view_section_t *section, view_color_t color);
int view_section_init();
int view_section_exit();

#endif /* _XBOOK_DRIVERS_VIEW_SECTION_H */
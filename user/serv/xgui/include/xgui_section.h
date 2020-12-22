#ifndef _XGUI_SECTION_H
#define _XGUI_SECTION_H

#include <stddef.h>
#include "xgui_color.h"

#define XGUI_SECTION_NR 32

#define XGUI_SECTION_USING 0X01

typedef struct {
    int handle;
    void *addr;  // 共享内存地址
    int width;
    int height;
    int flags;
    size_t size;
} xgui_section_t;

xgui_section_t *xgui_section_get_ptr(int section_id);
int xgui_section_get_id(xgui_section_t *section);
xgui_section_t *xgui_section_new(int width, int height);
int xgui_section_put(xgui_section_t *section);

int xgui_section_fill_rect(xgui_section_t *section, xgui_color_t color);

int xgui_section_init();

#endif /* _XGUI_SECTION_H */
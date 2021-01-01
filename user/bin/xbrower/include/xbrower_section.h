#ifndef _XGUI_SECTION_H
#define _XGUI_SECTION_H

#include <stddef.h>
#include <xbrower_color.h>

#define XGUI_SECTION_NR 32

#define XGUI_SECTION_USING 0X01

typedef struct {
    int handle;
    void *addr;  // 共享内存地址
    int width;
    int height;
    int flags;
    size_t size;
} xbrower_section_t;

xbrower_section_t *xbrower_section_get_ptr(int section_id);
int xbrower_section_get_id(xbrower_section_t *section);
xbrower_section_t *xbrower_section_create(int width, int height);
int xbrower_section_destroy(xbrower_section_t *section);

int xbrower_section_fill_rect(xbrower_section_t *section, xbrower_color_t color);
int xbrower_section_init();
int xbrower_section_exit();

#endif /* _XGUI_SECTION_H */
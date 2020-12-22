#include "xgui_hal.h"
#include <sys/ipc.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

static xgui_section_t section_table[XGUI_SECTION_NR];

static xgui_section_t *xgui_section_alloc(int width, int height)
{
    xgui_section_t *section;
    int i; for (i = 0; i < XGUI_SECTION_NR; i++) {
        section = &section_table[i];
        if (!section->flags) {
            section->width = width;
            section->height = height;
            section->size = width * height * sizeof(xgui_color_t);
            return section;
        }
    }
    return NULL;
}

static void xgui_section_free(xgui_section_t *section)
{
    assert(section >= section_table && section < &section_table[XGUI_SECTION_NR]);
    section->flags = 0;
}

xgui_section_t *xgui_section_get_ptr(int section_id)
{
    assert(section_id >= 0 && section_id < XGUI_SECTION_NR);
    return section_table + section_id;
}

int xgui_section_get_id(xgui_section_t *section)
{
    assert(section >= section_table && section < &section_table[XGUI_SECTION_NR]);
    return section - section_table;
}

xgui_section_t *xgui_section_new(int width, int height)
{
    xgui_section_t *section = xgui_section_alloc(width, height);
    if (!section) {
        printf("alloc section failed!\n");
        return NULL;
    }
    if (xgui_section_open(section) < 0) {
        printf("oepn section failed!\n");
        return NULL;
    }
    section->flags |= XGUI_SECTION_USING;
    printf("section id=%d, addr=%x\n", section->handle, section->addr);
    memset(section->addr, 0, section->size);
    return section;
}

int xgui_section_put(xgui_section_t *section)
{
    if (!section)
        return -1;
    if (xgui_section_close(section) < 0) {
        printf("close section failed!\n");
        return -1;
    }
    xgui_section_free(section);
    return 0;
}

int xgui_section_fill_rect(xgui_section_t *section, xgui_color_t color)
{
    if (!section)
        return -1;
    xgui_color_t *buf = (xgui_color_t *)section->addr;
    int x, y;
    for (y = 0; y < section->height; y++) {
        for (x = 0; x < section->width; x++) {
            buf[y * section->width + x] = color;
        }
    }
    return 0;
}

/* 服务端根据客户端的消息，选择一个section，并根据区域进行合并 */

/* 所有有效的节都放在一个节指针表中，对它进行操作，来进行排序 */

int xgui_section_init()
{
    memset(section_table, 0, sizeof(xgui_section_t) * XGUI_SECTION_NR);
    return 0;
}

#include "xbrower_hal.h"
#include <sys/ipc.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

static xbrower_section_t section_table[XGUI_SECTION_NR];

static xbrower_section_t *xbrower_section_alloc(int width, int height)
{
    xbrower_section_t *section;
    int i; for (i = 0; i < XGUI_SECTION_NR; i++) {
        section = &section_table[i];
        if (!section->flags) {
            section->width = width;
            section->height = height;
            section->size = width * height * sizeof(xbrower_color_t);
            return section;
        }
    }
    return NULL;
}

static void xbrower_section_free(xbrower_section_t *section)
{
    assert(section >= section_table && section < &section_table[XGUI_SECTION_NR]);
    section->flags = 0;
}

xbrower_section_t *xbrower_section_get_ptr(int section_id)
{
    assert(section_id >= 0 && section_id < XGUI_SECTION_NR);
    return section_table + section_id;
}

int xbrower_section_get_id(xbrower_section_t *section)
{
    assert(section >= section_table && section < &section_table[XGUI_SECTION_NR]);
    return section - section_table;
}

xbrower_section_t *xbrower_section_create(int width, int height)
{
    xbrower_section_t *section = xbrower_section_alloc(width, height);
    if (!section) {
        printf("alloc section failed!\n");
        return NULL;
    }
    if (xbrower_section_open(section) < 0) {
        printf("oepn section failed!\n");
        return NULL;
    }
    section->flags |= XGUI_SECTION_USING;
    memset(section->addr, 0, section->size);
    return section;
}

int xbrower_section_destroy(xbrower_section_t *section)
{
    if (!section)
        return -1;
    if (xbrower_section_close(section) < 0) {
        return -1;
    }
    xbrower_section_free(section);
    return 0;
}

int xbrower_section_fill_rect(xbrower_section_t *section, xbrower_color_t color)
{
    if (!section)
        return -1;
    xbrower_color_t *buf = (xbrower_color_t *)section->addr;
    int x, y;
    for (y = 0; y < section->height; y++) {
        for (x = 0; x < section->width; x++) {
            buf[y * section->width + x] = color;
        }
    }
    return 0;
}

int xbrower_section_init()
{
    memset(section_table, 0, sizeof(xbrower_section_t) * XGUI_SECTION_NR);
    return 0;
}

int xbrower_section_exit()
{
    xbrower_section_t *section;
    int i; for (i = 0; i < XGUI_SECTION_NR; i++) {
        section = &section_table[i];
        if (section->flags) {
            xbrower_section_destroy(section);
        }
    }
    return 0;
}

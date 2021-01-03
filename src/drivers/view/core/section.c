#include "drivers/view/hal.h"
#include <sys/ipc.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

static view_section_t section_table[VIEW_SECTION_NR];

static view_section_t *view_section_alloc(int width, int height)
{
    view_section_t *section;
    int i; for (i = 0; i < VIEW_SECTION_NR; i++) {
        section = &section_table[i];
        if (!section->flags) {
            section->width = width;
            section->height = height;
            section->size = width * height * sizeof(view_color_t);
            return section;
        }
    }
    return NULL;
}

static void view_section_free(view_section_t *section)
{
    assert(section >= section_table && section < &section_table[VIEW_SECTION_NR]);
    section->flags = 0;
}

view_section_t *view_section_get_ptr(int section_id)
{
    assert(section_id >= 0 && section_id < VIEW_SECTION_NR);
    return section_table + section_id;
}

int view_section_get_id(view_section_t *section)
{
    assert(section >= section_table && section < &section_table[VIEW_SECTION_NR]);
    return section - section_table;
}

view_section_t *view_section_create(int width, int height)
{
    view_section_t *section = view_section_alloc(width, height);
    if (!section) {
        keprint("alloc section failed!\n");
        return NULL;
    }
    if (view_section_open(section) < 0) {
        keprint("oepn section failed!\n");
        return NULL;
    }
    section->flags |= VIEW_SECTION_USING;
    memset(section->addr, 0, section->size);
    return section;
}

int view_section_destroy(view_section_t *section)
{
    if (!section)
        return -1;
    if (view_section_close(section) < 0) {
        return -1;
    }
    view_section_free(section);
    return 0;
}

int view_section_clear(view_section_t *section)
{
    if (!section)
        return -1;
    memset(section->addr, 0, section->size);
    return 0;
}

int view_section_init()
{
    memset(section_table, 0, sizeof(view_section_t) * VIEW_SECTION_NR);
    return 0;
}

int view_section_exit()
{
    view_section_t *section;
    int i; for (i = 0; i < VIEW_SECTION_NR; i++) {
        section = &section_table[i];
        if (section->flags) {
            view_section_destroy(section);
        }
    }
    return 0;
}

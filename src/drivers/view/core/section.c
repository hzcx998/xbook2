#include "drivers/view/hal.h"
#include <sys/ipc.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <xbook/memcache.h>

static LIST_HEAD(view_section_list_head);

static view_section_t *view_section_alloc(int width, int height)
{
    view_section_t *section = mem_alloc(sizeof(view_section_t));
    if (!section)
        return NULL;
    section->handle = -1;
    section->addr = NULL;
    section->width = width;
    section->height = height;
    section->size = width * height * sizeof(view_color_t);
    list_add(&section->list, &view_section_list_head);
    return section;
}

static void view_section_free(view_section_t *section)
{
    if (section) {
        list_del(&section->list);
        mem_free(section);
    }
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
    list_init(&view_section_list_head);
    return 0;
}

int view_section_exit()
{
    view_section_t *section, *next;
    list_for_each_owner_safe (section, next, &view_section_list_head, list) {
        view_section_free(section);
    }
    return 0;
}

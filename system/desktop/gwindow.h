#ifndef _G_WINDOW_H
#define _G_WINDOW_H

#include <stdint.h>
#include <sys/list.h>

#define GW_TITLE_LEN    64

typedef struct {
    list_t wlist;
    int layer;
    uint32_t flags;
    uint32_t width;     /* window width */
    uint32_t height;    /* window height */
    char title[GW_TITLE_LEN];
} g_window_t;


int g_new_window(char *title, int x, int y, uint32_t width, uint32_t height, uint32_t flags);
int g_del_window(int win);

int g_show_window(int win);
int g_hide_window(int win);
int g_update_window(int win);

#endif  /* _G_WINDOW_H */
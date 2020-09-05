#ifndef _DESKTOP_ICON_H
#define _DESKTOP_ICON_H

#include <gapi.h>
#include <stddef.h>
#include <types.h>

#include <sys/list.h>

#define ICON_SIZE   32 
#define ICON_BODY   64
#define ICON_DISTANCE   2

#define ICON_TEXT_LEN 8

typedef struct {
    int x;
    int y;
    uint32_t width;
    uint32_t height;
    bool_t selected;
    g_touch_t *gtch;    /* 触碰块 */
    g_bitmap_t *icon_bmp;    /* 图片位图 */
    g_bitmap_t *char_bmp;    /* 文字位图 */
    char path[MAX_PATH]; /* 可执行文件路径 */
    char text[ICON_TEXT_LEN + 1]; /* 图标文本 */  
} icon_t; 

typedef struct {
    int icon_x;
    int icon_y;
    list_t touch_list;
} icon_man_t;

extern icon_man_t icon_man;

icon_t *new_icon(char *pic, char *path, char *text);
int del_icon(icon_t *icon);
int icon_show(icon_t *icon);
int init_icon();
#endif  /* _DESKTOP_ICON_H */

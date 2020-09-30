#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>

#include <iniparser.h>

#include "icon.h"
#include "desktop.h"
#include "taskbar.h"

icon_man_t icon_man;

#define ICON_NR 3

/**
 * [0]: icon path
 * [1]: app path
 * [2]: icon text
 * 
 */
char *icon_table[ICON_NR][3] = {
    {
        "/res/icon/bosh.png",
        "/bin/bosh",
        "bosh"
    },
    {
        "/res/icon/infones.png",
        "/bin/infones",
        "infones"
    },
    {
        "/res/icon/win.jpg",
        "/bin/win",
        "win"
    }
};

static int icon_click_handler(void *arg)
{
    g_touch_t *gtch = (g_touch_t *) arg;
    if (!gtch)
        return -1;
    icon_t *icon = (icon_t *) gtch->extension;
    if (!icon)
        return -1;
    
    /* 把文字加载到位图中 */
    int len = strlen(icon->text);
    int x = 0; /* 左边对齐 */
    if (len <= 8) {
        /* 居中对齐 */
        x = icon->width / 2 - len * 8 / 2;
    }
    
    if (icon->selected == false) {
        icon->selected = true;
        /* 将文字置红 */
        g_text(icon->char_bmp, x, 2, icon->text, GC_RED);
    
    } else {
        icon->selected = false;
        /* 将文字置红 */
        g_text(icon->char_bmp, x, 2, icon->text, GC_WHITE);

        /* 执行程序 */
        desktop_launch_app(icon->path);
    }
    icon_show(icon);
    return 0;
}

icon_t *new_icon(char *pic, char *path, char *text)
{
    icon_t *icon = malloc(sizeof(icon_t));
    if (icon == NULL)
        return NULL;
    icon->x = 0;
    icon->y = 0;
    icon->width = ICON_BODY;
    icon->height = ICON_BODY;
    icon->selected = false;

    icon->gtch = g_new_touch(icon->width, icon->height);
    if (icon->gtch == NULL) {
        free(icon);
        return NULL;
    }
    
    icon->icon_bmp = g_new_bitmap(ICON_SIZE, ICON_SIZE);
    if (icon->icon_bmp == NULL) {
        g_del_touch(icon->gtch);
        free(icon);
        return NULL;
    }
    
    icon->char_bmp = g_new_bitmap(icon->width, 20);
    if (icon->char_bmp == NULL) {
        g_del_bitmap(icon->icon_bmp);
        g_del_touch(icon->gtch);
        free(icon);
        return NULL;
    }

    g_set_touch_handler(icon->gtch, 0, icon_click_handler);
    g_set_touch_layer(icon->gtch, desktop_layer, &icon_man.touch_list);
    g_set_touch_extension(icon->gtch, icon);
    g_set_touch_shape(icon->gtch, G_TOUCH_SHAPE_BORDER);
    g_set_touch_color(icon->gtch, GC_RGB(128, 128, 128), GC_RGB(225, 225, 225));
    
    memset(icon->path, 0, MAX_PATH);
    strcpy(icon->path, path);

    
    int iw, ih, channels_in_file;
    unsigned char *image =  g_load_image(pic, &iw, &ih, &channels_in_file);
    if (image) {
        g_resize_image(image, iw, ih, (unsigned char *) icon->icon_bmp->buffer,
                icon->icon_bmp->width, icon->icon_bmp->height, 4, GRSZ_BILINEAR);
    } else {
        /* 使用默认的位图 */
        g_rectfill(icon->icon_bmp, 0, 0, ICON_SIZE/2, ICON_SIZE/2, GC_RED);
        g_rectfill(icon->icon_bmp, ICON_SIZE/2, 0, ICON_SIZE/2, ICON_SIZE/2, GC_YELLOW);
        g_rectfill(icon->icon_bmp, 0, ICON_SIZE/2, ICON_SIZE/2, ICON_SIZE/2, GC_BLUE);
        g_rectfill(icon->icon_bmp, ICON_SIZE/2, ICON_SIZE/2, ICON_SIZE/2, ICON_SIZE/2, GC_GREEN);
    }
    
    memset(icon->text, 0, ICON_TEXT_LEN + 1);
    strcpy(icon->text, text);

    /* 把文字加载到位图中 */
    int len = strlen(icon->text);
    int x = 0; /* 左边对齐 */
    if (len <= 8) {
        /* 居中对齐 */
        x = icon->width / 2 - len * 8 / 2;
    }
    /* 阴影效果 */
    g_text(icon->char_bmp, x + 1, 2, icon->text, GC_BLACK);
    g_text(icon->char_bmp, x + 1, 2, icon->text, GC_BLACK);
    g_text(icon->char_bmp, x + 1, 2 + 1, icon->text, GC_BLACK);
    g_text(icon->char_bmp, x + 1, 2 - 1, icon->text, GC_BLACK);
    
    g_text(icon->char_bmp, x - 1, 2, icon->text, GC_BLACK);
    g_text(icon->char_bmp, x - 1, 2, icon->text, GC_BLACK);
    g_text(icon->char_bmp, x - 1, 2 + 1, icon->text, GC_BLACK);
    g_text(icon->char_bmp, x - 1, 2 - 1, icon->text, GC_BLACK);
    
    g_text(icon->char_bmp, x, 2, icon->text, GC_WHITE);
    
    return icon;
}

int del_icon(icon_t *icon)
{
    if (!icon)
        return -1;
    g_del_bitmap(icon->icon_bmp);
    g_del_touch(icon->gtch);
    free(icon);
    return 0;
}
/**
 * 显示图标，把图标绘制到桌面上
 */
int icon_locate(icon_t *icon, int x, int y)
{
    if (!icon)
        return -1;
    icon->x = x;
    icon->y = y;
    g_set_touch_location(icon->gtch, icon->x, icon->y);
    return 0;
}

/**
 * 设置图标位置，根据系统自动设置
 */
int icon_locate_all()
{
    g_touch_t *gtch;
    icon_t *icon;
    list_for_each_owner (gtch, &icon_man.touch_list, list) {
        icon = gtch->extension;
        icon_locate(icon, icon_man.icon_x, icon_man.icon_y);
        icon_man.icon_y += ICON_BODY + ICON_DISTANCE;
        if (icon_man.icon_y >= g_screen.height - (ICON_BODY + ICON_DISTANCE)) {
            icon_man.icon_x -= ICON_BODY + ICON_DISTANCE;
            if (icon_man.icon_x <= ICON_BODY) {
                icon_man.icon_x += ICON_BODY + ICON_DISTANCE;
            }
        }
    }
    return 0;
}

/**
 * 显示图标，把图标绘制到桌面上
 */
int icon_show(icon_t *icon)
{
    if (!icon)
        return -1;
    /*  */
    g_paint_touch(icon->gtch);

    g_bitmap_sync(icon->icon_bmp, desktop_layer, icon->x + icon->width / 4, icon->y + icon->height / 8);
    g_bitmap_sync(icon->char_bmp, desktop_layer, icon->x, icon->y + icon->height - 20);
    
    return 0;
}


/**
 * 显示所有图标
 */
void icon_show_all()
{
    g_touch_t *gtch;
    icon_t *icon;
    list_for_each_owner (gtch, &icon_man.touch_list, list) {
        icon = gtch->extension;
        icon_show(icon);
    }
}

#define ICON_CONFIG_PATH    "/etc/icon.ini"

/**
 * 读取配置文件，并创建图标
 * 
 */
int icon_read_config()
{
    int i;
    icon_t *icon;
    char *icon_path, *bin_path, *icon_name;
  
    for (i = 0; i < ICON_NR; i++) {
        icon_path = icon_table[i][0];
        bin_path = icon_table[i][1];
        icon_name = icon_table[i][2];
        icon = new_icon(icon_path, bin_path, icon_name);
        if (icon == NULL) {
            printf("[desktop]: load icon pic path:%s bin path:%s failed!\n", icon_path, bin_path);
        }
    }
    
    return 0;
}

int init_icon()
{
    icon_man.icon_x = g_screen.width - ICON_BODY - ICON_DISTANCE;
    icon_man.icon_y = TASKBAR_HEIGHT + ICON_DISTANCE;
    init_list(&icon_man.touch_list);
    /* 加载所有图标 */
    icon_read_config();
    icon_locate_all();
    icon_show_all();
    return 0;
}
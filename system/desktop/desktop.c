#include "desktop.h"
#include <gapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>

int desktop_layer;
g_bitmap_t *desktop_bitmap;

/**
 * 启动程序
 * @path: 程序的路径
 */
int desktop_launch_app(char *path)
{
    int pid = fork();
    if (pid < 0) {
        printf("[desktop]: fork failed!\n");
        return -1;
    }
    if (!pid) { /* 子进程就执行其他程序 */
        /* 配置环境变量 */
        exit(execv(path, NULL));
    }
    return 0;
}

#define BG_PIC_PATH "/res/bg.jpg"

int init_desktop()
{
    /* 桌面 */
    int layer = g_new_layer(0, 0, g_screen.width, g_screen.height);
    if (layer < 0) {
        printf("[desktop]: new desktop layer failed!\n");
        return -1;
    }
    desktop_layer = layer;
    g_set_layer_z(layer, 0);    /* 0 is desktop */

    
    desktop_bitmap = g_new_bitmap(g_screen.width, g_screen.height);
    if (desktop_bitmap == NULL)
        return -1;
    
    g_set_layer_focus(layer);   /* set as focus layer */
    g_set_layer_desktop(layer); /* set as focus desktop */
    
    /* 加载背景图片 */
    char *filename = BG_PIC_PATH;
    unsigned char *image = NULL;
    int iw, ih, channels_in_file;
    image =  g_load_image(filename, &iw, &ih, &channels_in_file);
    if (image) {
        g_resize_image(image, iw, ih, (unsigned char *) desktop_bitmap->buffer, g_screen.width, g_screen.height, 4, GRSZ_BILINEAR);
    } else {
        g_rectfill(desktop_bitmap, 0, 0, g_screen.width, g_screen.height, GC_GRAY);
    }
    g_bitmap_sync(desktop_bitmap, desktop_layer, 0, 0);
    return 0;
}



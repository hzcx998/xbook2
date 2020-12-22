#include "xgui_hal.h"
#include <stdint.h>
#include <stdio.h>

static xgui_screen_t xgui_screen;

static int screen_out_pixel8(int x, int y, xgui_color_t color)
{
    uint32_t  r, g, b;
    b = color&0xC0;
    g = color&0xE000;
    r = color&0xE00000;
    *(xgui_screen.vram_start + (xgui_screen.width)*y+x) = 
        (unsigned char)((b>>6)|(g>>11)|(r>>16));
    return  0;
}

static int screen_out_pixel15(int x, int y, xgui_color_t color)
{
    uint32_t  r, g, b;
    b = color&0xF8;
    g = color&0xF800;
    r = color&0xF80000;
    *((short int*)((xgui_screen.vram_start) + 2*((xgui_screen.width)*y+x))) = 
        (short int)((b>>3)|(g>>6)|(r>>9));
    return  0;
}

static int screen_out_pixel16(int x, int y, xgui_color_t color)
{
    uint32_t  r, g, b;
    b = color&0xF8;
    g = color&0xFC00;
    r = color&0xF80000;
    *((short*)((xgui_screen.vram_start) + 2*((xgui_screen.width)*y+x))) = 
        (short)((b>>3)|(g>>5)|(r>>8));
    return  0;
}

static int screen_out_pixel24(int x, int y, xgui_color_t color)
{
    *((xgui_screen.vram_start) + 3*((xgui_screen.width)*y+x) + 0) = color&0xFF;
    *((xgui_screen.vram_start) + 3*((xgui_screen.width)*y+x) + 1) = (color&0xFF00) >> 8;
    *((xgui_screen.vram_start) + 3*((xgui_screen.width)*y+x) + 2) = (color&0xFF0000) >> 16;
    return  0;
}

static int screen_out_pixel32(int x, int y, xgui_color_t color)
{
    *((unsigned int*)((xgui_screen.vram_start) + 4*((xgui_screen.width)*y+x))) = (unsigned int)color;
    return  0;
}

int xgui_screen_init()
{
    if (xgui_screen_open(&xgui_screen) < 0) {
        return -1;
    }
    switch (xgui_screen.bpp) 
    {
    case 8:
        xgui_screen.out_pixel = screen_out_pixel8;
        break;
    case 15:
        xgui_screen.out_pixel = screen_out_pixel15;
        break;
    case 16:
        xgui_screen.out_pixel = screen_out_pixel16;
        break;
    case 24:
        xgui_screen.out_pixel = screen_out_pixel24;
        break;
    case 32:
        xgui_screen.out_pixel = screen_out_pixel32;
        break;
    default:
        printf("xgui: unknown screen bpp\n");
        xgui_screen_close(&xgui_screen);
        return -1;
    }
    return 0;
}

void xgui_screen_write_pixel(int x, int y, xgui_color_t color) 
{
    xgui_screen.out_pixel(x, y, color);
}

/**
 * x: 屏幕横坐标
 * y: 屏幕纵坐标
 * rect: 在矩阵中绘制某个矩形区域
 * matrix: 需要绘制的矩阵
 */
void xgui_screen_write_bitmap(int x, int y, xgui_bitmap_t *bitmap) 
{
    int x0, y0;
    for (y0 = 0; y0 < bitmap->region.h; y0++) {
        int martix_y = bitmap->region.y + y0;
        for (x0 = 0; x0 < bitmap->region.w; x0++) {
            int martix_x = bitmap->region.x + x0;
            xgui_color_t color = bitmap->colors[martix_y * bitmap->width + martix_x];
            xgui_screen_write_pixel(x + martix_x, y + martix_y, color);
        }
    }
}
int xgui_screen_get_width()
{
    return xgui_screen.width;
}

int xgui_screen_get_height()
{
    return xgui_screen.height;
}

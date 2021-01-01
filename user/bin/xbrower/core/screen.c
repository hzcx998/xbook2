#include "xbrower_hal.h"
#include <stdint.h>
#include <stdio.h>

xbrower_screen_t xbrower_screen;

static int screen_out_pixel8(int x, int y, xbrower_color_t color)
{
    uint32_t  r, g, b;
    b = color&0xC0;
    g = color&0xE000;
    r = color&0xE00000;
    *(xbrower_screen.vram_start + (xbrower_screen.width)*y+x) = 
        (unsigned char)((b>>6)|(g>>11)|(r>>16));
    return  0;
}

static int screen_out_pixel15(int x, int y, xbrower_color_t color)
{
    uint32_t  r, g, b;
    b = color&0xF8;
    g = color&0xF800;
    r = color&0xF80000;
    *((short int*)((xbrower_screen.vram_start) + 2*((xbrower_screen.width)*y+x))) = 
        (short int)((b>>3)|(g>>6)|(r>>9));
    return  0;
}

static int screen_out_pixel16(int x, int y, xbrower_color_t color)
{
    uint32_t  r, g, b;
    b = color&0xF8;
    g = color&0xFC00;
    r = color&0xF80000;
    *((short*)((xbrower_screen.vram_start) + 2*((xbrower_screen.width)*y+x))) = 
        (short)((b>>3)|(g>>5)|(r>>8));
    return  0;
}

static int screen_out_pixel24(int x, int y, xbrower_color_t color)
{
    *((xbrower_screen.vram_start) + 3*((xbrower_screen.width)*y+x) + 0) = color&0xFF;
    *((xbrower_screen.vram_start) + 3*((xbrower_screen.width)*y+x) + 1) = (color&0xFF00) >> 8;
    *((xbrower_screen.vram_start) + 3*((xbrower_screen.width)*y+x) + 2) = (color&0xFF0000) >> 16;
    return  0;
}

static int screen_out_pixel32(int x, int y, xbrower_color_t color)
{
    *((unsigned int*)((xbrower_screen.vram_start) + 4*((xbrower_screen.width)*y+x))) = (unsigned int)color;
    return  0;
}

void xbrower_screen_write_pixel(int x, int y, xbrower_color_t color) 
{
    xbrower_screen.out_pixel(x, y, color);
}

int xbrower_screen_init()
{
    if (xbrower_screen_open(&xbrower_screen) < 0) {
        return -1;
    }
    switch (xbrower_screen.bpp) 
    {
    case 8:
        xbrower_screen.out_pixel = screen_out_pixel8;
        break;
    case 15:
        xbrower_screen.out_pixel = screen_out_pixel15;
        break;
    case 16:
        xbrower_screen.out_pixel = screen_out_pixel16;
        break;
    case 24:
        xbrower_screen.out_pixel = screen_out_pixel24;
        break;
    case 32:
        xbrower_screen.out_pixel = screen_out_pixel32;
        break;
    default:
        printf("xbrower: unknown screen bpp\n");
        xbrower_screen_close(&xbrower_screen);
        return -1;
    }
    return 0;
}

int xbrower_screen_exit()
{
    xbrower_screen_close(&xbrower_screen);
    return 0;
}
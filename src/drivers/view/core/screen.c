#include "drivers/view/hal.h"
#include <stdint.h>
#include <xbook/debug.h>

view_screen_t view_screen;

static int screen_out_pixel8(int x, int y, view_color_t color)
{
    uint32_t  r, g, b;
    b = color&0xC0;
    g = color&0xE000;
    r = color&0xE00000;
    *(view_screen.vram_start + (view_screen.width)*y+x) = 
        (unsigned char)((b>>6)|(g>>11)|(r>>16));
    return  0;
}

static int screen_out_pixel15(int x, int y, view_color_t color)
{
    uint32_t  r, g, b;
    b = color&0xF8;
    g = color&0xF800;
    r = color&0xF80000;
    *((short int*)((view_screen.vram_start) + 2*((view_screen.width)*y+x))) = 
        (short int)((b>>3)|(g>>6)|(r>>9));
    return  0;
}

static int screen_out_pixel16(int x, int y, view_color_t color)
{
    uint32_t  r, g, b;
    b = color&0xF8;
    g = color&0xFC00;
    r = color&0xF80000;
    *((short*)((view_screen.vram_start) + 2*((view_screen.width)*y+x))) = 
        (short)((b>>3)|(g>>5)|(r>>8));
    return  0;
}

static int screen_out_pixel24(int x, int y, view_color_t color)
{
    *((view_screen.vram_start) + 3*((view_screen.width)*y+x) + 0) = color&0xFF;
    *((view_screen.vram_start) + 3*((view_screen.width)*y+x) + 1) = (color&0xFF00) >> 8;
    *((view_screen.vram_start) + 3*((view_screen.width)*y+x) + 2) = (color&0xFF0000) >> 16;
    return  0;
}

static int screen_out_pixel32(int x, int y, view_color_t color)
{
    *((unsigned int*)((view_screen.vram_start) + 4*((view_screen.width)*y+x))) = (unsigned int)color;
    return  0;
}

void view_screen_write_pixel(int x, int y, view_color_t color) 
{
    view_screen.out_pixel(x, y, color);
}

int view_screen_init()
{
    if (view_screen_open(&view_screen) < 0) {
        return -1;
    }
    switch (view_screen.bpp) 
    {
    case 8:
        view_screen.out_pixel = screen_out_pixel8;
        break;
    case 15:
        view_screen.out_pixel = screen_out_pixel15;
        break;
    case 16:
        view_screen.out_pixel = screen_out_pixel16;
        break;
    case 24:
        view_screen.out_pixel = screen_out_pixel24;
        break;
    case 32:
        view_screen.out_pixel = screen_out_pixel32;
        break;
    default:
        keprint("xbrower: unknown screen bpp\n");
        view_screen_close(&view_screen);
        return -1;
    }
    return 0;
}

int view_screen_exit()
{
    view_screen_close(&view_screen);
    return 0;
}
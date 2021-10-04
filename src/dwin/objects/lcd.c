#include <dwin/objects.h>
#include <dwin/hal.h>
#include <dwin/dwin.h>

static int lcd_out_pixel8(struct dwin_lcd *lcd, int x, int y, unsigned int color)
{
    unsigned int r, g, b;
    b = color&0xC0;
    g = color&0xE000;
    r = color&0xE00000;
    *(lcd->vram_start + (lcd->width)*y+x) = (unsigned char)((b>>6)|(g>>11)|(r>>16));
    return  0;
}

static int lcd_out_pixel15(struct dwin_lcd *lcd, int x, int y, unsigned int color)
{
    unsigned int r, g, b;
    b = color&0xF8;
    g = color&0xF800;
    r = color&0xF80000;
    *((short int*)((lcd->vram_start) + 2*((lcd->width)*y+x))) = (short int)((b>>3)|(g>>6)|(r>>9));
    return  0;
}

static int lcd_out_pixel16(struct dwin_lcd *lcd, int x, int y, unsigned int color)
{
    unsigned int r, g, b;
    b = color&0xF8;
    g = color&0xFC00;
    r = color&0xF80000;
    *((short*)((lcd->vram_start) + 2*((lcd->width)*y+x))) = (short)((b>>3)|(g>>5)|(r>>8));
    return  0;
}

static int lcd_out_pixel24(struct dwin_lcd *lcd, int x, int y, unsigned int color)
{
    *((lcd->vram_start) + 3*((lcd->width)*y+x) + 0) = color&0xFF;
    *((lcd->vram_start) + 3*((lcd->width)*y+x) + 1) = (color&0xFF00) >> 8;
    *((lcd->vram_start) + 3*((lcd->width)*y+x) + 2) = (color&0xFF0000) >> 16;
    return  0;
}

static int lcd_out_pixel32(struct dwin_lcd *lcd, int x, int y, unsigned int color)
{
    *((unsigned int*)((lcd->vram_start) + 4*((lcd->width)*y+x))) = (unsigned int)color;
    return  0;
}

void dwin_lcd_init(struct dwin_lcd *lcd)
{
    lcd->handle = -1;
}

int dwin_lcd_map(struct dwin_lcd *lcd)
{
    struct dwin_hal_lcd *hal = (struct dwin_hal_lcd *)lcd;
    
    if (hal->map == NULL || hal->map(lcd) < 0)
    {
        return -1;
    }

    switch (lcd->bpp)
    {
    case 8:
        lcd->out_pixel = lcd_out_pixel8;
        break;
    case 15:
        lcd->out_pixel = lcd_out_pixel15;
        break;
    case 16:
        lcd->out_pixel = lcd_out_pixel16;
        break;
    case 24:
        lcd->out_pixel = lcd_out_pixel24;
        break;
    case 32:
        lcd->out_pixel = lcd_out_pixel32;
        break;
    default:
        dwin_log("unknown lcd bpp!\n");
        if (hal->unmap != NULL)
        {
            hal->unmap(lcd);
        }
        return -1;
    }
    
    return 0;
}

int dwin_lcd_unmap(struct dwin_lcd *lcd)
{
    struct dwin_hal_lcd *hal = (struct dwin_hal_lcd *)lcd;
    if (hal->unmap == NULL || hal->unmap(lcd) < 0)
    {
        return -1;
    }
    lcd->out_pixel = NULL;
    return 0;
}

void dwin_lcd_draw_rect(struct dwin_lcd *lcd, int x, int y, int w, int h, unsigned int color)
{
    /* TODO: add bound check */
    
    int i, j;
    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            lcd->out_pixel(lcd, x + i, y + j, color);
        }
    }
}


void dwin_lcd_demo(struct dwin_lcd *lcd)
{
    dwin_lcd_draw_rect(lcd, 0, 0, 
        lcd->width,
        lcd->height,
        0xFFFF0000);
    
    dwin_lcd_draw_rect(lcd, 100, 100, 
        lcd->width / 2,
        lcd->height / 2,
        0xFF00FF00);

    dwin_lcd_draw_rect(lcd, 200, 200, 
        lcd->width / 4,
        lcd->height / 4,
        0xFF0000FF);
}

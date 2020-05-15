#include  <string.h>
#include  <stdio.h>

#include  <learninggui.h>

#include  <sys/res.h>
#include  <sys/ioctl.h>
#include  <sys/vmm.h>

#include "guisrv.h"

#ifdef  _LG_SCREEN_


#ifndef   GUI_FRAME_BUFFER_DEVICE_NAME
#define   GUI_FRAME_BUFFER_DEVICE_NAME        "video"
#endif

/*
 * Color conversion interfaces
 *
 * User maybe rewrite these interfaces according to your lcd type 
 */

/* R3G3B2 color helper */
static  SCREEN_COLOR  GUI_TO_R3G3B2(GUI_COLOR gui_color)
{
    SCREEN_COLOR  r, g, b;

    b = gui_color&0xC0;
    g = gui_color&0xE000;
    r = gui_color&0xE00000;

    return  (b>>6)|(g>>11)|(r>>16);
}

static  GUI_COLOR  R3G3B2_TO_GUI(SCREEN_COLOR screen_color) 
{
    GUI_COLOR  r, g, b;

    b = screen_color&0x03;
    g = screen_color&0x1C;
    r = screen_color&0xE0;

    return  (b<<6)|(g<<11)|(r<<16);
}

/* R5G5B5 color helper */
static  SCREEN_COLOR  GUI_TO_R5G5B5(GUI_COLOR gui_color)
{
    SCREEN_COLOR  r, g, b;

    b = gui_color&0xF8;
    g = gui_color&0xF800;
    r = gui_color&0xF80000;

    return  (b>>3)|(g>>6)|(r>>9);
}

static  GUI_COLOR  R5G5B5_TO_GUI(SCREEN_COLOR screen_color) 
{
    GUI_COLOR  r, g, b;

    b = screen_color&0x1F;
    g = screen_color&0x03E0;
    r = screen_color&0x7C00;

    return  (b<<3)|(g<<6)|(r<<9);
}


/* R5G6B5 color helper */
static  SCREEN_COLOR  GUI_TO_R5G6B5(GUI_COLOR gui_color)
{
    SCREEN_COLOR  r, g, b;

    b = gui_color&0xF8;
    g = gui_color&0xFC00;
    r = gui_color&0xF80000;

    return  (b>>3)|(g>>5)|(r>>8);
}

static  GUI_COLOR  R5G6B5_TO_GUI(SCREEN_COLOR screen_color) 
{
    GUI_COLOR  r, g, b;

    b = screen_color&0x1F;
    g = screen_color&0x07E0;
    r = screen_color&0xF800;

    return  (b<<3)|(g<<5)|(r<<8);
}


/* R8G8B8 color helper */
static  SCREEN_COLOR  GUI_TO_R8G8B8(GUI_COLOR gui_color)
{
    return  gui_color;
}

static  GUI_COLOR  R8G8B8_TO_GUI(SCREEN_COLOR screen_color) 
{
    return  screen_color;
}

static  int             video_res           = -1;

static  unsigned int    video_ram_size  = 0;
static  unsigned char  *video_ram_start = NULL;
static  unsigned int    bits_per_pixel  = 0;
static  unsigned int    bytes_per_pixel = 0;

static  unsigned int    screen_width    = 0;
static  unsigned int    screen_height   = 0;

static  video_info_t    video_info;

static  int  video_detect_var(GUI_SCREEN *screen)
{
    
    int    ret = 0;

    video_res = res_open( GUI_FRAME_BUFFER_DEVICE_NAME, RES_DEV, 0 );
    if ( video_res < 0 ) 
        return  -1;

    ret = res_ioctl(video_res, VIDEOIO_GETINFO, (unsigned long) &video_info);
    if ( ret < 0 ) 
        return  -1;

    screen->width     = video_info.x_resolution;
    screen->height    = video_info.y_resolution;
    
    bits_per_pixel   = video_info.bits_per_pixel;
    bytes_per_pixel  = (video_info.bits_per_pixel+7)/8;
 
    screen_width     = screen->width;
    screen_height    = screen->height;

    printf("video info: w:%d h:%d bpp:%d \n", screen->width, screen->height, video_info.bits_per_pixel);

    switch (video_info.bits_per_pixel) 
    {
    case 8:
        screen->gui_to_screen_color = GUI_TO_R3G3B2;
        screen->screen_to_gui_color = R3G3B2_TO_GUI;
        break;

    case 15:
        screen->gui_to_screen_color = GUI_TO_R5G5B5;
        screen->screen_to_gui_color = R5G5B5_TO_GUI;
        break;

    case 16:
        screen->gui_to_screen_color = GUI_TO_R5G6B5;
        screen->screen_to_gui_color = R5G6B5_TO_GUI; 
        break;

    case 24:
        screen->gui_to_screen_color = GUI_TO_R8G8B8;
        screen->screen_to_gui_color = R8G8B8_TO_GUI;
        break;

    case 32:
        screen->gui_to_screen_color = GUI_TO_R8G8B8;
        screen->screen_to_gui_color = R8G8B8_TO_GUI;
        break;

    default:
        break;
    }


    res_close(video_res);
    video_res = -1;

    return  1;
}

/*
 * Lcd driver interfaces
 */

static  int  lcd_open(void)
{
    /* User must write the right code */

    /* 
     * Your lcd device name: /dev/fb0. 
     * Yes or no ? 
     */
    video_res = res_open( GUI_FRAME_BUFFER_DEVICE_NAME, RES_DEV, 0 );
    if ( video_res < 0 ) 
        return  -1;

    video_ram_size = video_info.bytes_per_scan_line * video_info.y_resolution;
    video_ram_start = res_mmap(video_res, video_ram_size, 0);
    if (video_ram_start == NULL) {
        printf("%s: video mapped failed!\n", SRV_NAME);
        return -1;
    }

    printf("%s: mapped addr %x\n", SRV_NAME, video_ram_start);
    //memset(video_ram_start, 0xff, video_ram_size);

    return  1;
}

static  int  lcd_close(void)
{
    munmap(video_ram_start, video_ram_size);
    res_close(video_res);
    video_res = -1;

    return  1;
}

/* Optimize interface */
static  int  lcd_output_sequence_start(void)
{
    /* For SPI lcd etc, you can pull down CS singnal */

    /* In most case, return 1 */

    return  1;
}
    
static  int  lcd_output_pixel(int x, int y, SCREEN_COLOR  color)
{
    /* User must write the right code */
	int  offset = 0;


    if ( x > (screen_width-1) )
        return  -1;

    if ( y > (screen_height-1) )
        return  -1;


    /* Output pixel */
    switch( (bits_per_pixel) )
    {
        case  8:
            offset = (screen_width)*y+x;
            if ( offset >= ((screen_width)*(screen_height)-1))
                return  -1;

    	    *(video_ram_start+offset) = (unsigned char)color;
            break;

        case  15:
            offset = 2*((screen_width)*y+x);
            if ( offset >= ((screen_width)*(screen_height)*2-2))
                return  -1;

    	    *((short int*)((video_ram_start) + offset)) = (short int)color;
            break;

        case  16:
            offset = 2*((screen_width)*y+x);
            if ( offset >= ((screen_width)*(screen_height)*2-2))
                return  -1;

    	    *((short int*)((video_ram_start) + offset)) = (short int)color;
            break;

        case  24:
            offset = 3*((screen_width)*y+x);
            if ( offset >= ((screen_width)*(screen_height)*3-3))
                return  -1;

    	    *((video_ram_start) + offset + 0) = color&0xFF;
    	    *((video_ram_start) + offset + 1) = (color&0xFF00) >> 8;
    	    *((video_ram_start) + offset + 2) = (color&0xFF0000) >> 16;
            break;

        case  32:
            offset = 4*((screen_width)*y+x);
            if ( offset >= ((screen_width)*(screen_height)*4-4))
                return  -1;

    	    *((int*)((video_ram_start) + offset)) = (int)color;
            break;

        default:
            break;

    }
    return  1;
}

/* Hardware accelerbrate interface */
static  int  lcd_output_hline(int left, int right, int top, SCREEN_COLOR  color)
{
     return  0;
}

/* Hardware accelerbrate interface */
static  int  lcd_output_vline(int left, int top, int bottom, SCREEN_COLOR  color)
{
    return  0;
}

/* Hardware accelerbrate interface */
static  int  lcd_output_rect_fill(int left, int top, int right, int bottom, SCREEN_COLOR  color)
{
    return  0;
}

/* Optimize interface */
static  int  lcd_output_sequence_end(void)
{
    /* For SPI lcd etc, you can pull up CS singnal */

    /* In most case, return 1 */

    return  1;
}


/* Optimize interface */
static  int  lcd_input_sequence_start(void)
{
    /* For SPI lcd etc, you can pull down CS singnal */
 
    return  1;
}

static  int  lcd_input_pixel(int x, int y, SCREEN_COLOR *color)
{
    /* Maybe have code or maybe have not code */


    return  1;
}

/* Optimize interface */
static  int  lcd_input_sequence_end(void)
{
    /* For SPI lcd etc, you can pull up CS singnal */

    /* In most case, return 1 */

    return  1;
}

    
#ifndef  _LG_WINDOW_
static  int  lcd_clear(SCREEN_COLOR  color)
{
    /* Maybe have code or maybe have not code */ 

    return  1;
}
#endif


/*
 * App interfaces
 */
static  int  lcd_control(void *p1, void *p2)
{
    return  1;
}

static  int  lcd_on(void)
{
    return  1;
}

static  int  lcd_off(void)
{
    return  1;
}

static  int  lcd_reinit(void)
{
    return  1;
}


/*
 * Register API
 */

int  register_screen(void)
{
    GUI_SCREEN  screen = {0};
    int         ret     = 0;


    memset(&screen, 0, sizeof(screen));
    ret = video_detect_var(&screen);
    if ( ret < 1 )
        return  -1;
    
    screen.is_hline_accelerate      = 0;
    screen.is_vline_accelerate      = 0;
    screen.is_rect_fill_accelerate  = 0;

    screen.open                     = lcd_open;
    screen.close                    = lcd_close;

    screen.output_sequence_start    = lcd_output_sequence_start;
    screen.output_pixel             = lcd_output_pixel;
    screen.output_hline             = lcd_output_hline;
    screen.output_vline             = lcd_output_vline;
    screen.output_rect_fill         = lcd_output_rect_fill;
    screen.output_sequence_end      = lcd_output_sequence_end;

    screen.input_sequence_start     = lcd_input_sequence_start;
    screen.input_pixel              = lcd_input_pixel;
    screen.input_sequence_end       = lcd_input_sequence_end;


    screen.control                  = lcd_control;
    screen.on                       = lcd_on;
    screen.off                      = lcd_off;
    screen.reinit                   = lcd_reinit;
    
    #ifndef  _LG_WINDOW_
    screen.clear                    = lcd_clear;
    #endif
    
    #ifdef  _LG_NEED_REFRESH_SCREEN_
    screen.is_refresh               = 0;
    screen.refresh_interval         = 10;
    screen.refresh                  = NULL;
    #endif
	 
    #ifdef  _LG_PALETTE_ROUTINE_
    screen.is_palette               = 0;
    screen.palette                  = NULL;
    screen.palette_set              = NULL;
    screen.palette_get              = NULL;
    #endif

    in_driver_register(DRIVER_SCREEN, &screen);

    return  1;
}

#endif  /* _LG_SCREEN_ */

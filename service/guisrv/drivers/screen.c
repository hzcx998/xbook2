#include <drivers/screen.h>
#include <sys/res.h>
#include <sys/ioctl.h>
#include <sys/vmm.h>
#include <string.h>
#include <stdio.h>
#include <guisrv.h>

#include <layer/color.h>

#define DEBUG_LOCAL 1

#ifndef   GUI_SCREEN_DEVICE_NAME
#define   GUI_SCREEN_DEVICE_NAME        "video"
#endif

/*
 * Color conversion interfaces
 *
 * User maybe rewrite these interfaces according to your screen type 
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

static  video_info_t    video_info;
static  int             video_res           = -1;
static  unsigned int    video_ram_size  = 0;
static  unsigned char  *video_ram_start = NULL;
static  unsigned int    bits_per_pixel  = 0;
static  unsigned int    bytes_per_pixel = 0;
static  unsigned int    screen_width    = 0;
static  unsigned int    screen_height   = 0;

static  int  screen_detect_var(drv_screen_t *screen)
{
    int ret = 0;

    video_res = res_open( GUI_SCREEN_DEVICE_NAME, RES_DEV, 0 );
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

    //printf("video info: w:%d h:%d bpp:%d \n", screen->width, screen->height, video_info.bits_per_pixel);

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
    return  0;
}

static int screen_open(void)
{
    video_res = res_open( GUI_SCREEN_DEVICE_NAME, RES_DEV, 0 );
    if ( video_res < 0 ) 
        return  -1;

    video_ram_size = video_info.bytes_per_scan_line * video_info.y_resolution;
    video_ram_start = res_mmap(video_res, video_ram_size, 0);
    if (video_ram_start == NULL) {
        printf("%s: video mapped failed!\n", SRV_NAME);
        return -1;
    }
#if DEBUG_LOCAL == 1
    printf("%s: mapped addr %x\n", SRV_NAME, video_ram_start);
#endif
    return  0;
}

static int screen_close(void)
{
    munmap(video_ram_start, video_ram_size);
    res_close(video_res);
    video_res = -1;
    return  1;
}


static  int  screen_output_pixel(int x, int y, SCREEN_COLOR  color)
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
    return  0;
}

static  int  screen_input_pixel(int x, int y, SCREEN_COLOR *color)
{
    /* Maybe have code or maybe have not code */
    int           offset    = 0;
    SCREEN_COLOR  tmp_color = 0;


    if ( color == NULL )
        return  -1;


    if ( x > (screen_width-1) )
        return  -1;

    if ( y > (screen_height-1) )
        return  -1;


    /* Get pixel */
    switch( (bits_per_pixel) )
    {
        case  8:
            offset = (screen_width)*y+x;
            if ( offset >= ((screen_width)*(screen_height)-1))
                return  -1;

    	    *color = *(video_ram_start+offset);
            break;

        case  15:
            offset = 2*((screen_width)*y+x);
            if ( offset >= ((screen_width)*(screen_height)*2-2))
                return  -1;

    	    *color = *((short int*)(video_ram_start + offset));
            break;

        case  16:
            offset = 2*((screen_width)*y+x);
            if ( offset >= ((screen_width)*(screen_height)*2-2))
                return  -1;

    	    *color = *((short int*)(video_ram_start + offset));
            break;

        case  24:
            offset = 3*((screen_width)*y+x);
            if ( offset >= ((screen_width)*(screen_height)*3-3))
                return  -1;

    	    tmp_color  = *(video_ram_start + offset + 0)&0xFF;
    	    tmp_color |= ((*(video_ram_start + offset + 1))&0xFF00) >> 8;
    	    tmp_color |= ((*(video_ram_start + offset + 2))&0xFF0000) >> 16;
            *color     = tmp_color;
            break;

        case  32:
            offset = 4*((screen_width)*y+x);
            if ( offset >= ((screen_width)*(screen_height)*4-4))
                return  -1;

    	    *color = *((int*)(video_ram_start + offset));
            break;

        default:
            break;

    }
    return  0;
}


/* Hardware accelerbrate interface */
static  int  screen_output_hline(int left, int right, int top, SCREEN_COLOR  color)
{
    SCREEN_COLOR  a_color;
    SCREEN_COLOR  b_color;
    SCREEN_COLOR  c_color;

    int  offset = 0;
    int  i      = 0;

    if ( left > (screen_width-1) )
        return  -1;
    if ( right > (screen_width-1) )
        return  -1;

    if ( top > (screen_height-1) )
        return  -1;

    switch( (bits_per_pixel) )
    {
        case  8:
            offset = (screen_width)*top+left;
            if ( offset >= ((screen_width)*(screen_height)-1))
                return  -1;

            for ( i = 0; i <= right - left; i++ )
    	        *(video_ram_start+offset+i) = (unsigned char)color;

            break;

        case  15:
            offset = 2*((screen_width)*top+left);
            if ( offset >= ((screen_width)*(screen_height)*2-2))
                return  -1;

            for ( i = 0; i <= right - left; i++ )
    	        *((short int*)((video_ram_start) + offset + 2*i)) = (short int)color;

            break;

        case  16:
            offset = 2*((screen_width)*top+left);
            if ( offset >= ((screen_width)*(screen_height)*2-2))
                return  -1;

            for ( i = 0; i <= right - left; i++ )
    	        *((short int*)((video_ram_start) + offset + 2*i)) = (short int)color;

            break;

        case  24:
            offset = 3*((screen_width)*top+left);
            if ( offset >= ((screen_width)*(screen_height)*3-3))
                return  -1;

            a_color = color&0xFF;
    	    b_color = (color&0xFF00) >> 8;
    	    c_color = (color&0xFF0000) >> 16;
            for ( i = 0; i <= right - left; i++ )
            {
    	        *((video_ram_start) + offset + 3*i + 0) = a_color;
    	        *((video_ram_start) + offset + 3*i + 1) = b_color;
    	        *((video_ram_start) + offset + 3*i + 2) = c_color;
            }
            break;

        case  32:
            offset = 4*((screen_width)*top+left);
            if ( offset >= ((screen_width)*(screen_height)*4-4))
                return  -1;

            for ( i = 0; i <= right - left; i++ )
    	        *((int*)((video_ram_start) + offset + 4*i)) = (int)color;

            break;

        default:
            break;

    }

    return  0;
}

/* Hardware accelerbrate interface */
static  int  screen_output_vline(int left, int top, int bottom, SCREEN_COLOR  color)
{
    SCREEN_COLOR  a_color;
    SCREEN_COLOR  b_color;
    SCREEN_COLOR  c_color;

    int  offset = 0;
    int  i      = 0;

    if ( left > (screen_width-1) )
        return  -1;
    if ( top > (screen_height-1) )
        return  -1;
    if ( bottom > (screen_height-1) )
        return  -1;


    switch( (bits_per_pixel) )
    {
        case  8:
            for ( i = 0; i <= bottom - top; i++ )
            {
                offset = (screen_width)*(top+i)+left;
                if ( offset >= ((screen_width)*(screen_height)-1))
                    return  -1;

    	        *(video_ram_start+offset) = (unsigned char)color;
            }
            break;

        case  15:
            for ( i = 0; i <= bottom - top; i++ )
            {
                offset = 2*((screen_width)*(top+i)+left);
                if ( offset >= ((screen_width)*(screen_height)*2-2))
                    return  -1;

    	        *((short int*)((video_ram_start) + offset)) = (short int)color;
            }
            break;

        case  16:
            for ( i = 0; i <= bottom - top; i++ )
            {
                offset = 2*((screen_width)*(top+i)+left);
                if ( offset >= ((screen_width)*(screen_height)*2-2))
                    return  -1;

    	        *((short int*)((video_ram_start) + offset)) = (short int)color;
            }
            break;

        case  24:
            a_color = color&0xFF;
    	    b_color = (color&0xFF00) >> 8;
    	    c_color = (color&0xFF0000) >> 16;

            for ( i = 0; i <= bottom - top; i++ )
            {
                offset = 3*((screen_width)*(top+i)+ left);
                if ( offset >= ((screen_width)*(screen_height)*3-3))
                    return  -1;

       	        *((video_ram_start) + offset + 0) = a_color;
    	        *((video_ram_start) + offset + 1) = b_color;
    	        *((video_ram_start) + offset + 2) = c_color;
            }
            break;

        case  32:
            for ( i = 0; i <= bottom - top; i++ )
            {
                offset = 4*((screen_width)*(top+i)+left);
                if ( offset >= ((screen_width)*(screen_height)*4-4))
                    return  -1;

    	        *((int*)((video_ram_start) + offset)) = (int)color;
            }
            break;

        default:
            break;

    }

    return  0;
}


/* Hardware accelerbrate interface */
static  int  screen_output_rect_fill(int left, int top, int right, int bottom, SCREEN_COLOR  color)
{
    SCREEN_COLOR  a_color;
    SCREEN_COLOR  b_color;
    SCREEN_COLOR  c_color;

    int  offset = 0;
    int  i      = 0;
    int  j      = 0;

    if ( left > (screen_width-1) )
        return  -1;
    if ( right > (screen_width-1) )
        return  -1;

    if ( top > (screen_height-1) )
        return  -1;



    switch( (bits_per_pixel) )
    {
        case  8:
            for ( j = 0; j <= bottom - top; j++ )
            {
                offset = (screen_width)*(top+j)+left;
                if ( offset >= ((screen_width)*(screen_height)-1))
                    return  -1;

                for ( i = 0; i <= right - left; i++ )
    	            *(video_ram_start+offset+i) = (unsigned char)color;
            }
            break;

        case  15:
            for ( j = 0; j <= bottom - top; j++ )
            {
                offset = 2*((screen_width)*(top+j)+left);
                if ( offset >= ((screen_width)*(screen_height)*2-2))
                    return  -1;

                for ( i = 0; i <= right - left; i++ )
    	            *((short int*)((video_ram_start) + offset + 2*i)) = (short int)color;
            }
            break;

        case  16:
            for ( j = 0; j <= bottom - top; j++ )
            {
                offset = 2*((screen_width)*(top+j)+left);
                if ( offset >= ((screen_width)*(screen_height)*2-2))
                    return  -1;

                for ( i = 0; i <= right - left; i++ )
    	            *((short int*)((video_ram_start) + offset + 2*i)) = (short int)color;
            }
            break;

        case  24:
            a_color = color&0xFF;
    	    b_color = (color&0xFF00) >> 8;
    	    c_color = (color&0xFF0000) >> 16;
            for ( j = 0; j <= bottom - top; j++ )
            {
                offset = 3*((screen_width)*(top+j)+left);
                if ( offset >= ((screen_width)*(screen_height)*3-3))
                    return  -1;

                for ( i = 0; i <= right - left; i++ )
                {
    	            *((video_ram_start) + offset + 3*i + 0) = a_color;
    	            *((video_ram_start) + offset + 3*i + 1) = b_color;
    	            *((video_ram_start) + offset + 3*i + 2) = c_color;
                }
            }
            break;

        case  32:
            for ( j = 0; j <= bottom - top; j++ )
            {
                offset = 4*((screen_width)*(top+j)+left);
                if ( offset >= ((screen_width)*(screen_height)*4-4))
                    return  -1;

                for ( i = 0; i <= right - left; i++ )
    	            *((int*)((video_ram_start) + offset + 4*i)) = (int)color;
            }
            break;

        default:
            break;

    }
    return  1;
}



drv_screen_t drv_screen = {0};

int init_screen_driver()
{
    int ret = 0;

    memset(&drv_screen, 0, sizeof(drv_screen));
    ret = screen_detect_var(&drv_screen);
    if (ret < 0)
        return -1;
    
    drv_screen.open = screen_open;
    drv_screen.close = screen_close;

    drv_screen.output_pixel = screen_output_pixel;
    drv_screen.input_pixel  = screen_input_pixel;

    drv_screen.output_hline = screen_output_hline;
    drv_screen.output_vline = screen_output_vline;
    drv_screen.output_hline = screen_output_hline;
    drv_screen.output_rect_fill = screen_output_rect_fill;
    
    return 0;
}

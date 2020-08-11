#include "test.h"

static  video_info_t    video_info;

/* R5G6B5 color helper */
static  uint32_t  GUI_TO_R5G6B5(uint32_t gui_color)
{
    uint32_t  r, g, b;

    b = gui_color&0xF8;
    g = gui_color&0xFC00;
    r = gui_color&0xF80000;

    return  (b>>3)|(g>>5)|(r>>8);
}

static  unsigned int    video_ram_size  = 0;
static  unsigned char  *video_ram_start = NULL;

int  screen_output_pixel(int x, int y, uint32_t  color)
{
    /* User must write the right code */
	int  offset = 0;

    if ( x > (video_info.x_resolution-1) )
        return  -1;

    if ( y > (video_info.y_resolution-1) )
        return  -1;

    color = GUI_TO_R5G6B5(color);

    /* Output pixel */
    switch( (video_info.bits_per_pixel) )
    {
        case  8:
            offset = (video_info.x_resolution)*y+x;
            if ( offset >= ((video_info.x_resolution)*(video_info.y_resolution)-1))
                return  -1;

    	    *(video_ram_start+offset) = (unsigned char)color;
            break;

        case  15:
            offset = 2*((video_info.x_resolution)*y+x);
            if ( offset >= ((video_info.x_resolution)*(video_info.y_resolution)*2-2))
                return  -1;

    	    *((short int*)((video_ram_start) + offset)) = (short int)color;
            break;

        case  16:
            offset = 2*((video_info.x_resolution)*y+x);
            if ( offset >= ((video_info.x_resolution)*(video_info.y_resolution)*2-2))
                return  -1;

    	    *((short int*)((video_ram_start) + offset)) = (short int)color;
            break;

        case  24:
            offset = 3*((video_info.x_resolution)*y+x);
            if ( offset >= ((video_info.x_resolution)*(video_info.y_resolution)*3-3))
                return  -1;

    	    *((video_ram_start) + offset + 0) = color&0xFF;
    	    *((video_ram_start) + offset + 1) = (color&0xFF00) >> 8;
    	    *((video_ram_start) + offset + 2) = (color&0xFF0000) >> 16;
            break;

        case  32:
            offset = 4*((video_info.x_resolution)*y+x);
            if ( offset >= ((video_info.x_resolution)*(video_info.y_resolution)*4-4))
                return  -1;

    	    *((int*)((video_ram_start) + offset)) = (int)color;
            break;

        default:
            break;

    }
    return  0;
}
int video_res;

int init_libvideo()
{
    /* 映射显存 */
    int video_res = res_open("video", RES_DEV, 0 );
    if ( video_res < 0 ) 
        return  -1;

    int ret = res_ioctl(video_res, VIDEOIO_GETINFO, (unsigned long) &video_info);
    if ( ret < 0 ) {
        res_close(video_res);
        return  -1;
    }
        
    video_ram_size = video_info.bytes_per_scan_line * video_info.y_resolution;
    video_ram_start = res_mmap(video_res, video_ram_size, 0);
    if (video_ram_start == NULL) {
        printf("test: video mapped failed!\n");
        res_close(video_res);
        return -1;
    }
    return 0;
}

int video_test(int argc, char *argv[])
{
    if (argc < 3) {
        printf("please input file type 'jpg' or 'png' + file.\n");
        return -1;
    }

    int pic_type = -1;
    if (!strcmp(argv[1], "jpg")) {
        pic_type = 0;
    } else if (!strcmp(argv[1], "png")) {
        pic_type = 1;
    } else {
        printf("please input file type error!\n");
        return - 1;
    }

    if (init_libvideo() < 0)
        return -1;


    if (pic_type == 0) {
        if (jpg_display(argv[2]) < 0)
            printf("display failed!\n");      
    } else {
        if (test_png_main(argc, argv) < 0)
            printf("display failed!\n");

    }
    munmap(video_ram_start, video_ram_size);
    res_close(video_res); 
    return 0;   
}

#include "desktop.h"
#include <graph.h>
#include <stdio.h>
#include <malloc.h>

#include <jpeglib.h>
#include <jmorecfg.h>
#include <jconfig.h>

uint32_t desktop_width, desktop_height;
uint32_t gui_bpp;
int desktop_layer;

int jpg_display(char * path)// **指定图片的路径就可以调用这个jpg的解析。**
{
    /*
    １.　分配并初始化一个jpeg解压对象
    */
    struct jpeg_decompress_struct dinfo; //定义了一个jpeg的解压对象

    struct jpeg_error_mgr jerr; //定义一个错误变量
    dinfo.err = jpeg_std_error(&jerr);

    jpeg_create_decompress(&dinfo); //初始化这个解压对象

    /*
    2. 指定要解压缩的图像文件
    */
    FILE *infile;
    infile = fopen(path, "r");
    if (infile == NULL)
    {
        printf("fopen error!");
        return -1;
    }

    jpeg_stdio_src(&dinfo, infile); //为解压对象dinfo指定要解压的文件

    /*
    3. 调用jpeg_read_header()获取图像信息

    */
    if (jpeg_read_header(&dinfo, TRUE) != JPEG_HEADER_OK) {
        fclose(infile);
        return -1;
    } 
    /*
    4. 设置jpeg解压缩对象dinfo的一些参数，可采用默认参数
    */
    /*
    5.调用jpeg_start_decompress()启动解压过程
    jpeg_start_decompress(&dinfo); 
    调用jpeg_start_decompress(&dinfo);函数之后，JPEG解压对象dinfo中下面这几个成员变量
    将会比较有用：
    output_width:  图像的输出宽度
    output_height：　图像的输出高度
    output_component：　每个像素的分量数，也即字节数，3/4字节
    在调用jpeg_start_decompress(&dinfo); 之后往往需要为解压后的扫描线上的所有像素点分配存储空间，
    output_width * output_components(一行的字节数，output_height行)

    */
    jpeg_start_decompress(&dinfo); 
    
    unsigned char *buffer = malloc(dinfo.output_width * dinfo.output_components);
    if (buffer == NULL) {
        fclose(infile);
        jpeg_destroy_decompress(&dinfo);
        return -1;
    }
    printf("width %d, height %d, components %d.\n", dinfo.output_width, dinfo.output_height, dinfo.output_components);
    
    /*
    ６.　读取一行或者多行扫描线数据并处理，通常的代码是这样的：

    //output_scanline表示扫描的总行数
    //output_height表示图像的总行数
    while (dinfo.output_scanline < dinfo.output_height)
    {
    jpeg_read_scanlines(&dinfo, &buffer, 1);

    //deal with scanlines . RGB/ARGB
    }
    */
    
    uint32_t *pixbuf = malloc(dinfo.output_width * 4 * dinfo.output_height);
    if (pixbuf == NULL) {
        free(buffer);
        fclose(infile);
        jpeg_destroy_decompress(&dinfo);
        return -1;
    }
    uint32_t pixidx = 0;
    //output_scanline表示扫描的总行数
    //output_height表示图像的总行数
    while (dinfo.output_scanline < dinfo.output_height)
    {
        unsigned char *buf1[1];
        buf1[0] = buffer;

        jpeg_read_scanlines(&dinfo, buf1, 1); //dinfo.output_scanline + 1

        //deal with scanlines . RGB/ARGB
        int x; //一行的像素点数量

        unsigned char *p = buf1[0];
        for (x = 0; x < dinfo.output_width; x++)
        {
            unsigned char r, g, b, a = 255;
            uint32_t color;
            if (dinfo.output_components == 3)
            {
                r = *p++;
                g = *p++;
                b = *p++;
            } else if (dinfo.output_components == 4)
            {
                a = *p++;
                r = *p++;
                g = *p++;
                b = *p++;
            }
            color = (a << 24) | (r << 16) | (g << 8) |(b) ;
            /* 放到缓冲区 */
            pixbuf[pixidx] = color;
            pixidx++;
            //g_layer_outp(desktop_layer, x, dinfo.output_scanline - 1 ,  color);
        }
    }
    g_layer_pixmap(desktop_layer, 0, 0, dinfo.output_width, dinfo.output_height, pixbuf, 4);
    g_layer_refresh(desktop_layer, 0, 0, desktop_width, desktop_height);
    
    /*
    7. 调用　jpeg_finish_decompress()完成解压过程
    */
    jpeg_finish_decompress(&dinfo);

    /*
    ８.调用jpeg_destroy_decompress()释放jpeg解压对象dinfo
    jpeg_destroy_decompress(&dinfo);
    */
    jpeg_destroy_decompress(&dinfo);
    
    free(buffer);
    free(pixbuf);
    return 0;
}

#define BG_PIC_PATH "/res/pic1.jpg"

int init_desktop()
{
    
    g_gui_info(&desktop_width, &desktop_height, &gui_bpp);
    if (!desktop_width || !desktop_height)
        return -1;

    /* 桌面 */
    int layer = g_layer_new(0, 0, desktop_width, desktop_height);
    if (layer < 0) {
        printf("[desktop]: new desktop layer failed!\n");
        return -1;
    }
    desktop_layer = layer;
    g_layer_z(layer, 0);    /* 0 is desktop */
    /* draw desktop */
    g_layer_rect_fill(layer, 0, 0, desktop_width, desktop_height, GC_GRAY);
    /* refresh desktop */
    g_layer_refresh(layer, 0, 0, desktop_width, desktop_height);
    
    g_layer_set_focus(layer);   /* set as focus layer */

    /* 加载背景图片 */
    if (jpg_display(BG_PIC_PATH) < 0) {
        printf("show background pic %s failed!\n", BG_PIC_PATH);
    }

    return 0;
}



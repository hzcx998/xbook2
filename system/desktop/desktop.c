#include "desktop.h"
#include <gapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <malloc.h>

#include <jpeglib.h>
#include <jmorecfg.h>
#include <jconfig.h>

#include <png.h>

int desktop_layer;
g_bitmap_t *desktop_bitmap;

// **指定图片的路径就可以调用这个jpg的解析。**
int jpg_load_bitmap(char * path, uint32_t *bitmap, uint32_t width, uint32_t height)
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
    
    uint32_t *pixbuf = bitmap;
    
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
        for (x = 0; x < dinfo.output_width && x < width && dinfo.output_scanline < height; x++)
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
        }
    }
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
    return 0;
}

#define PNG_BYTES_TO_CHECK	8
#define HAVE_ALPHA			1
#define NOT_HAVE_ALPHA		0
 
typedef struct _png_pic_data {
	int width, height; 	//长宽
	int bit_depth; 	   	//位深度
	int alpha_flag;		//是否有透明通道
	unsigned char *rgba;//实际rgb数据
} png_pic_data_t;

int check_is_png(FILE **fp, const char *filename) //检查是否png文件
{
	char checkheader[PNG_BYTES_TO_CHECK]; //查询是否png头
	*fp = fopen(filename, "rb");
	if (*fp == NULL) {
		printf("open failed ...1\n");
		return -1;
	}
	if (fread(checkheader, 1, PNG_BYTES_TO_CHECK, *fp) != PNG_BYTES_TO_CHECK) //读取png文件长度错误直接退出
		return -1;
	return png_sig_cmp((png_const_bytep)checkheader, 0, PNG_BYTES_TO_CHECK); //0正确, 非0错误
}
 
int png_decode_data(const char *filename, png_pic_data_t *out) //取出png文件中的rgb数据
{
	png_structp png_ptr; //png文件句柄
	png_infop	info_ptr;//png图像信息句柄
	FILE *fp;
	if (check_is_png(&fp, filename) != 0) {
		printf("file is not png ...\n");
		return -1;
	}
	printf("launcher[%s] ...\n", PNG_LIBPNG_VER_STRING); //打印当前libpng版本号
 
	//1: 初始化libpng的数据结构 :png_ptr, info_ptr
	png_ptr  = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL); 
	info_ptr = png_create_info_struct(png_ptr);
 
	//2: 设置错误的返回点
	setjmp(png_jmpbuf(png_ptr));
	rewind(fp); //等价fseek(fp, 0, SEEK_SET);
 
	//3: 把png结构体和文件流io进行绑定 
	png_init_io(png_ptr, fp);
	//4:读取png文件信息以及强转转换成RGBA:8888数据格式
	png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_EXPAND, 0); //读取文件信息
	int channels, color_type; 
	channels 	= png_get_channels(png_ptr, info_ptr); //通道数量
	color_type 	= png_get_color_type(png_ptr, info_ptr);//颜色类型
	out->bit_depth = png_get_bit_depth(png_ptr, info_ptr);//位深度	
	out->width 	 = png_get_image_width(png_ptr, info_ptr);//宽
	out->height  = png_get_image_height(png_ptr, info_ptr);//高
	
	//if(color_type == PNG_COLOR_TYPE_PALETTE)
	//	png_set_palette_to_rgb(png_ptr);//要求转换索引颜色到RGB
	//if(color_type == PNG_COLOR_TYPE_GRAY && out->bit_depth < 8)
	//	png_set_expand_gray_1_2_4_to_8(png_ptr);//要求位深度强制8bit
	//if(out->bit_depth == 16)
	//	png_set_strip_16(png_ptr);//要求位深度强制8bit
	//if(png_get_valid(png_ptr,info_ptr,PNG_INFO_tRNS))
	//	png_set_tRNS_to_alpha(png_ptr);
	//if(color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
	//	png_set_gray_to_rgb(png_ptr);//灰度必须转换成RG
	printf("channels = %d color_type = %d bit_depth = %d width = %d height = %d ...\n",
			channels, color_type, out->bit_depth, out->width, out->height);
 
	int i, j, k;
	int size, pos = 0;
	int temp;
	
	//5: 读取实际的rgb数据
	png_bytepp row_pointers; //实际存储rgb数据的buf
	row_pointers = png_get_rows(png_ptr, info_ptr); //也可以分别每一行获取png_get_rowbytes();
	size = out->width * out->height; //申请内存先计算空间
	if (channels == 4 || color_type == PNG_COLOR_TYPE_RGB_ALPHA) { //判断是24位还是32位
		out->alpha_flag = HAVE_ALPHA; //记录是否有透明通道
		size *= (sizeof(unsigned char) * 4); //size = out->width * out->height * channel
		out->rgba = (png_bytep)malloc(size);
		if (NULL == out->rgba) {
			printf("malloc rgba faile ...\n");
			png_destroy_read_struct(&png_ptr, &info_ptr, 0);
			fclose(fp);
			return -1;
		}
		//从row_pointers里读出实际的rgb数据出来
		temp = channels - 1;
		for (i = 0; i < out->height; i++) 
			for (j = 0; j < out->width * 4; j += 4) 
				for (k = temp; k >= 0; k--)
					out->rgba[pos++] = row_pointers[i][j + k];
	} else if (channels == 3 || color_type == PNG_COLOR_TYPE_RGB) { //判断颜色深度是24位还是32位
		out->alpha_flag = NOT_HAVE_ALPHA;
		size *= (sizeof(unsigned char) * 3);
		out->rgba = (png_bytep)malloc(size);
		if (NULL == out->rgba) {
			printf("malloc rgba faile ...\n");
			png_destroy_read_struct(&png_ptr, &info_ptr, 0);
			fclose(fp);
			return -1;
		}
		//从row_pointers里读出实际的rgb数据
		temp = (3 * out->width);
		for (i = 0; i < out->height; i ++) {
			for (j = 0; j < temp; j += 3) {
				out->rgba[pos++] = row_pointers[i][j+2];
				out->rgba[pos++] = row_pointers[i][j+1];
				out->rgba[pos++] = row_pointers[i][j+0];
			}
		}
	} else return -1; 
	//6:销毁内存
	png_destroy_read_struct(&png_ptr, &info_ptr, 0);
	fclose(fp);
	//此时， 我们的out->rgba里面已经存储有实际的rgb数据了
	//处理完成以后free(out->rgba)
	return 0;
}

int png_load_bitmap(const char *filename, uint32_t *bitmap) 
{
    png_pic_data_t out;
    if (png_decode_data(filename, &out) < 0)
        return -1;

    /* 绘制像素 */
    unsigned char *p;
    uint32_t color;
    int x, y;
    for (y = 0; y < out.height; y++) {
        for (x = 0; x < out.width; x++) {
            unsigned char r, g, b, a = 255;
            if (out.alpha_flag & HAVE_ALPHA) {
                p = out.rgba + (y * out.width + x) * 4;
                a = *p++;
                b = *p++;
                g = *p++;
                r = *p++;
            } else {
                p = out.rgba + (y * out.width + x) * 3;

                b = *p++;
                g = *p++;
                r = *p++;
            }
            color = (a << 24) | (r << 16) | (g << 8) |(b) ;
            bitmap[y * out.width + x] = color;
        }
    }
    free(out.rgba);
    return 0;
}

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
        exit(execv(path, NULL));
    }
    return 0;
}

#define BG_PIC_PATH "/res/bg.jpg"

int init_desktop()
{
    /* 桌面 */
    int layer = g_layer_new(0, 0, g_screen.width, g_screen.height);
    if (layer < 0) {
        printf("[desktop]: new desktop layer failed!\n");
        return -1;
    }
    desktop_layer = layer;
    g_layer_z(layer, 0);    /* 0 is desktop */
    /* draw desktop */
    g_layer_rect_fill(layer, 0, 0, g_screen.width, g_screen.height, GC_GRAY);
    /* refresh desktop */
    g_layer_refresh(layer, 0, 0, g_screen.width, g_screen.height);
    
    g_layer_set_focus(layer);   /* set as focus layer */

    g_layer_set_desktop(layer);   /* set as focus layer */
    
    desktop_bitmap = g_new_bitmap(g_screen.width, g_screen.height);
    if (desktop_bitmap == NULL)
        return -1;
    
    /* 加载背景图片 */
    #if 1
    if (jpg_load_bitmap(BG_PIC_PATH, desktop_bitmap->buffer, g_screen.width, g_screen.height) < 0)
        return -1;
        g_bitmap_sync(desktop_bitmap, desktop_layer, 0, 0);
    #if 0
    if (jpg_display(BG_PIC_PATH) < 0) {
        printf("show background pic %s failed!\n", BG_PIC_PATH);
    }
    #endif
    #endif
    printf("[desktop]: desktop layer=%d\n", desktop_layer);

    

    return 0;
}



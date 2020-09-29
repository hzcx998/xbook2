#if 0
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_resize.h"
#endif
#include "surface.h"
#include <assert.h>
#include <png.h>
#include <gapi.h>
#include <string.h>

#include <jpeglib.h>
#include <jmorecfg.h>
#include <jconfig.h>

#define PNG_BYTES_TO_CHECK	8
#define HAVE_ALPHA			1
#define NOT_HAVE_ALPHA		0
 
typedef struct _png_pic_data {
	int width, height; 	//长宽
	int bit_depth; 	   	//位深度
    int channels;       // 通道数
	int alpha_flag;		//是否有透明通道
	unsigned char *rgba;//实际rgb数据
} png_pic_data_t;

int check_is_png(FILE **fp, const char *filename) //检查是否png文件
{
	char checkheader[PNG_BYTES_TO_CHECK]; //查询是否png头
	*fp = fopen(filename, "rb");
	if (*fp == NULL) {
		printf("open file %s failed ...\n", filename);
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
		printf("file %s is not png ...\n", filename);
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
	out->channels = channels;
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

unsigned char *png_load_bitmap(const char *filename, int *width, int *height,
    int *channels_in_file) 
{
    png_pic_data_t out;
    if (png_decode_data(filename, &out) < 0)
        return NULL;

    uint32_t *bitmap = malloc(out.width * out.height * 4);
    if (bitmap == NULL) {
        free(out.rgba);
        return NULL;
    }
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
    *width = out.width;
    *height = out.height;
    *channels_in_file = out.channels;
    return (unsigned char *) bitmap;
}


// **指定图片的路径就可以调用这个jpg的解析。**
unsigned char *jpg_load_bitmap(char * path, int *width, int *height, int *channels_in_file)
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
        printf("fopen %s error!", path);
        return NULL;
    }

    jpeg_stdio_src(&dinfo, infile); //为解压对象dinfo指定要解压的文件

    /*
    3. 调用jpeg_read_header()获取图像信息

    */
    if (jpeg_read_header(&dinfo, TRUE) != JPEG_HEADER_OK) {
        fclose(infile);
        return NULL;
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
        return NULL;
    }
    printf("width %d, height %d, components %d.\n", dinfo.output_width, dinfo.output_height, dinfo.output_components);
    
    *width = dinfo.output_width;
    *height = dinfo.output_height;
    *channels_in_file = dinfo.output_components;
    
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

    uint32_t *bitmap = malloc(dinfo.output_width * dinfo.output_height * 4);
    if (bitmap == NULL) {
        free(buffer);
        fclose(infile);
        jpeg_destroy_decompress(&dinfo);
        return NULL;
    }

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

    fclose(infile);
    return (unsigned char *) bitmap;
}



enum stretch_mode {
    nearest = 0,  //最临近插值算法
    bilinear  //双线性内插值算法
};

void stretch(unsigned char *src_buf, int src_w, int src_h, 
            unsigned char *dst_buf, int dst_w, int dst_h, 
            int num_channels, enum stretch_mode mode)
{
    int bitcount = num_channels * 8;

    //原图像缓存
    int src_bufsz = src_w * num_channels * src_h;
    int src_linesz = bitcount * src_w / 8;

    int des_bufsz = ((dst_w * bitcount + 31) / 32) * 4 * dst_h;
    int des_linesz = ((dst_w * bitcount + 31) / 32) * 4;
    double rate_h = (double)src_h / dst_h;
    double rate_w = (double)src_w / dst_w;
    int i, j;
    //最临近插值算法
    if (mode == nearest)
    {
        for (i = 0; i < dst_h; i++)
        {
            //选取最邻近的点
            int tSrcH = (int)(rate_h * i + 0.5);
            for (j = 0; j < dst_w; j++)
            {
                int tSrcW = (int)(rate_w * j + 0.5);
                memcpy(&dst_buf[i * des_linesz] + j * bitcount / 8,&src_buf[tSrcH * src_linesz] + tSrcW * bitcount / 8,bitcount / 8);
            }
        }
    } //双线型内插值算法
    else
    {
        for (i = 0; i < dst_h; i++)
        {
            int tH = (int)(rate_h * i);
            int tH1 = min(tH + 1,src_h - 1);
            float u = (float)(rate_h * i - tH);
            for (j = 0; j < dst_w; j++)
            {
                int tW = (int)(rate_w * j);
                int tW1 = min(tW + 1,src_w - 1);
                float v = (float)(rate_w * j - tW);

                //f(i+u,j+v) = (1-u)(1-v)f(i,j) + (1-u)vf(i,j+1) + u(1-v)f(i+1,j) + uvf(i+1,j+1)
                for (int k = 0; k < 4; k++)
                {
                    dst_buf[i * des_linesz + j * bitcount / 8 + k] =
                        (1 - u)*(1 - v) * src_buf[tH * src_linesz + tW * bitcount / 8 + k] +
                        (1 - u)*v*src_buf[tH1 * src_linesz + tW * bitcount / 8+ k] +
                        u * (1 - v) * src_buf[tH * src_linesz + tW1 * bitcount / 8 + k] +
                        u * v * src_buf[tH1 * src_linesz + tW1 * bitcount / 8 + k];
                }
            }
        }
    }
}

surface_t *surface_image_load(char const *filename, int w, int h)
{
    int iw, ih, channels_in_file;

    printf("surface load image %s start.\n", filename);
    //char *image = stbi_load(filename, &iw, &ih, &channels_in_file, STBI_rgb_alpha);
    //assert(image);
    char *image;
    char *type =  strrchr(filename, '.');
    assert(type);
    type++;
    if (!strcmp(type, "png")) {
        image =  png_load_bitmap(filename, &iw, &ih, &channels_in_file);
    } else if (!strcmp(type, "jpg")) {
        image =  jpg_load_bitmap(filename, &iw, &ih, &channels_in_file);
    } else {
        assert(0);
    }
    
    assert(image);

    surface_t *s = surface_alloc(w, h);
    assert(s);
    stretch(image, iw, ih, (unsigned char *) s->pixels, w, h, 4, bilinear);
    //stbir_resize_uint8(image, iw, ih, 0, s->pixels, w, h, 0, STBI_rgb_alpha);
    
    #if 0
    for (int i = 0; i < s->height; i++)
    {
        for (int j = 0; j < s->width; j++)
        {
            color_t c = s->pixels[i * s->width + j];
            s->pixels[i * s->width + j] = (color_t){c.r, c.g, c.b, c.a};
        }
    }
    
    #endif
    
    if (image)
        free(image);
    return s;
}

surface_t *surface_image_resize(surface_t *image, int w, int h)
{
    assert(image && image->pixels);
    surface_t *s = surface_alloc(w, h);
    assert(s);
    stretch((unsigned char *) image->pixels, image->width, image->height, (unsigned char *) s->pixels, w, h, 4, nearest);
    //stbir_resize_uint8(image->pixels, image->width, image->height, 0, s->pixels, w, h, 0, STBI_rgb_alpha);
    return s;
}

#ifndef _XGUI_IMAGE_H
#define _XGUI_IMAGE_H

unsigned char *xgui_load_png_image(const char *filename, int *width, int *height, int *channels_in_file);
unsigned char *xgui_load_jpg_image(const char *filename, int *width, int *height, int *channels_in_file);
unsigned char *xgui_load_image(const char *filename, int *width, int *height, int *channels_in_file);

/* 图片缩放算法 */
typedef enum {
    GRSZ_NEAREST = 0,   /* 最临近插值算法 */
    GRSZ_BILINEAR       /* 双线性内插值算法 */
} xgui_image_stretch_mode_t;

void xgui_resize_image_mode(unsigned char *src_buf, int src_w, int src_h, 
        unsigned char *dst_buf, int dst_w, int dst_h, 
        int num_channels, xgui_image_stretch_mode_t mode);
        
void xgui_resize_image(unsigned char *src_buf, int src_w, int src_h, 
        unsigned char *dst_buf, int dst_w, int dst_h, 
        int num_channels);
#endif /* _XGUI_IMAGE_H */

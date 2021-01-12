#ifndef _LIB_XTK_IMAGE_H
#define _LIB_XTK_IMAGE_H

typedef struct {
    int w;
    int h;
    int channels;
    unsigned char *buf; 
} xtk_image_t;

xtk_image_t *xtk_image_load(char *filename);
int xtk_image_destroy(xtk_image_t *img);
int xtk_image_resize(xtk_image_t *img, int w, int h);
xtk_image_t *xtk_image_load2(char *filename, int w, int h);

#endif /* _LIB_XTK_IMAGE_H */
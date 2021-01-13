#ifndef _LIB_XTK_COLLISION_H
#define _LIB_XTK_COLLISION_H

#include <uview.h>
#include <stdbool.h>

typedef struct {
    int x;
    int y;
    int width;
    int height;
    uview_color_t color;
    char visible;
} xtk_collision_t;

#define XTK_COLLISION_INVISIBLE     0 
#define XTK_COLLISION_VISIBLE       1

#define XTK_COLLISION_COLOR UVIEW_BLUE

xtk_collision_t *xtk_collision_create(int x, int y, int width, int height);
int xtk_collision_destroy(xtk_collision_t *collision);
int xtk_collision_set_pos(xtk_collision_t *collision, int x, int y);
int xtk_collision_set_size(xtk_collision_t *collision, int width, int height);
int xtk_collision_set_color(xtk_collision_t *collision, uview_color_t color);
int xtk_collision_set_visible(xtk_collision_t *collision, char visible);
bool xtk_collision_check_point(xtk_collision_t *collision, int x, int y);
int xtk_collision_show(xtk_collision_t *collision, uview_bitmap_t *bmp, int x, int y);
#define xtk_collision_get_width(collision) ((collision)->width)
#define xtk_collision_get_height(collision) ((collision)->height)

#endif /* _LIB_XTK_COLLISION_H */
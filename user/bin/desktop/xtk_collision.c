#include "xtk_collision.h"
#include <stdlib.h>
#include <string.h>

xtk_collision_t *xtk_collision_create(int x, int y, int width, int height)
{
    xtk_collision_t *collision = malloc(sizeof(xtk_collision_t));
    if (!collision)
        return NULL;
    collision->x = x;
    collision->y = y;
    collision->width = width;
    collision->height = height;
    collision->visible = 0;
    collision->color = XTK_COLLISION_COLOR;
    return collision;
}

int xtk_collision_destroy(xtk_collision_t *collision)
{
    if (!collision)
        return -1;
    memset(collision, 0, sizeof(xtk_collision_t));
    free(collision);
    return 0;
}

int xtk_collision_set_pos(xtk_collision_t *collision, int x, int y)
{
    if (!collision)
        return -1;
    collision->x = x;
    collision->y = y;
    return 0;
}

int xtk_collision_set_size(xtk_collision_t *collision, int width, int height)
{
    if (!collision)
        return -1;
    collision->width = width;
    collision->height = height;
    return 0;
}

int xtk_collision_set_visible(xtk_collision_t *collision, char visible)
{
    if (!collision)
        return -1;
    collision->visible = visible;
    return 0;
}

int xtk_collision_set_color(xtk_collision_t *collision, uview_color_t color)
{
    if (!collision)
        return -1;
    collision->color = color;
    return 0;
}

bool xtk_collision_check_point(xtk_collision_t *collision, int x, int y)
{
    if (!collision)
        return false;
    if (collision->x <= x && x < collision->x + collision->width && 
        collision->y <= y && y < collision->y + collision->height)
        return true;
    return false;
}

int xtk_collision_show(xtk_collision_t *collision, uview_bitmap_t *bmp, int x, int y)
{
    if (!collision)
        return -1;
    /* 包围盒 */
    if (collision->visible) {
        uview_bitmap_rect(bmp, x + collision->x, y + collision->y,
            collision->width, collision->height, collision->color);
    }
    return 0;
}
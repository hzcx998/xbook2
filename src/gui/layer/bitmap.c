#include <gui/draw.h>

/**
 * 同步位图到图层的某个区域范围内
 * 
*/
void layer_sync_bitmap(layer_t *layer, gui_rect_t *rect, GUI_COLOR *bitmap, gui_region_t *region)
{
    /* TODO: 进行矩形裁剪后再刷新 */
    int x, y, i, j;
    GUI_COLOR c;
    if (region && gui_region_valid(region)) {
        for (j = 0; j < rect->height; j++) {
            y = rect->y + j;
            if (y < region->top || y >= region->bottom)
                continue;
            for (i = 0; i < rect->width; i++) {
                x = rect->x + i;
                if (x < region->left || x >= region->right)
                    continue;
                
                c = bitmap[j * rect->width + i];
                if ((c >> 24) & 0xff) { /* 不透明才填充 */
                    layer_put_point(layer, x, y, c);
                }
            }
        }
    } else {
        for (j = 0; j < rect->height; j++) {
            y = rect->y + j;
            for (i = 0; i < rect->width; i++) {
                x = rect->x + i;
                c = bitmap[j * rect->width + i];
                if ((c >> 24) & 0xff) { /* 不透明才填充 */
                    layer_put_point(layer, x, y, c);
                }
            }
        }
    }
}

/**
 * 同步位图到图层的某个区域范围内
 * 
*/
void layer_copy_bitmap(layer_t *layer, gui_rect_t *rect, GUI_COLOR *bitmap, gui_region_t *region)
{
    /* TODO: 进行矩形裁剪后再刷新 */
    int x, y, i, j;
    GUI_COLOR c;
    if (region && gui_region_valid(region)) {
        for (j = 0; j < rect->height; j++) {
            y = rect->y + j;
            if (y < region->top || y >= region->bottom)
                continue;
            for (i = 0; i < rect->width; i++) {
                x = rect->x + i;
                if (x < region->left || x >= region->right)
                    continue;
                
                layer_get_point(layer, x, y, &c);
                bitmap[j * rect->width + i] = c;
            }
        }
    } else {
        for (j = 0; j < rect->height; j++) {
            y = rect->y + j;
            for (i = 0; i < rect->width; i++) {
                x = rect->x + i;
                layer_get_point(layer, x, y, &c);
                bitmap[j * rect->width + i] = c;
            }
        }
    }
}
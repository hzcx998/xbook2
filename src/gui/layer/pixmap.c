#include <gui/draw.h>

/**
 * layer_draw_pixmap - 绘制一个缓冲区里面的数据
 * @layer: 
 * @x: 横坐标
 * @y: 纵坐标
 * @width: 位图数据宽度
 * @height: 位图数据高度
 * @buffer: 位图数据缓冲区
 */
void layer_draw_pixmap(layer_t *layer, int x, int y, uint32_t width, uint32_t height, GUI_COLOR *buffer, int bps)
{
    int i, j;
    if (bps == 3) {
        uint8_t *buf = (uint8_t *) buffer;
        GUI_COLOR color;
        for (j = 0; j < height; j++) {
            for (i = 0; i < width; i++) {
                color = COLOR_RGB(buf[2], buf[1], buf[0]);
                layer_put_point(layer, x + i, y + j, color);
                buf += 3;
            }
        }
    } else if (bps == 4) {
        for (j = 0; j < height; j++) {
            for (i = 0; i < width; i++) {
                layer_put_point(layer, x + i, y + j, buffer[j * width + i]);
            }
        }
    } 
}
/**
 * 同步位图到图层的某个区域范围内
 * 
*/
void layer_sync_bitmap(layer_t *layer, gui_rect_t *rect, GUI_COLOR *bitmap, gui_region_t *region)
{
    /* TODO: 进行矩形裁剪后再刷新 */
    int x, y, i, j;
    GUI_COLOR c;
    if (gui_region_valid(region)) {
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
#include <layer/point.h>
#include <layer/bitmap.h>

/**
 * layer_draw_bitmap - 绘制一个缓冲区里面的数据
 * @layer: 
 * @x: 横坐标
 * @y: 纵坐标
 * @width: 位图数据宽度
 * @height: 位图数据高度
 * @buffer: 位图数据缓冲区
 */
void layer_draw_bitmap(layer_t *layer, int x, int y, int width, int height, GUI_COLOR *buffer)
{
    int i, j;
    for (j = 0; j < height; j++) {
        for (i = 0; i < width; i++) {
            layer_put_point(layer, x + i, y + j, buffer[j * width + i]);
        }
    }
}

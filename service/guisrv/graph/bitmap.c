#include <graph/point.h>
#include <graph/bitmap.h>

/**
 * graph_draw_bitmap - 绘制一个位图
 * @x: 横坐标
 * @y: 纵坐标
 * @width: 位图数据宽度
 * @height: 位图数据高度
 * @buffer: 位图数据缓冲区
 */
void graph_draw_bitmap(int x, int y, int width, int height, GUI_COLOR *buffer)
{
    int i, j;
    for (j = 0; j < height; j++) {
        for (i = 0; i < width; i++) {
            graph_put_point(x + i, y + j, buffer[j * width + i]);
        }
    }
}

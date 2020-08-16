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

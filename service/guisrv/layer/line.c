#include <layer/line.h>
#include <layer/point.h>
#include <drivers/screen.h>

void layer_put_vline(layer_t *layer, int left, int top, int buttom, GUI_COLOR color)
{
    int i;
    for (i = top; i <= buttom; i++) {
        layer_put_point(layer, left, i, color);
    }
}

void layer_put_hline(layer_t *layer, int left, int right, int top, GUI_COLOR color)
{
    int i;
    for (i = left; i <= right; i++) {
        layer_put_point(layer, i, top, color);
    }
}

void layer_draw_line(layer_t *layer, int x0, int y0, int x1, int y1, GUI_COLOR color)
{
    if (x0 == x1) { /* 垂直的线 */
        if (y0 < y1) 
            layer_put_vline(layer, x0, y0, y1, color);
        else 
            layer_put_vline(layer, x0, y1, y0, color);
        return;
    } else if (y0 == y1) {  /* 水平的直线 */
        if (x0 < x1) 
            layer_put_hline(layer, x0, x1, y0, color);
        else 
            layer_put_hline(layer, x1, x0, y0, color);
        return;
    }
    int i, x, y, len, dx, dy;
	dx = x1 - x0;
	dy = y1 - y0;
	
	x = x0 << 10;
	y = y0 << 10;
	
	if(dx < 0){
		dx = -dx;
	}
	if(dy < 0){
		dy = -dy;
	}
	if(dx >= dy ){
		len = dx + 1;
		if(x0 > x1){
			dx = -1024;
		} else {
			dx = 1024;
		}
		if(y0 <= y1){
			dy = ((y1 - y0 + 1) << 10)/len;
		} else {
			dy = ((y1 - y0 - 1) << 10)/len;
		}
		
		
	}else{
		len = dy + 1;
		if(y0 > y1){
			dy = -1024;
		} else {
			dy = 1024;
			
		}
		if(x0 <= x1){
			dx = ((x1 - x0 + 1) << 10)/len;
		} else {
			dx = ((x1 - x0 - 1) << 10)/len;
		}	
	}
	for(i = 0; i < len; i++){
        layer_put_point(layer, (x >> 10), (y >> 10), color);
		x += dx;
		y += dy;
	}
}

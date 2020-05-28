#include <graph/line.h>
#include <graph/point.h>
#include <drivers/screen.h>

void graph_draw_line(int x0, int y0, int x1, int y1, GUI_COLOR color)
{
    if (x0 == x1) { /* 垂直的线 */
        if (y0 < y1) 
            screen.output_vline(x0, y0, y1, screen.gui_to_screen_color(color));
        else 
            screen.output_vline(x0, y1, y0, screen.gui_to_screen_color(color));
        return;
    } else if (y0 == y1) {  /* 水平的直线 */
        if (x0 < x1) 
            screen.output_hline(x0, x1, y0, screen.gui_to_screen_color(color));
        else 
            screen.output_hline(x1, x0, y0, screen.gui_to_screen_color(color));
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
        graph_put_point((x >> 10), (y >> 10), color);
		x += dx;
		y += dy;
	}
}

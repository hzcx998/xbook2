#ifndef _INVADER_H
#define _INVADER_H

#include <xtk.h>

#define WINDOW_NAME "invader"

#define BACK_COLOR XTK_RGB(0,0,0)
#define MIDDLE_COLOR XTK_RGB(190,190,190)
#define FRONT_COLOR XTK_RGB(255,255,255)

#define WIN_WIDTH 320
#define WIN_HEIGHT 240

/*
用一个窗口结构体保存所有窗体信息
*/
struct window_s
{
    xtk_spirit_t *win;
    xtk_surface_t *surface;
	int width, height;	//窗口的宽高
	uint32_t bcolor;	//背景颜色
	uint32_t fcolor;	//前景颜色
}window;

int init_window(int width, int height);

void setdec8(char *s, int i);
void putstr(int x, int y, uint32_t color, char *s);
void time_wait(int i);

#endif  //_INVADER_H

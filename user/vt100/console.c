#include "vt.h"
#include <gapi.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

extern int g_win;
struct vt100 vt100;
struct vt100 *vt_term;

void _puts(char *str) {
	vt100_puts(vt_term, str, strlen(str));
}

void _putc(char c){
	vt100_puts(&vt100, &c, 1);
}


static void send_response(char *str)
{
}

static void _draw_char(struct vt100 *term, uint16_t x, uint16_t y, uint8_t ch, bool bsync)
{
    g_color_t bg = *(g_color_t *)&term->back_color;
    g_color_t fg = *(g_color_t *)&term->front_color;

    x = x * term->char_width;
    y = y * term->char_height;

    #if 0
    /* 绘制背景 */
    g_window_rect_fill(g_win, x, y, term->char_width, term->char_height, bg);
    /* 绘制字符 */
    g_window_char(g_win, x, y, ch, fg);
    #else
    g_bitmap_t *render = g_new_bitmap(term->char_width, term->char_height); 
    g_rectfill(render, 0, 0, term->char_width, term->char_height, bg);
    g_char(render, 0, 0, ch, fg);
    #endif

    if (bsync == TRUE)
    {
        //framebuffer_present_render(fb, render, &(struct dirty_rect_t) {x, y, term->char_width, term->char_height}, 1);
        //sync(term);
        #if 0
        g_refresh_window_rect(g_win, x, y, term->char_width, term->char_height);
        #else
        g_paint_window(g_win, x, y, render);
        #endif
    }
    g_del_bitmap(render);
}

static void draw_char(struct vt100 *term, uint16_t x, uint16_t y, uint8_t ch)
{
    _draw_char(term, x, y, ch, TRUE);
}

static void fill_rect(struct vt100 *term, uint16_t x, uint16_t y, uint16_t w, uint16_t h, int front)
{
    g_color_t bg;
    if (front)
        bg = *(g_color_t *)&term->front_color;
    else
        bg = *(g_color_t *)&term->back_color;
    #if 0
    g_window_rect_fill(g_win, term->char_width * x, term->char_height * y, term->char_width * w, term->char_height * h, bg);
    g_refresh_window_rect(g_win,term->char_width * x, term->char_height * y, term->char_width * w, term->char_height * h);
    #else
    g_bitmap_t *render = g_new_bitmap(term->char_width * w, term->char_height * h);
    g_rectfill(render, 0, 0, term->char_width * w, term->char_height * h, bg); 
    g_paint_window(g_win, term->char_width * x, term->char_height * y, render);
    g_del_bitmap(render);
    #endif
}

static void move_char(struct vt100 *term, uint16_t dx, uint16_t dy, uint16_t sx, uint16_t sy)
{
	dx = dx * term->char_width;
	dy = dy * term->char_height;
	sx = sx * term->char_width;
	sy = sy * term->char_height;
    g_color_t col;
    g_bitmap_t *render = g_new_bitmap(term->char_width, term->char_height);

    /* 获取一个矩形区域里面的数据 */
    g_paint_window_copy(g_win, sx, sy, render);
    #if 0   
    for (int b = 0; b < term->char_height; b++)
	{
		for (int j = 0; j < term->char_width; j++)
		{
            g_window_get_point(g_win, sx + j, sy + b, &col);
			g_window_put_point(g_win, dx + j, dy + b, col);
         
         }
	}
    #endif

    //g_refresh_window_rect(g_win, dx, dy, term->char_width, term->char_height);
    g_paint_window(g_win, dx, dy, render);
    g_del_bitmap(render);
}

static void scroll(struct vt100 *term, int lines)
{
	uint16_t top = term->scroll_start_row;
	uint16_t bottom = term->scroll_end_row;
	int height = bottom - top + 1;

	if (lines > 0)
		for (int i = top; i <= bottom - lines; i++)
			for (int j = 0; j < term->width; j++)
				move_char(term, j, i, j, i + lines);
	else if (lines < 0)
		for (int i = bottom; i >= top - lines; i--)
			for (int j = 0; j < term->width; j++)
				move_char(term, j, i, j, i + lines);

	if (lines < 0) {
		term->fill_rect(term, 0, top, term->width, -lines, 0);
	} else if (lines > 0) {
		term->fill_rect(term, 0, bottom - lines + 1, term->width, lines, 0);
	}
}

static void terminal_init(struct vt100 *term, int char_width, int char_height, int screen_width, int screen_height)
{
    term->char_width = char_width;
    term->char_height = char_height;
    term->font_width = term->char_width;
    term->font_height = term->char_height;
    term->screen_width = screen_width;
    term->screen_height = screen_height;
    term->width = term->screen_width / term->char_width;
    term->height = term->screen_height / term->char_height;
    term->draw_char = draw_char;
    term->fill_rect = fill_rect;
    term->scroll = scroll;
    term->send_response = send_response;
    vt100_init(term);
}

int init_console(int screen_width, int screen_height)
{
    terminal_init(&vt100, 8, 16, screen_width, screen_height);
    vt_term = &vt100;
    return 0;
}

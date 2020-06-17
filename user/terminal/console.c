#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "terminal.h"
#include "window.h"
#include "cmd.h"
#include "cursor.h"
#include "console.h"

/* 控制台全局变量 */
con_screen_t screen;

void con_set_chars(char ch, int counts, int x, int y)
{
    int cx = x, cy = y;
    while (counts > 0) {
        con_set_char(ch, cx, cy);
        cx++;
        if (cx >= screen.columns) {
            cx = 0;
            cy++;
            /*if (cy >= screen.rows) {
                cy = screen.rows;
            }*/
        }
        counts--;
    }
    
}

void con_select_char(int cx, int cy)
{
    char ch;
    
    con_get_char(&ch, cx, cy);

    int x = cx * screen.char_width, y = cy * screen.char_height;
    
    SGI_Argb bgcolor = (0xffffff - (screen.background_color & 0xffffff)) | (0xff << 24);
    SGI_Argb fontcolor = (0xffffff - (screen.font_color & 0xffffff)) | (0xff << 24);

    /* 绘制背景 */
    SGI_DrawFillRect(screen.display, screen.win, x, y,
        screen.char_width, screen.char_height, bgcolor);
    
    /* 绘制字符 */
    SGI_DrawChar(screen.display, screen.win, x, y, ch, fontcolor);
}

void con_region_chars(int x0, int y0, int x1, int y1)
{
    int x, y;
    int cx, cy; // 字符坐标

    /* 计算刷新区域 */
    int left, right, top, bottom;
    top = min(y0, y1);
    bottom = max(y0, y1);
    
    top = top / screen.char_height * screen.char_height;
    bottom = bottom / screen.char_height * screen.char_height + screen.char_height;
    
    /* 计算行数 */
    int lines = (bottom - top) / screen.char_height;
    if (lines > 1) {    /* 选取多行，就需要刷新多行 */
        if (y1 > y0) {  /* 第一个点位于上方，那么，第一行的左边就是第一个点 */
            left = x0;
        } else {    /* 第二个点位于上方，那么，第一行的左边就是第二个点 */
            left = x1;
        }
        /* 第一行从左最小到右边框 */
        right = screen.width;
        left = left / screen.char_width * screen.char_width;
        y = top;
        for (x = left; x < right; x += screen.char_width) {
            cx = x / screen.char_width;
            cy = y / screen.char_height;
            /* 选中某个字符 */
            con_select_char(cx, cy);    
        }
        y += screen.char_height;
        if (lines > 2) { /* 大于2行 */
            /* 整行都选择 */
            left = 0;
            right = screen.width;
            while (lines > 2) {
                for (x = left; x < right; x += screen.char_width) {
                    cx = x / screen.char_width;
                    cy = y / screen.char_height;
                    /* 选中某个字符 */
                    con_select_char(cx, cy);    
                }
                y += screen.char_height;
                lines--;
            } 
        }
        if (y1 > y0) {
            right = x1; /* 第二个点位于下方，那么，最后一行的右边就是第二个点 */
        } else {
            right = x0; /* 第一个点位于下方，那么，最后一行的右边就是第一个点 */
        }
        /* 从左边框到最右最大 */
        left = 0;
        right = right / screen.char_width * screen.char_width + screen.char_width;
        /* 最后一行 */
        for (x = left; x < right; x += screen.char_width) {
            cx = x / screen.char_width;
            cy = y / screen.char_height;
            /* 选中某个字符 */
            con_select_char(cx, cy);    
        }
    } else {
        /* 最左到最右 */
        left = min(x0, x1);
        right = max(x0, x1);
        left = left / screen.char_width * screen.char_width;
        right = right / screen.char_width * screen.char_width + screen.char_width;
        /* 向下对齐 */
        for (y = top; y < bottom; y += screen.char_height) {
            for (x = left; x < right; x += screen.char_width) {
                cx = x / screen.char_width;
                cy = y / screen.char_height;
                /* 选中某个字符 */
                con_select_char(cx, cy);    
            }
        }
    }
    if (bottom - top > screen.char_height) {    /* 选取多行，就需要刷新多行 */
        left = 0;
        right = screen.width;
    } else {
        left = min(x0, x1);
        right = max(x0, x1);
        left = left / screen.char_width * screen.char_width;
        right = right / screen.char_width * screen.char_width + screen.char_width;
    }
    /* 只刷新一次 */
    SGI_UpdateWindow(screen.display, screen.win, left, top, right, bottom);
}

void draw_char(char ch)
{
    int x = cursor.x * screen.char_width;
    int y = cursor.y * screen.char_height;
    
    /* 绘制背景 */
    SGI_DrawFillRect(screen.display, screen.win, x, y,
        screen.char_width, screen.char_height, screen.background_color);
    
    /* 绘制字符 */
    SGI_DrawChar(screen.display, screen.win, x, y, ch, screen.font_color);

    /* 刷新光标 */
    SGI_UpdateWindow(screen.display, screen.win, x, y, 
        x + screen.char_width, y + screen.char_height);
}

void load_char_buffer()
{
	int bx, by, x, y;
	char ch;
	for (by = 0; by < screen.rows; by++) {
		for (bx = 0; bx < screen.columns; bx++) {
			con_get_char(&ch, bx, by);
			
			x = bx * screen.char_width;
			y = by * screen.char_height;

            /* 绘制字符 */
            SGI_DrawChar(screen.display, screen.win, x, 
                y, ch, screen.font_color);
		}
    }
}

static int can_scroll_up()
{
	if (screen.cur_pos > screen.buffer) {
		return 1;
	}
	return 0;
}

static int can_scroll_down()
{
	if (screen.cur_pos < (screen.buffer + screen.buflen) - \
        screen.rows * screen.columns) {
		return 1;
	}
	return 0;
}




/**
 * con_flush - 刷新屏幕
 * 
 */
void con_flush()
{
    //清空背景
    SGI_DrawFillRect(screen.display, screen.win, 0, 0,
        screen.width, screen.height, screen.background_color);

    //把字符全部加载到窗口
    load_char_buffer();

    /* 刷新全部 */
    SGI_UpdateWindow(screen.display, screen.win, 0, 0,
        screen.width, screen.height);

    draw_cursor();
}

/**
 * scroll_screen - 向上或者向下滚动屏幕
 * @dir: 滚动方向
 * @lines 滚动几行
 * @cursorx: 滚动后光标x是否改变
 * @cursory: 滚动后光标y是否改变
 *
 */
void scroll_screen(int dir, int lines, int cursorx, int cursory)
{
	if (dir == CON_SCROLL_UP) {
		//判断是否能滚屏
		if (!can_scroll_up()) {
            //如果不能向下滚屏就返回
			return;
		}

        //清空背景
        SGI_DrawFillRect(screen.display, screen.win, 0, 0,
            screen.width, screen.height, screen.background_color);

        //修改显存起始位置
        screen.cur_pos -= screen.columns * lines;
        
		//把字符全部加载到窗口
		load_char_buffer();

        /* 刷新全部 */
        SGI_UpdateWindow(screen.display, screen.win, 0, 0,
            screen.width, screen.height);
        if (cursorx)
            cursor.x = 0;
		if (cursory) {
            cursor.y += lines;
            if (cursor.y > screen.rows - 1) {
                //cursor.y = screen.rows - 1;
                cursor.visual = 0;
            } else {
                cursor.visual = 1;
            }
        }
        //修改光标位置
		draw_cursor();
	} else if (dir == CON_SCROLL_DOWN) {
		//判断是否能滚屏
		if (!can_scroll_down()) {
			//如果不能向下滚屏就返回
			return;
		}
		//清空背景
        SGI_DrawFillRect(screen.display, screen.win, 0, 0,
            screen.width, screen.height, screen.background_color);

        //修改显存起始位置
        screen.cur_pos += screen.columns * lines;
        
		//把字符全部加载到窗口
		load_char_buffer();

        /* 刷新全部 */
        SGI_UpdateWindow(screen.display, screen.win, 0, 0,
            screen.width, screen.height);
        
		//if (!accord) {
        if (cursorx)
            cursor.x = 0;

        if (cursory) {
			cursor.y -= lines;
			if (cursor.y < 0) {
                //cursor.y = 0;
                cursor.visual = 0;
			} else {
                cursor.visual = 1;
            }
        }
		//}
        //绘制光标
		draw_cursor();
	}
}

void con_set_cur_pos()
{
    //清空背景
    SGI_DrawFillRect(screen.display, screen.win, 0, 0,
        screen.width, screen.height, screen.background_color);

}


/*
显示一个可见字符
*/
void con_ouput_visual(char ch, int x, int y)
{
	if (0x20 <= ch && ch <= 0x7e) {
        /* 绘制字符 */
        SGI_DrawChar(screen.display, screen.win, x, 
            y, ch, screen.font_color);

        /* 刷新光标 */
        SGI_UpdateWindow(screen.display, screen.win, x, y, 
            x * screen.char_width, y * screen.char_height);
	}
}

void con_out_char(char ch)
{
	//先把光标去除
	clean_cursor();
    int counts;
	//对字符进行设定，如果是可显示字符就显示
	switch (ch) {
		case '\n':
			//光标的位置设定一个字符
			con_set_char(' ', cursor.x, cursor.y);
            
			//能否回车
			if (can_scroll_down())
				move_cursor(0, cursor.y + 1);
			break;
		case '\b':
			//改变位置
			cursor.x--;

			//改变位置后需要做检测，因为要写入字符
			cursor_pos_check();

			con_set_char(0, cursor.x, cursor.y);

			draw_cursor();
			break;
		case '\t':
            /* 离当前位置有多少个字符 */
            counts = ((cursor.x + 4) & (~(4 - 1))) - cursor.x;
            while (counts--) {
                con_set_char(' ', cursor.x, cursor.y);
                move_cursor(cursor.x + 1, cursor.y);
            }
			break;
        case '\r':  /* 不处理 */
            break;
		default :
			con_set_char(ch, cursor.x, cursor.y);

            move_cursor(cursor.x + 1, cursor.y);
            break;
	}
}

void con_out_str(char *str)
{
    while (*str)
    {
        con_out_char(*str);
        str++;
    }
}

/*
清除屏幕上的所有东西，
字符缓冲区里面的文字
*/
static void con_clear()
{
	//清空背景
    SGI_DrawFillRect(screen.display, screen.win, 0, 0,
        screen.width, screen.height, screen.background_color);

    /* 刷新全部 */
    SGI_UpdateWindow(screen.display, screen.win, 0, 0,
        screen.width, screen.height);
    
	//清空字符缓冲区
	memset(screen.buffer, 0, screen.buflen);

	//修改字符缓冲区指针
	screen.cur_pos = screen.buffer;

	//重置光标
	cursor.x = 0;
	cursor.y = 0;

	//绘制光标
	draw_cursor();
}

/*
清除屏幕上的所有东西，
字符缓冲区里面的文字
*/
static void con_clear_area(int x, int y, unsigned int width, unsigned int height)
{
	//清空背景
    SGI_DrawFillRect(screen.display, screen.win, x, y,
        width, height, screen.background_color);

    /* 刷新全部 */
    SGI_UpdateWindow(screen.display, screen.win, x, y,
        x + width, y + height);
    
}

int cprintf(const char *fmt, ...)
{
	char buf[STR_DEFAULT_LEN];
	va_list arg = (va_list)((char*)(&fmt) + 4); /*4是参数fmt所占堆栈中的大小*/
	vsprintf(buf, fmt, arg);
	
    /* 输出到控制台 */
    screen.outs(buf);
	return 0;
}

int init_con_screen()
{
    screen.buflen = CON_BUFFER_SIZE;
    screen.buffer = malloc(screen.buflen);
    if (screen.buffer == NULL)
        return -1;
    memset(screen.buffer, 0, screen.buflen);

    screen.cur_pos = screen.buffer;
    screen.width = CON_SCREEN_WIDTH;
    screen.height = CON_SCREEN_HEIGHT;
    screen.rows = CON_ROWS;
    screen.columns = CON_CLOUMNS;
    screen.char_width = CON_CHAR_WIDTH;
    screen.char_height = CON_CHAR_HEIGHT;
    screen.codepage = CON_CODEPAGE;
    screen.background_color = CON_SCREEN_BG_COLOR;
    screen.font_color = CON_SCREEN_FONT_COLOR;
    screen.select_color = CON_SCREEN_SELECT_COLOR;
    screen.mousex = -1;
    screen.mousey = -1;
    screen.display = NULL;
    screen.font = NULL;
    screen.win = 0;
    screen.outc = con_out_char;
    screen.outs = con_out_str;
    screen.clear = con_clear;
    screen.clear_area = con_clear_area;
    init_con_cursor();
    if (init_cmd_man() < 0) {
        free(screen.buffer);
        return -1;
    }

    return 0;
}
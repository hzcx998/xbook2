#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "terminal.h"
#include "window.h"

/* 控制台全局变量 */
con_screen_t screen;

/* 控制台光标 */
con_cursor_t cursor;


static inline void con_get_char(char *ch, int x, int y)
{

	*ch = screen.cur_pos[y * screen.columns + x];
}

static inline void con_set_char(char ch, int x, int y)
{
	//保存字符
	screen.cur_pos[y * screen.columns + x] = ch;
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

char cusor_size[CS_MAX_NR][2] = {
    {8, 16},
    {8, 16},
    {1, 16},
    {8, 1},
};

void set_cursor_size()
{
    cursor.width = cusor_size[(unsigned char) cursor.shape][0];
    cursor.height = cusor_size[(unsigned char) cursor.shape][1];
}

void clean_cursor()
{
    int x = cursor.x * screen.char_width;
    int y = cursor.y * screen.char_height;
    
    /* 绘制背景 */
    SGI_DrawFillRect(screen.display, screen.win, x, y,
        screen.char_width, screen.char_height, screen.background_color);
    
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
                cursor.y = screen.rows - 1;
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
				cursor.y = 0;
			}
        }
		//}
        //绘制光标
		draw_cursor();
	}
}

static void cursor_pos_check()
{
	//如果光标向左移动超出，就切换到上一行最后
	if (cursor.x < 0) {
		
		if (cursor.y > 0) {
			//向左移动，如果发现y > 0，那么就可以移动行尾
			cursor.x = screen.columns - 1;
		} else {
			//如果向左移动，发现y <= 0，那么就只能在行首
			cursor.x = 0;
		}
		//移动到上一行
		cursor.y--;
	}

	//如果光标向右移动超出，就切换到下一行
	if (cursor.x > screen.columns - 1) {
        /* 如果行超出最大范围，就回到开头 */
        if (cursor.y < screen.rows) {
            //如果y 没有到达最后一行，就移动到行首
			cursor.x = 0;
		} else {
			//如果y到达最后一行，就移动到行尾
			cursor.x = screen.columns - 1;
		}
		//移动到下一行
		cursor.y++;
	}

	//如果光标向上移动超出，就修复
	if (cursor.y < 0) {
		//做修复处理
		cursor.y = 0;

	}

	//如果光标向下移动超出，就向下滚动屏幕
	if (cursor.y > screen.rows -1) {
		//暂时做修复处理
		cursor.y = screen.rows -1;

		scroll_screen(CON_SCROLL_DOWN, 1, 1, 0);
	}
}

/*
显示一个可见字符
*/
static void con_ouput_visual(char ch, int x, int y)
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


/*
光标移动一个位置
x是x方向上的移动
y是y方向上的移动
*/
void move_cursor(int x, int y)
{
	//先把光标消除
	clean_cursor();

	//把原来位置上的字符显示出来
	char ch;
	con_get_char(&ch, cursor.x, cursor.y);
	
	//文字颜色
    con_ouput_visual(ch, cursor.x * screen.char_width,
        cursor.y * screen.char_height);

	//移动光标
	cursor.x = x;
	cursor.y = y;
	//修复位置
	cursor_pos_check();

	//显示光标
	draw_cursor();
	//把光标所在的字符显示出来
	con_get_char(&ch, cursor.x, cursor.y);
	
	//背景的颜色
	con_ouput_visual(ch, cursor.x * screen.char_width,
        cursor.y * screen.char_height);
}

void move_cursor_off(int x, int y)
{
    move_cursor(cursor.x + x, cursor.y + y);
}

void get_cursor(int *x, int *y)
{   
    *x = cursor.x;
    *y = cursor.y;
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


void draw_cursor()
{
    int x = cursor.x * screen.char_width;
    int y = cursor.y * screen.char_height;
    
    /* 绘制背景 */
    SGI_DrawFillRect(screen.display, screen.win, x, y,
        screen.char_width, screen.char_height, screen.background_color);
    
    /* 绘制光标 */
    SGI_DrawFillRect(screen.display, screen.win, x, y,
        cursor.width, cursor.height, cursor.color);
    
    /* 刷新光标 */
    SGI_UpdateWindow(screen.display, screen.win, x, y, 
        x + screen.char_width, y + screen.char_height);
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


void init_con_cursor()
{
    cursor.x = 0;
    cursor.y = 0;
    cursor.shape = CS_SOLID_FRAME;
    set_cursor_size();
    cursor.color = CON_CURSOR_COLOR;
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
    screen.display = NULL;
    screen.font = NULL;
    screen.win = 0;
    screen.outc = con_out_char;
    screen.outs = con_out_str;
    screen.clear = con_clear;
    init_con_cursor();

    return 0;
}
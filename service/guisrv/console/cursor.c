#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/// 程序本地头文件
#include <console/cursor.h>
#include <console/console.h>
#include <graph/rect.h>
#include <graph/text.h>

/* 控制台光标 */
con_cursor_t cursor;

char cursor_size[CS_MAX_NR][2] = {
    {8, 16},
    {8, 16}, 
    {1, 16},
    {8, 1},
};

void set_cursor_size()
{
    cursor.width = cursor_size[(unsigned char) cursor.shape][0];
    cursor.height = cursor_size[(unsigned char) cursor.shape][1];
}

void set_cursor_shape(int shape)
{
    cursor.shape = shape;
    set_cursor_size();
    draw_cursor();
}

void set_cursor_color(GUI_COLOR color)
{
    cursor.color = color;
    draw_cursor();
}

void clean_cursor()
{
    int x = cursor.x * screen.char_width;
    int y = cursor.y * screen.char_height;
    
    /* 绘制背景 */
    draw_rect_fill(x, y,
        screen.char_width, screen.char_height, screen.background_color);

}

void cursor_pos_check()
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
        cursor.visual = 0;
	} else {
        cursor.visual = 1;
    }
    
	//如果光标向下移动超出，就向下滚动屏幕
	if (cursor.y >= screen.rows) {
		//暂时做修复处理
		cursor.y = screen.rows - 1;

        scroll_screen(CON_SCROLL_DOWN, 1, 1, 0);
	} else {
        cursor.visual = 1;
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

void draw_cursor()
{
    if (!cursor.visual)
        return;

    int x = cursor.x * screen.char_width;
    int y = cursor.y * screen.char_height;
    
    /* 绘制背景 */
    draw_rect_fill(x, y,
        screen.char_width, screen.char_height, screen.background_color);
    //printf("bg color %x\n", screen.background_color);
    /* 绘制光标 */
    switch (cursor.shape)
    {
    case CS_SOLID_FRAME:
        draw_rect_fill(x, y,
            cursor.width, cursor.height, cursor.color);
        break;
    case CS_HOLLOW_BOX:
        draw_rect(x, y, cursor.width, cursor.height, cursor.color);
        break;
    case CS_VERTICAL_BAR:
        draw_rect_fill(x, y,
            cursor.width, cursor.height, cursor.color);
        break;
    case CS_UNDERLINE:
        draw_rect_fill(x, y + screen.char_height - 1,
            cursor.width, cursor.height, cursor.color);
        break;
    default:
        break;
    }
}

/** 
 * focus_cursor - 聚焦光标
 * 
 * 如果光标不再屏幕范围内，就会先聚焦到光标所在行
 */
void focus_cursor()
{
    if (!cursor.visual) {   /* 没有显示 */
        //printf("%s: cursor x=%d, y=%d\n", APP_NAME, cursor.x, cursor.y);
        if (cursor.y < 0) { /* 光标在顶部 */
            scroll_screen(CON_SCROLL_UP, -cursor.y, 0, 1);
        } else if (cursor.y >= screen.rows) { /* 光标在顶部 */
            scroll_screen(CON_SCROLL_DOWN, cursor.y - screen.rows + 1, 0, 1);
        }
    }
}

void print_cursor()
{
    //printf("%s: cursor x=%d, y=%d\n", APP_NAME, cursor.x, cursor.y);
}

void init_con_cursor()
{
    cursor.x = 0;
    cursor.y = 0;
    cursor.shape = CS_SOLID_FRAME;
    set_cursor_size();
    cursor.color = CON_CURSOR_COLOR;
    cursor.visual = 1;
}

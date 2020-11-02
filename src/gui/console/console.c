#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xbook/memalloc.h>

/// 程序本地头文件
#include <gui/console.h>
#include <gui/cursor.h>
#include <gui/rect.h>
#include <gui/text.h>
#include <gui/screen.h>

/* 控制台全局变量 */
con_screen_t gui_con_screen;

void con_set_chars(char ch, int counts, int x, int y)
{
    int cx = x, cy = y;
    while (counts > 0) {
        con_set_char(ch, cx, cy);
        cx++;
        if (cx >= gui_con_screen.columns) {
            cx = 0;
            cy++;
            /*if (cy >= gui_con_screen.rows) {
                cy = gui_con_screen.rows;
            }*/
        }
        counts--;
    }
    
}

void con_get_chars(char *buf, int counts, int x, int y)
{
    int cx = x, cy = y;
    char *p = buf;
    while (counts > 0) {
        con_get_char(p, cx, cy);
        cx++;
        if (cx >= gui_con_screen.columns) {
            cx = 0;
            cy++;
            /*if (cy >= gui_con_screen.rows) {
                cy = gui_con_screen.rows;
            }*/
        }
        counts--;
        if (*p) /* 非结束字符才继续往后移动 */
            p++;
    }
    
}

void gui_draw_char(char ch)
{
    int x = cursor.x * gui_con_screen.char_width;
    int y = cursor.y * gui_con_screen.char_height;
    
    /* 绘制背景 */
    gui_draw_rect_fill(x, y,
        gui_con_screen.char_width, gui_con_screen.char_height, gui_con_screen.background_color);
    
    /* 绘制字符 */
    gui_draw_word(x, y, ch, gui_con_screen.font_color);
}

void load_char_buffer()
{
	int bx, by, x, y;
	char ch = 0;
	for (by = 0; by < gui_con_screen.rows; by++) {
		for (bx = 0; bx < gui_con_screen.columns; bx++) {
			con_get_char(&ch, bx, by);
			
            if (0x20 <= ch && ch <= 0x7e) {
                x = bx * gui_con_screen.char_width;
			    y = by * gui_con_screen.char_height;
                /* 绘制字符 */
                gui_draw_word(x, 
                    y, ch, gui_con_screen.font_color);
            }
		}
    }
}

static int can_scroll_up()
{
	if (gui_con_screen.cur_pos > gui_con_screen.buffer) {
		return 1;
	}
	return 0;
}

static int can_scroll_down()
{
	if (gui_con_screen.cur_pos < (gui_con_screen.buffer + gui_con_screen.buflen) - \
        gui_con_screen.rows * gui_con_screen.columns) {
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
    gui_draw_rect_fill(0, 0,
        gui_con_screen.width, gui_con_screen.height, gui_con_screen.background_color);

    //把字符全部加载到窗口
    load_char_buffer();

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
        gui_draw_rect_fill(0, 0,
            gui_con_screen.width, gui_con_screen.height, gui_con_screen.background_color);

        //修改显存起始位置
        gui_con_screen.cur_pos -= gui_con_screen.columns * lines;
        
		//把字符全部加载到窗口
		load_char_buffer();

        if (cursorx)
            cursor.x = 0;
		if (cursory) {
            cursor.y += lines;
            if (cursor.y > gui_con_screen.rows - 1) {
                //cursor.y = gui_con_screen.rows - 1;
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
        gui_draw_rect_fill(0, 0,
            gui_con_screen.width, gui_con_screen.height, gui_con_screen.background_color);

        //修改显存起始位置
        gui_con_screen.cur_pos += gui_con_screen.columns * lines;
        
		//把字符全部加载到窗口
		load_char_buffer();

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

/*
显示一个可见字符
*/
void con_ouput_visual(char ch, int x, int y)
{
	if (0x20 <= ch && ch <= 0x7e) {
        /* 绘制字符 */
        gui_draw_word(x, 
            y, ch, gui_con_screen.font_color);

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
			con_set_char('\n', cursor.x, cursor.y);
            
			//能否回车
			if (can_scroll_down())
				move_cursor(0, cursor.y + 1);
			break;
		case '\b':
            con_set_char(' ', cursor.x, cursor.y);
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
    char *s = str;
    while (*s)
    {
        con_out_char(*s);
        s++;
    }
}

/*
清除屏幕上的所有东西，
字符缓冲区里面的文字
*/
static void con_clear()
{
    
	//清空字符缓冲区
	memset(gui_con_screen.buffer, 0, gui_con_screen.buflen);
    
	//修改字符缓冲区指针
	gui_con_screen.cur_pos = gui_con_screen.buffer;

	//重置光标
	cursor.x = 0;
	cursor.y = 0;

	//清空背景
    gui_draw_rect_fill(0, 0,
        gui_con_screen.width, gui_con_screen.height, gui_con_screen.background_color);
    //绘制光标
	//draw_cursor();
    
}

void con_set_back_color(GUI_COLOR color)
{
    gui_con_screen.background_color = color;
}

void con_set_font_color(GUI_COLOR color)
{
    gui_con_screen.font_color = color;
}

int cprintk(const char *fmt, ...)
{
	char buf[BUFSIZ];
	va_list arg = (va_list)((char*)(&fmt) + 4); /*4是参数fmt所占堆栈中的大小*/
	vsprintf(buf, fmt, arg);
	
    /* 输出到控制台 */
    gui_con_screen.outs(buf);
	return 0;
}

int gui_init_console()
{
    gui_con_screen.width = gui_screen.width;
    gui_con_screen.height = gui_screen.height;
    gui_con_screen.char_width = CON_CHAR_WIDTH;
    gui_con_screen.char_height = CON_CHAR_HEIGHT;
    gui_con_screen.rows = gui_con_screen.height / gui_con_screen.char_height;
    gui_con_screen.columns = gui_con_screen.width / gui_con_screen.char_width;
    gui_con_screen.columns_width = gui_con_screen.columns * gui_con_screen.char_width;
    gui_con_screen.rows_height = gui_con_screen.rows * gui_con_screen.char_height;

    gui_con_screen.codepage = CON_CODEPAGE;
    gui_con_screen.background_color = CON_SCREEN_BG_COLOR;
    gui_con_screen.font_color = CON_SCREEN_FONT_COLOR;
    gui_con_screen.mouse_color = CON_MOUSE_COLOR;
    gui_con_screen.outc = con_out_char;
    gui_con_screen.outs = con_out_str;
    gui_con_screen.clear = con_clear;

    gui_con_screen.buflen = gui_con_screen.rows * gui_con_screen.columns * CON_FRAME_NR;

    printk("[gui]: alloc console screen buffer rows %d columns %d frames %d size %x!\n", 
        gui_con_screen.rows ,gui_con_screen.columns, CON_FRAME_NR,gui_con_screen.buflen);
    
    gui_con_screen.buffer = mem_alloc(gui_con_screen.buflen);
    if (gui_con_screen.buffer == NULL)
        return -1;
    memset(gui_con_screen.buffer, 0, gui_con_screen.buflen);
    gui_con_screen.cur_pos = gui_con_screen.buffer;
    
    gui_draw_rect(0, 0, gui_screen.width, gui_screen.height, COLOR_BLACK);

    init_con_cursor();

    print_gui_console = 1;
    return 0;
}
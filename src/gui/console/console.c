#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <sys/input.h>
#include <sys/trigger.h>

/// 程序本地头文件
#include <gui/console/cursor.h>
#include <gui/console/console.h>
#include <gui/console/clipboard.h>
#include <gui/screen.h>
#include <gui/keyboard.h>
#include <gui/mouse.h>
#include <sys/input.h>
#include <gui/rect.h>
#include <gui/text.h>
#include <gui/event.h>


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

void con_select_char(int cx, int cy)
{
    char ch;
    
    con_get_char(&ch, cx, cy);

    int x = cx * gui_con_screen.char_width, y = cy * gui_con_screen.char_height;
    
    GUI_COLOR bgcolor = (0xffffff - (gui_con_screen.background_color & 0xffffff)) | (0xff << 24);
    GUI_COLOR fontcolor = (0xffffff - (gui_con_screen.font_color & 0xffffff)) | (0xff << 24);

    /* 绘制背景 */
    gui_draw_rect_fill(x, y,
        gui_con_screen.char_width, gui_con_screen.char_height, bgcolor);
    
    if (0x20 <= ch && ch <= 0x7e) {
        /* 绘制字符 */
        gui_draw_word(x, y, ch, fontcolor);
    }
}

void con_region_chars(int x0, int y0, int x1, int y1)
{
    int x, y;
    int cx, cy; // 字符坐标

    /* 计算刷新区域 */
    int left, right, top, bottom;
    top = min(y0, y1);
    bottom = max(y0, y1);
    /* 取整对齐 */
    top = top / gui_con_screen.char_height * gui_con_screen.char_height;
    bottom = bottom / gui_con_screen.char_height * gui_con_screen.char_height + gui_con_screen.char_height;
    
    /* 计算行数 */
    int lines = (bottom - top) / gui_con_screen.char_height;
    if (lines > 1) {    /* 选取多行，就需要刷新多行 */
        if (y1 > y0) {  /* 第一个点位于上方，那么，第一行的左边就是第一个点 */
            left = x0;
        } else {    /* 第二个点位于上方，那么，第一行的左边就是第二个点 */
            left = x1;
        }
        /* 第一行从左最小到右边框 */
        right = gui_con_screen.width;
        left = left / gui_con_screen.char_width * gui_con_screen.char_width;
        y = top;
        for (x = left; x < right; x += gui_con_screen.char_width) {
            cx = x / gui_con_screen.char_width;
            cy = y / gui_con_screen.char_height;
            /* 选中某个字符 */
            con_select_char(cx, cy);    
        }
        y += gui_con_screen.char_height;
        if (lines > 2) { /* 大于2行 */
            /* 整行都选择 */
            left = 0;
            right = gui_con_screen.width;
            while (lines > 2) {
                for (x = left; x < right; x += gui_con_screen.char_width) {
                    cx = x / gui_con_screen.char_width;
                    cy = y / gui_con_screen.char_height;
                    /* 选中某个字符 */
                    con_select_char(cx, cy);    
                }
                y += gui_con_screen.char_height;
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
        right = right / gui_con_screen.char_width * gui_con_screen.char_width + gui_con_screen.char_width;
        /* 最后一行 */
        for (x = left; x < right; x += gui_con_screen.char_width) {
            cx = x / gui_con_screen.char_width;
            cy = y / gui_con_screen.char_height;
            /* 选中某个字符 */
            con_select_char(cx, cy);    
        }
    } else {
        /* 最左到最右 */
        left = min(x0, x1);
        right = max(x0, x1);
        left = left / gui_con_screen.char_width * gui_con_screen.char_width;
        right = right / gui_con_screen.char_width * gui_con_screen.char_width + gui_con_screen.char_width;
        /* 向下对齐 */
        for (y = top; y < bottom; y += gui_con_screen.char_height) {
            for (x = left; x < right; x += gui_con_screen.char_width) {
                cx = x / gui_con_screen.char_width;
                cy = y / gui_con_screen.char_height;
                /* 选中某个字符 */
                con_select_char(cx, cy);    
            }
        }
    }
    if (bottom - top > gui_con_screen.char_height) {    /* 选取多行，就需要刷新多行 */
        left = 0;
        right = gui_con_screen.width;
    } else {
        left = min(x0, x1);
        right = max(x0, x1);
        left = left / gui_con_screen.char_width * gui_con_screen.char_width;
        right = right / gui_con_screen.char_width * gui_con_screen.char_width + gui_con_screen.char_width;
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
	char ch;
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
 * con_flush_area - 刷新屏幕区域
 */
void con_flush_area(int x0, int y0, int x1, int y1)
{
    //清空背景
    gui_draw_rect_fill(x0, y0,
        x1 - x0, y1 - y0, gui_con_screen.background_color);

    //把字符全部加载到窗口
    load_char_buffer();

    draw_cursor();
}

/**
 * con_flush_rect - 刷新屏幕区域
 */
void con_flush_rect(int x, int y, int w, int h)
{
    //清空背景
    gui_draw_rect_fill(x, y,
        w, h, gui_con_screen.background_color);

    //把字符全部加载到窗口
    load_char_buffer();

    draw_cursor();
}

/**
 * con_flush2 - 根据鼠标移动位置来刷新
 * 如果鼠标移动的时候跨越字符后，就需要重新刷新，不然就不刷新
 */
void con_flush2(int mx0, int my0, int mx1, int my1)
{
    int cx0, cy0, cx1, cy1;
    cx0 = mx0 / gui_con_screen.char_width;
    cy0 = my0 / gui_con_screen.char_height;
    cx1 = mx1 / gui_con_screen.char_width;
    cy1 = my1 / gui_con_screen.char_height;
    /* 鼠标移动以单个字符为单位进行判断，在单个字符内移动就不刷新 */
    if ((cx0 != cx1) || (cy0 != cy1)) {
        /* 选取一个合适的范围 */
        con_flush_rect(0, min(cy0, cy1) * gui_con_screen.char_height,
            gui_con_screen.width, (abs(cy1 - cy0) + 1)  * gui_con_screen.char_height);        
    }
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
    /* 滚屏时，要把鼠标上次的颜色设置成背景色 */
    gui_mouse.old_color = gui_screen.gui_to_screen_color(gui_con_screen.background_color);
    gui_mouse.show(gui_mouse.x, gui_mouse.y);
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
    
    /* 显示鼠标 */
    gui_mouse.old_color = gui_screen.gui_to_screen_color(gui_con_screen.background_color);
    gui_mouse.show(gui_mouse.x, gui_mouse.y);
}

/*
清除屏幕上的所有东西，
字符缓冲区里面的文字
*/
static void con_clear_area(int x, int y, unsigned int width, unsigned int height)
{
	//清空背景
    gui_draw_rect_fill(x, y,
        width, height, gui_con_screen.background_color);
    /* 显示鼠标 */
    gui_mouse.old_color = gui_screen.gui_to_screen_color(gui_con_screen.background_color);
    gui_mouse.show(gui_mouse.x, gui_mouse.y);
}

void con_set_back_color(GUI_COLOR color)
{
    gui_con_screen.background_color = color;
}

void con_set_font_color(GUI_COLOR color)
{
    gui_con_screen.font_color = color;
}

int cprintf(const char *fmt, ...)
{
	char buf[BUFSIZ];
	va_list arg = (va_list)((char*)(&fmt) + 4); /*4是参数fmt所占堆栈中的大小*/
	vsprintf(buf, fmt, arg);
	
    /* 输出到控制台 */
    gui_con_screen.outs(buf);
	return 0;
}

int con_loop()
{
    gui_event e;
    while (1) {
        /* 获取事件 */
        if (!gui_event_poll(&e)) {
            switch (e.type)
            {
            case GUI_NOEVENT:
                break;
            case GUI_EVENT_MOUSE_BUTTON:
                if (e.button.state == GUI_PRESSED) {    // 按下
                    if (e.button.button == 0) {
                        //printf("[%s] left button pressed.\n", SRV_NAME);
                        clipboard_start_select(e.button.x, e.button.y);

                    } else if (e.button.button == 2) {
                        clipboard_copy_select();
                    } 
                } else {
                    if (e.button.button == 0) {
                        //printf("[%s] left button released.\n", SRV_NAME);
                        clipboard_end_select(e.button.x, e.button.y);
                    }
                }
                
                break;
            case GUI_EVENT_MOUSE_MOTION:
                clipboard_move_select(e.motion.x, e.motion.y);
                
                break;
            case GUI_EVENT_KEY:
                if (e.key.state == GUI_PRESSED) {
                    /* 组合按键 */
                    if (e.key.modify & GUI_KMOD_CTRL) {
                        if (e.key.code == KEY_UP) {
                            scroll_screen(CON_SCROLL_UP, 1, 0, 1);
                            break;
                        } else if (e.key.code == KEY_DOWN) {
                            scroll_screen(CON_SCROLL_DOWN, 1, 0, 1);
                            break;
                        }
                    }
                    /* save key data */
                    fifo_io_put(gui_con_screen.keyfifo, e.key.code);
                    fifo_io_put(gui_con_screen.keyfifo, e.key.modify);
                    break;
                }
                break;
            default:
                break;
            }
        }
    }
    return 0;
}

/**
 * 从控制台获取按键
 * @key: 按键缓冲区
 * @flags: 标志
 * 
 * 成功返回0，失败返回-1
 */
int sys_xcon_get(int *key, int flags)
{
    if (flags > 0) {
        if (fifo_io_len(gui_con_screen.keyfifo) <= 0)   /* no data, return */
            return -1;
    }
    *key++ = fifo_io_get(gui_con_screen.keyfifo);   /* keycode */
    *key   = fifo_io_get(gui_con_screen.keyfifo);   /* modify */
    return 0;
}

/* 获取按键 */
void sys_xcon_clear()
{
    gui_con_screen.clear();
}

/**
 * 往控制台输出数据
 * @buf: 按键缓冲区
 * @len: 标志
 * 
 * 成功返回0，失败返回-1
 */
int sys_xcon_put(void *buf, int len)
{
    char *str = (char *) buf;
    while (*str && len > 0) {
        gui_con_screen.outc(*str);
        str++;
        len--;
    }
    return len;
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
    gui_con_screen.clear_area = con_clear_area;

    gui_con_screen.buflen = gui_con_screen.rows * gui_con_screen.columns * CON_FRAME_NR;

    printf("[GUISRV]: alloc screen buffer rows %d columns %d frames %d size %x!\n", 
        gui_con_screen.rows ,gui_con_screen.columns, CON_FRAME_NR,gui_con_screen.buflen);
    
    gui_con_screen.buffer = kmalloc(gui_con_screen.buflen);
    if (gui_con_screen.buffer == NULL)
        return -1;
    memset(gui_con_screen.buffer, 0, gui_con_screen.buflen);
    gui_con_screen.cur_pos = gui_con_screen.buffer;
    
    gui_draw_rect(0, 0, gui_screen.width, gui_screen.height, COLOR_BLACK);

    init_con_cursor();

    if (init_clipboard() < 0) {
        kfree(gui_con_screen.buffer);
        return -1;
    }

    gui_con_screen.keyfifo = fifo_io_alloc(64);
    if (gui_con_screen.keyfifo == NULL) {
        kfree(gui_con_screen.buffer);
        exit_clipboard();
        return -1;
    }

    /* 显示鼠标光标 */
    gui_mouse.old_color = gui_screen.gui_to_screen_color(gui_con_screen.background_color);
    gui_mouse.show(gui_mouse.x, gui_mouse.y);

    return 0;
}
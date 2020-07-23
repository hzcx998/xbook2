#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <sys/input.h>
#include <sys/trigger.h>
#include <pthread.h>

/// 程序本地头文件
#include <console/cursor.h>
#include <console/console.h>
#include <console/clipboard.h>
#include <console/if.h>
#include <drivers/screen.h>
#include <drivers/keyboard.h>
#include <input/keyboard.h>
#include <input/mouse.h>
#include <graph/rect.h>
#include <graph/text.h>
#include <event/event.h>


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

void con_get_chars(char *buf, int counts, int x, int y)
{
    int cx = x, cy = y;
    char *p = buf;
    while (counts > 0) {
        con_get_char(p, cx, cy);
        cx++;
        if (cx >= screen.columns) {
            cx = 0;
            cy++;
            /*if (cy >= screen.rows) {
                cy = screen.rows;
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

    int x = cx * screen.char_width, y = cy * screen.char_height;
    
    GUI_COLOR bgcolor = (0xffffff - (screen.background_color & 0xffffff)) | (0xff << 24);
    GUI_COLOR fontcolor = (0xffffff - (screen.font_color & 0xffffff)) | (0xff << 24);

    /* 绘制背景 */
    draw_rect_fill(x, y,
        screen.char_width, screen.char_height, bgcolor);
    
    if (0x20 <= ch && ch <= 0x7e) {
        /* 绘制字符 */
        draw_word(x, y, ch, fontcolor);
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
}

void draw_char(char ch)
{
    int x = cursor.x * screen.char_width;
    int y = cursor.y * screen.char_height;
    
    /* 绘制背景 */
    draw_rect_fill(x, y,
        screen.char_width, screen.char_height, screen.background_color);
    
    /* 绘制字符 */
    draw_word(x, y, ch, screen.font_color);
}

void load_char_buffer()
{
	int bx, by, x, y;
	char ch;
	for (by = 0; by < screen.rows; by++) {
		for (bx = 0; bx < screen.columns; bx++) {
			con_get_char(&ch, bx, by);
			
            if (0x20 <= ch && ch <= 0x7e) {
                x = bx * screen.char_width;
			    y = by * screen.char_height;
                /* 绘制字符 */
                draw_word(x, 
                    y, ch, screen.font_color);
            }
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
    draw_rect_fill(0, 0,
        screen.width, screen.height, screen.background_color);

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
    draw_rect_fill(x0, y0,
        x1 - x0, y1 - y0, screen.background_color);

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
    draw_rect_fill(x, y,
        w, h, screen.background_color);

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
    cx0 = mx0 / screen.char_width;
    cy0 = my0 / screen.char_height;
    cx1 = mx1 / screen.char_width;
    cy1 = my1 / screen.char_height;
    /* 鼠标移动以单个字符为单位进行判断，在单个字符内移动就不刷新 */
    if ((cx0 != cx1) || (cy0 != cy1)) {
        /* 选取一个合适的范围 */
        con_flush_rect(0, min(cy0, cy1) * screen.char_height,
            screen.width, (abs(cy1 - cy0) + 1)  * screen.char_height);        
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
        draw_rect_fill(0, 0,
            screen.width, screen.height, screen.background_color);

        //修改显存起始位置
        screen.cur_pos -= screen.columns * lines;
        
		//把字符全部加载到窗口
		load_char_buffer();

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
        draw_rect_fill(0, 0,
            screen.width, screen.height, screen.background_color);

        //修改显存起始位置
        screen.cur_pos += screen.columns * lines;
        
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
    input_mouse.old_color = drv_screen.gui_to_screen_color(screen.background_color);
    input_mouse.show(input_mouse.x, input_mouse.y);
}

/*
显示一个可见字符
*/
void con_ouput_visual(char ch, int x, int y)
{
	if (0x20 <= ch && ch <= 0x7e) {
        /* 绘制字符 */
        draw_word(x, 
            y, ch, screen.font_color);

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
	memset(screen.buffer, 0, screen.buflen);
    
	//修改字符缓冲区指针
	screen.cur_pos = screen.buffer;

	//重置光标
	cursor.x = 0;
	cursor.y = 0;

	//清空背景
    draw_rect_fill(0, 0,
        screen.width, screen.height, screen.background_color);
    //绘制光标
	//draw_cursor();
    
    /* 显示鼠标 */
    input_mouse.old_color = drv_screen.gui_to_screen_color(screen.background_color);
    input_mouse.show(input_mouse.x, input_mouse.y);
}

/*
清除屏幕上的所有东西，
字符缓冲区里面的文字
*/
static void con_clear_area(int x, int y, unsigned int width, unsigned int height)
{
	//清空背景
    draw_rect_fill(x, y,
        width, height, screen.background_color);
    /* 显示鼠标 */
    input_mouse.old_color = drv_screen.gui_to_screen_color(screen.background_color);
    input_mouse.show(input_mouse.x, input_mouse.y);
}

void con_set_back_color(GUI_COLOR color)
{
    screen.background_color = color;
}

void con_set_font_color(GUI_COLOR color)
{
    screen.font_color = color;
}

int cprintf(const char *fmt, ...)
{
	char buf[BUFSIZ];
	va_list arg = (va_list)((char*)(&fmt) + 4); /*4是参数fmt所占堆栈中的大小*/
	vsprintf(buf, fmt, arg);
	
    /* 输出到控制台 */
    screen.outs(buf);
	return 0;
}

int con_loop()
{
    int rd;
    gui_event e;
    char buf[CON_RDPIPE_BUF_LEN];
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
                    xcons_msg_t msg;
                
                    focus_cursor();
                    
                    /* 直接将按键发送给客户端 */
                    
                    msg.type = 0;
                    msg.data = e.key.code;
                    msg.ctrl = e.key.modify;
                    guisrv_if_send_msg(&msg);
                    break;
                
                }
                break;
            default:
                break;
            }
        }
        memset(buf, 0, CON_RDPIPE_BUF_LEN);
        /* 获取管道信息 */
        if ((rd = guisrv_if_recv_data(buf, CON_RDPIPE_BUF_LEN)) > 0) {
            //printf("[%s] recv data %d %x\n", SRV_NAME, rd, *buf);
            buf[CON_RDPIPE_BUF_LEN - 1] = 0;
            screen.outs(buf);
        }
        
    }
    return 0;
}

int init_con_screen()
{
    screen.width = drv_screen.width;
    screen.height = drv_screen.height;
    screen.char_width = CON_CHAR_WIDTH;
    screen.char_height = CON_CHAR_HEIGHT;
    screen.rows = screen.height / screen.char_height;
    screen.columns = screen.width / screen.char_width;
    screen.columns_width = screen.columns * screen.char_width;
    screen.rows_height = screen.rows * screen.char_height;

    screen.codepage = CON_CODEPAGE;
    screen.background_color = CON_SCREEN_BG_COLOR;
    screen.font_color = CON_SCREEN_FONT_COLOR;
    screen.mouse_color = CON_MOUSE_COLOR;
    screen.outc = con_out_char;
    screen.outs = con_out_str;
    screen.clear = con_clear;
    screen.clear_area = con_clear_area;

    screen.buflen = screen.rows * screen.columns * CON_FRAME_NR;

    printf("[GUISRV]: alloc screen buffer rows %d columns %d frames %d size %x!\n", 
        screen.rows ,screen.columns, CON_FRAME_NR,screen.buflen);
    
    screen.buffer = malloc(screen.buflen);
    if (screen.buffer == NULL)
        return -1;
    memset(screen.buffer, 0, screen.buflen);
    screen.cur_pos = screen.buffer;

    init_con_cursor();

    if (init_clipboard() < 0) {
        free(screen.buffer);
        return -1;
    }

    /* 显示鼠标光标 */
    input_mouse.old_color = drv_screen.gui_to_screen_color(screen.background_color);
    input_mouse.show(input_mouse.x, input_mouse.y);
    
    /* 开一个线程来接收服务 */
    pthread_t thread_echo;
    int retval = pthread_create(&thread_echo, NULL, guisrv_echo_thread, NULL);
    if (retval == -1) {
        exit_clipboard();
        return -1;
    }
        

    return 0;
}
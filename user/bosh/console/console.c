#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <math.h>
#include <gapi.h>
#include <sys/input.h>
#include <sys/trigger.h>

#include <sh_console.h>
#include <sh_cursor.h>
#include <sh_window.h>
#include <sh_clipboard.h>
#include <sh_cmd.h>

/* 控制台全局变量 */
con_screen_t con_screen;

void con_set_chars(char ch, int counts, int x, int y)
{
    int cx = x, cy = y;
    while (counts > 0) {
        con_set_char(ch, cx, cy);
        cx++;
        if (cx >= con_screen.columns) {
            cx = 0;
            cy++;
            /*if (cy >= con_screen.rows) {
                cy = con_screen.rows;
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
        if (cx >= con_screen.columns) {
            cx = 0;
            cy++;
            /*if (cy >= con_screen.rows) {
                cy = con_screen.rows;
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

    int x = cx * con_screen.char_width, y = cy * con_screen.char_height;
    
    g_color_t bgcolor = (0xffffff - (con_screen.background_color & 0xffffff)) | (0xff << 24);
    g_color_t fontcolor = (0xffffff - (con_screen.font_color & 0xffffff)) | (0xff << 24);

    /* 绘制背景 */
    sh_window_rect_fill(x, y,
        con_screen.char_width, con_screen.char_height, bgcolor);
    
    if (0x20 <= ch && ch <= 0x7e) {
        /* 绘制字符 */
        sh_window_char(x, y, ch, fontcolor);
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
    top = top / con_screen.char_height * con_screen.char_height;
    bottom = bottom / con_screen.char_height * con_screen.char_height + con_screen.char_height;
    
    /* 计算行数 */
    int lines = (bottom - top) / con_screen.char_height;
    if (lines > 1) {    /* 选取多行，就需要刷新多行 */
        if (y1 > y0) {  /* 第一个点位于上方，那么，第一行的左边就是第一个点 */
            left = x0;
        } else {    /* 第二个点位于上方，那么，第一行的左边就是第二个点 */
            left = x1;
        }
        /* 第一行从左最小到右边框 */
        right = con_screen.width;
        left = left / con_screen.char_width * con_screen.char_width;
        y = top;
        for (x = left; x < right; x += con_screen.char_width) {
            cx = x / con_screen.char_width;
            cy = y / con_screen.char_height;
            /* 选中某个字符 */
            con_select_char(cx, cy);    
        }
        y += con_screen.char_height;
        if (lines > 2) { /* 大于2行 */
            /* 整行都选择 */
            left = 0;
            right = con_screen.width;
            while (lines > 2) {
                for (x = left; x < right; x += con_screen.char_width) {
                    cx = x / con_screen.char_width;
                    cy = y / con_screen.char_height;
                    /* 选中某个字符 */
                    con_select_char(cx, cy);    
                }
                y += con_screen.char_height;
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
        right = right / con_screen.char_width * con_screen.char_width + con_screen.char_width;
        /* 最后一行 */
        for (x = left; x < right; x += con_screen.char_width) {
            cx = x / con_screen.char_width;
            cy = y / con_screen.char_height;
            /* 选中某个字符 */
            con_select_char(cx, cy);    
        }
    } else {
        /* 最左到最右 */
        left = min(x0, x1);
        right = max(x0, x1);
        left = left / con_screen.char_width * con_screen.char_width;
        right = right / con_screen.char_width * con_screen.char_width + con_screen.char_width;
        /* 向下对齐 */
        for (y = top; y < bottom; y += con_screen.char_height) {
            for (x = left; x < right; x += con_screen.char_width) {
                cx = x / con_screen.char_width;
                cy = y / con_screen.char_height;
                /* 选中某个字符 */
                con_select_char(cx, cy);    
            }
        }
    }
    if (bottom - top > con_screen.char_height) {    /* 选取多行，就需要刷新多行 */
        left = 0;
        right = con_screen.width;
    } else {
        left = min(x0, x1);
        right = max(x0, x1);
        left = left / con_screen.char_width * con_screen.char_width;
        right = right / con_screen.char_width * con_screen.char_width + con_screen.char_width;
    }
    /* 只刷新一次 */
    sh_window_update(left, top, right, bottom);
}

void gui_draw_char(char ch)
{
    int x = cursor.x * con_screen.char_width;
    int y = cursor.y * con_screen.char_height;
    //printf("cursor: x=%d, y=%d\n", cursor.x, cursor.y);
    /* 绘制背景 */
    sh_window_rect_fill(x, y,
        con_screen.char_width, con_screen.char_height, con_screen.background_color);
    
    /* 绘制字符 */
    sh_window_char(x, y, ch, con_screen.font_color);

    /* 刷新光标 */
    sh_window_update(x, y, 
        x +con_screen.char_width, y +con_screen.char_height);
}

void load_char_buffer()
{
	int bx, by, x, y;
	char ch;
	for (by = 0; by < con_screen.rows; by++) {
		for (bx = 0; bx < con_screen.columns; bx++) {
			con_get_char(&ch, bx, by);
			
            if (0x20 <= ch && ch <= 0x7e) {
                x = bx * con_screen.char_width;
			    y = by * con_screen.char_height;
                /* 绘制字符 */
                sh_window_char(x, 
                    y, ch, con_screen.font_color);
            }
		}
    }
}

static int can_scroll_up()
{
	if (con_screen.cur_pos > con_screen.buffer) {
		return 1;
	}
	return 0;
}

static int can_scroll_down()
{
	if (con_screen.cur_pos < (con_screen.buffer + con_screen.buflen) - \
        con_screen.rows * con_screen.columns) {
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
    sh_window_rect_fill(0, 0,
        con_screen.width, con_screen.height, con_screen.background_color);

    //把字符全部加载到窗口
    load_char_buffer();

    /* 刷新全部 */
    sh_window_update(0, 0,
       con_screen.width,con_screen.height);

    draw_cursor();
}


/**
 * con_flush_area - 刷新屏幕区域
 */
void con_flush_area(int x0, int y0, int x1, int y1)
{
    //清空背景
    sh_window_rect_fill(x0, y0,
        x1 - x0, y1 - y0, con_screen.background_color);

    //把字符全部加载到窗口
    load_char_buffer();

    /* 刷新全部 */
    sh_window_update(x0, y0, x1, y1);

    draw_cursor();
}

/**
 * con_flush_rect - 刷新屏幕区域
 */
void con_flush_rect(int x, int y, int w, int h)
{
    //清空背景
    sh_window_rect_fill(x, y,
        w, h, con_screen.background_color);

    //把字符全部加载到窗口
    load_char_buffer();

    /* 刷新全部 */
    sh_window_update(x, y, x + w, y + h);

    draw_cursor();
}

/**
 * con_flush2 - 根据鼠标移动位置来刷新
 * 如果鼠标移动的时候跨越字符后，就需要重新刷新，不然就不刷新
 */
void con_flush2(int mx0, int my0, int mx1, int my1)
{
    int cx0, cy0, cx1, cy1;
    cx0 = mx0 / con_screen.char_width;
    cy0 = my0 / con_screen.char_height;
    cx1 = mx1 / con_screen.char_width;
    cy1 = my1 / con_screen.char_height;
    /* 鼠标移动以单个字符为单位进行判断，在单个字符内移动就不刷新 */
    if ((cx0 != cx1) || (cy0 != cy1)) {
        /* 选取一个合适的范围 */
        con_flush_rect(0, min(cy0, cy1) * con_screen.char_height,
            con_screen.width, (abs(cy1 - cy0) + 1)  * con_screen.char_height);        
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
        sh_window_rect_fill(0, 0,
            con_screen.width, con_screen.height, con_screen.background_color);

        //修改显存起始位置
        con_screen.cur_pos -= con_screen.columns * lines;
        
		//把字符全部加载到窗口
		load_char_buffer();

        /* 刷新全部 */
        sh_window_update(0, 0,
           con_screen.width,con_screen.height);
        if (cursorx)
            cursor.x = 0;
		if (cursory) {
            cursor.y += lines;
            if (cursor.y > con_screen.rows - 1) {
                //cursor.y = con_screen.rows - 1;
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
        sh_window_rect_fill(0, 0,
            con_screen.width, con_screen.height, con_screen.background_color);

        //修改显存起始位置
        con_screen.cur_pos += con_screen.columns * lines;
        
		//把字符全部加载到窗口
		load_char_buffer();
        /* 刷新全部 */
        sh_window_update(0, 0,
           con_screen.width,con_screen.height);
        
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
        sh_window_char(x, 
            y, ch, con_screen.font_color);
            
        /* 刷新光标 */
        sh_window_update(x, y, 
            x  + con_screen.char_width, y  + con_screen.char_height);
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
	memset(con_screen.buffer, 0, con_screen.buflen);
    
	//修改字符缓冲区指针
	con_screen.cur_pos = con_screen.buffer;

	//重置光标
	cursor.x = 0;
	cursor.y = 0;

	//清空背景
    sh_window_rect_fill(0, 0,
        con_screen.width, con_screen.height, con_screen.background_color);
    
    /* 刷新全部 */
    sh_window_update(0, 0,
       con_screen.width,con_screen.height);

    //绘制光标
	draw_cursor();
    
}
#if 0
/*
清除屏幕上的所有东西，
字符缓冲区里面的文字
*/
static void con_fresh()
{
    
	//清空字符缓冲区
	memset(con_screen.buffer, 0, con_screen.buflen);
    
	//修改字符缓冲区指针
	con_screen.cur_pos = con_screen.buffer;

	//重置光标
	cursor.x = 0;
	cursor.y = 0;

	//清空背景
    sh_window_rect_fill(0, 0,
        con_screen.width, con_screen.height, con_screen.background_color);
    
    /* 刷新全部 */
    sh_window_update(0, 0,
        con_screen.width, con_screen.height);
    //绘制光标
	//draw_cursor();
    
}
#endif

/*
清除屏幕上的所有东西，
字符缓冲区里面的文字
*/
static void con_clear_area(int x, int y, unsigned int width, unsigned int height)
{
	//清空背景
    sh_window_rect_fill(x, y,
        width, height, con_screen.background_color);
    /* 刷新全部 */
    sh_window_update(x, y,
        x + width, y + height);
}

void con_set_back_color(g_color_t color)
{
    con_screen.background_color = color;
}

void con_set_font_color(g_color_t color)
{
    con_screen.font_color = color;
}

int con_printf(const char *fmt, ...)
{
	char buf[BUFSIZ];
	va_list arg = (va_list)((char*)(&fmt) + 4); /*4是参数fmt所占堆栈中的大小*/
	vsprintf(buf, fmt, arg);
	
    /* 输出到控制台 */
    con_screen.outs(buf);
	return 0;
}

void con_putchar(char ch)
{
    con_screen.outc(ch);
}

/**
 * 从窗口获取一个按键
 * 
 */
int con_get_key(int kcode, int kmod)
{
    //printf("kcode:%c kmod:%x\n", kcode, kmod);
    int j;
    char *q;
    int cx, cy;
    /* 组合按键 */
    if (kmod & GKMOD_CTRL) {
        if (kcode == GK_UP) {
            scroll_screen(CON_SCROLL_UP, 1, 0, 1);
            return 0;
        } else if (kcode == GK_DOWN) {
            scroll_screen(CON_SCROLL_DOWN, 1, 0, 1);
            return 0;
        }
    }
    /* 过滤一些按键 */
    switch (kcode) {
    case GK_UP:
        clipboard_break_select();
        focus_cursor();
        /* 选择上一个的命令 */
        //move_cursor_off(0, -1);
        cmd_buf_select(-1);
        break;
    case GK_DOWN:
        clipboard_break_select();
        focus_cursor();
        /* 选择下一个命令 */
        //move_cursor_off(0, 1);
        cmd_buf_select(1);
        break;
    case GK_NUMLOCK:
    case GK_CAPSLOCK:
    case GK_SCROLLOCK:
    case GK_RSHIFT:
    case GK_LSHIFT:
    case GK_RCTRL:
    case GK_LCTRL:
    case GK_RALT:
    case GK_LALT:
        break;
    case GK_LEFT:
        clipboard_break_select();
        focus_cursor();
        if(cmdman->cmd_pos > cmdman->cmd_line){
            --cmdman->cmd_pos;
            move_cursor_off(-1, 0);
        }
        break;
    case GK_RIGHT:
        clipboard_break_select();
        focus_cursor();
        if ((cmdman->cmd_pos - cmdman->cmd_line) < cmdman->cmd_len) {
            move_cursor_off(1, 0);
            ++cmdman->cmd_pos;
        }
        break;
    case GK_ENTER:
        clipboard_break_select();
        focus_cursor();
        move_cursor_off(cmdman->cmd_len - (cmdman->cmd_pos - cmdman->cmd_line), 0);
        con_screen.outc('\n');
        cmdman->cmd_line[cmdman->cmd_len] = 0;
        /* 发送给命令行 */
        cmdline_check();   /* 执行命令 */
        print_prompt();     /* 打印命令提示符 */
    case GK_BACKSPACE:
        clipboard_break_select();
        focus_cursor();
        if(cmdman->cmd_pos > cmdman->cmd_line){
            if (cmdman->cmd_pos >= cmdman->cmd_line + cmdman->cmd_len) { /* 在末尾 */
                --cmdman->cmd_pos;
                *cmdman->cmd_pos = '\0';
                con_screen.outc('\b');
            
            } else {    /* 在中间 */
                --cmdman->cmd_pos;
                /* 获取现在的位置 */
                get_cursor(&cx, &cy);
                /* 去除后面的字符 */
                j = strlen(cmdman->cmd_pos);
                while (j--)
                    con_screen.outc(' ');
                /* 移动回到原来的位置 */
                move_cursor(cx, cy);

                for (q = cmdman->cmd_pos; q < cmdman->cmd_line + cmdman->cmd_len; q++) {
                    *q = *(q + 1);
                }
                /* 向前移动一个位置 */
                move_cursor_off(-1, 0);
                /* 获取现在的位置 */
                get_cursor(&cx, &cy);
                con_screen.outs(cmdman->cmd_pos);
                /* 移动回到原来的位置 */
                move_cursor(cx, cy);
            }
            cmdman->cmd_len--;
        }
        break;
    default:
        clipboard_break_select();
        focus_cursor();
        /* 取消框选 */
        if (cmdman->cmd_pos >= cmdman->cmd_line + cmdman->cmd_len) { /* 在末尾 */
            *cmdman->cmd_pos = kcode;

            con_screen.outc(kcode);
            
        } else { 
            /* 把后面的数据向后移动一个单位 ab1c|def */
            for (q = cmdman->cmd_line + cmdman->cmd_len; q > cmdman->cmd_pos; q--) {
                *q = *(q - 1);
                *(q - 1) = '\0';
            }
            
            get_cursor(&cx, &cy);

            /* 发送给命令行 */
            *cmdman->cmd_pos = kcode;
            con_screen.outs(cmdman->cmd_pos);
            move_cursor(cx + 1, cy);
        }
        cmdman->cmd_pos++;
        cmdman->cmd_len++;
        break;
    }
    return 0;
}
extern int shell_child_pid;
extern int shell_child_key;

/**
 * 传输按键
 * 
 * 在调用子进程期间执行
 */
int con_xmit_key(int kcode, int kmod)
{
    //printf("kcode:%c kmod:%x\n", kcode, kmod);
    /* 组合按键 */
    if (kmod & GKMOD_CTRL) {
        if (kcode == GK_C || kcode == GK_c) {
            if (shell_child_pid > 0)
                triggeron(TRIGLSOFT, shell_child_pid);
                
            return 0;   /* 特殊按键处理 */
        }
    }
    /* 过滤一些按键 */
    switch (kcode) {
    case GK_NUMLOCK:
    case GK_CAPSLOCK:
    case GK_SCROLLOCK:
    case GK_RSHIFT:
    case GK_LSHIFT:
    case GK_RCTRL:
    case GK_LCTRL:
    case GK_RALT:
    case GK_LALT:
        return 0;   /* 特殊按键处理 */
    default:
        break;
    }
    shell_child_key = kcode;
    return 1;
}

int init_console()
{
    if (init_window() < 0) {
        printf("bosh: init window failed!\n");
        return -1;
    }

    uint32_t width, height;
    if (sh_window_size(&width, &height) < 0) {
        printf("bosh: get window size failed!\n");
        goto label_exit_window;
    }
    con_screen.width = width;
    con_screen.height = height;
    con_screen.char_width = CON_CHAR_WIDTH;
    con_screen.char_height = CON_CHAR_HEIGHT;
    con_screen.rows = con_screen.height / con_screen.char_height;
    con_screen.columns = con_screen.width / con_screen.char_width;
    con_screen.columns_width = con_screen.columns * con_screen.char_width;
    con_screen.rows_height = con_screen.rows * con_screen.char_height;

    con_screen.codepage = CON_CODEPAGE;
    con_screen.background_color = CON_SCREEN_BG_COLOR;
    con_screen.font_color = CON_SCREEN_FONT_COLOR;
    con_screen.mouse_color = CON_MOUSE_COLOR;
    con_screen.outc = con_out_char;
    con_screen.outs = con_out_str;
    con_screen.clear = con_clear;
    con_screen.clear_area = con_clear_area;
    con_screen.flush = con_flush;
    con_screen.buflen = con_screen.rows * con_screen.columns * CON_FRAME_NR;

    printf("[gui]: alloc console screen buffer rows %d columns %d frames %d size %x!\n", 
        con_screen.rows ,con_screen.columns, CON_FRAME_NR,con_screen.buflen);
    
    con_screen.buffer = malloc(con_screen.buflen);
    if (con_screen.buffer == NULL) {
        printf("bosh: init screen buffer failed!\n");
        goto label_exit_window;
    }
    memset(con_screen.buffer, 0, con_screen.buflen);
    con_screen.cur_pos = con_screen.buffer;
    
    sh_window_rect_fill(0, 0, con_screen.width, con_screen.height, GC_BLACK);

    init_con_cursor();

    if (init_clipboard() < 0) {
        printf("bosh: init clipboard failed!\n");
        goto label_free_buffer;
    }

    if (init_cmd_man() < 0) {
        printf("bosh: init cmd failed!\n");
        goto label_exit_clipboard;
    }

    return 0;

label_exit_clipboard:
    exit_clipboard();
label_free_buffer:
    free(con_screen.buffer);
label_exit_window:
    exit_window();
    return -1;
}

int exit_console()
{
    exit_cmd_man();
    exit_clipboard();
    free(con_screen.buffer);
    exit_window();
    return 0;
}

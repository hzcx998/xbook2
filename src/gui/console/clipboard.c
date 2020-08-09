#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stddef.h>

#include <gui/console/console.h>
#include <gui/console/cursor.h>
#include <gui/console/clipboard.h>
#include <gui/mouse.h>
#include <gui/screen.h>

#include <xbook/kmalloc.h>

clipboard_t clipboard;

int init_clipboard()
{
    clipboard.click_x = -1;
    clipboard.click_y = -1;
    clipboard.start_x = -1;
    clipboard.start_y = -1;

    clipboard.mx = -1;
    clipboard.my = -1;
    clipboard.state = CLIPBOARD_AVALI;

    clipboard.buflen = gui_con_screen.rows * gui_con_screen.columns;
    clipboard.buf = kmalloc(clipboard.buflen);
    if (clipboard.buf == NULL) 
        return -1;
    memset(clipboard.buf, 0, clipboard.buflen);

    return 0;
}

void exit_clipboard()
{
    kfree(clipboard.buf);
}
void clipboard_start_select(int x, int y)
{
    /* 开始框选 */
    con_flush();
    clipboard.click_x = x;
    clipboard.click_y = y;
    clipboard.start_x = x;
    clipboard.start_y = y;
    clipboard.mx = x;
    clipboard.my = y;
    con_region_chars(x, y, x, y);
    clipboard.state = CLIPBOARD_START;
}

void clipboard_end_select()
{
    clipboard.click_x = -1;
    clipboard.click_y = -1;
    clipboard.state = CLIPBOARD_END;
}

void clipboard_move_select(int x, int y)
{
    /* 持续框选 */
    if (clipboard.click_x >= 0 && clipboard.click_y >= 0) {
        /* 刷新范围内的窗口 */
        con_flush2(clipboard.mx, clipboard.my, x, y);
        clipboard.mx = x;
        clipboard.my = y;
        /* 选择选取 */
        con_region_chars(clipboard.click_x, clipboard.click_y, x, y);
    }
}

void clipboard_break_select()
{
    con_flush();
    clipboard.click_x = -1;
    clipboard.click_y = -1;
    
    /* 被打断时，要把鼠标上次的颜色设置成背景色 */
    gui_mouse.old_color = gui_screen.gui_to_screen_color(gui_con_screen.background_color);
    gui_mouse.show(gui_mouse.x, gui_mouse.y);
}

/**
 * clipboard_copy_select - 复制选中的内容
 * 
 */
void clipboard_copy_select()
{
    /* 复制选中的内容 */
    if (clipboard.state == CLIPBOARD_AVALI) {   /* 空闲状态，把剪切板中的内容复制到命令行 */
        focus_cursor();
        int buflen = strlen(clipboard.buf);
        if (buflen > 0) {
             
            /* 对剪切板进行数据复制操作 */

            //cmdline_set(clipboard.buf, buflen);
        }
    } else if (clipboard.state == CLIPBOARD_END) { /* 选择完毕，复制选中的内容 */
        /*printf("start: %d, %d end: %d, %d\n", clipboard.start_x, clipboard.start_y,
            clipboard.mx, clipboard.my);
        */
        int x0 = clipboard.start_x, y0 = clipboard.start_y;
        int x1 = clipboard.mx, y1 = clipboard.my;
        int x, y;
        int start_cx = -1, start_cy = -1;   /* 尚未计算字符起始位置 */
        int cx, cy;   /* 尚未计算字符起始位置 */

        int counts = 0; /* 字符数量 */

        /* 计算刷新区域 */
        int left, right, top, bottom;
        top = min(y0, y1);
        bottom = max(y0, y1);
        /* 取整对齐 */
        top = top / gui_con_screen.char_height * gui_con_screen.char_height;
        bottom = bottom / gui_con_screen.char_height * gui_con_screen.char_height + gui_con_screen.char_height;
        
        /* 计算行数 */
        int lines = (bottom - top) / gui_con_screen.char_height;
        //printf("lines=%d\n", lines);

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
                /* 记录字符起始位置 */
                if (start_cx == -1 && start_cy == -1) {
                    start_cx = cx;
                    start_cy = cy;
                }
                counts++;
            }
            y += gui_con_screen.char_height;
            if (lines > 2) { /* 大于2行 */
                /* 整行都选择 */
                left = 0;
                right = gui_con_screen.width;
                while (lines > 2) {
                    for (x = left; x < right; x += gui_con_screen.char_width) {
                        counts++;
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
                counts++;  
            }
        } else {
            /* 最左到最右 */
            left = min(x0, x1);
            right = max(x0, x1);
            left = left / gui_con_screen.char_width * gui_con_screen.char_width;
            right = right / gui_con_screen.char_width * gui_con_screen.char_width + gui_con_screen.char_width;
            
            y = top;
            for (x = left; x < right; x += gui_con_screen.char_width) {
                cx = x / gui_con_screen.char_width;
                cy = y / gui_con_screen.char_height;
                /* 记录字符起始位置 */
                if (start_cx == -1 && start_cy == -1) {
                    start_cx = cx;
                    start_cy = cy;
                }
                counts++;   
            }
        }
        //printf("cx=%d, cy=%d, counts=%d\n", start_cx, start_cy, counts);
        
        int buflen = min(clipboard.buflen, counts);
        memset(clipboard.buf, 0, clipboard.buflen);
        /* 获取字符 */
        con_get_chars(clipboard.buf, buflen, start_cx, start_cy);

        clipboard.state = CLIPBOARD_AVALI;
        /* 重置选区 */
        clipboard_break_select();
    }
}

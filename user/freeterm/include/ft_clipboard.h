#ifndef _FREETERM_CLIPBOARD_H
#define _FREETERM_CLIPBOARD_H

#define CLIPBOARD_AVALI 0
#define CLIPBOARD_START 1
#define CLIPBOARD_END   2

typedef struct {
    int click_x, click_y;       /* 点击时的位置 */
    int start_x, start_y;       /* 点击时的位置 */
    int mx, my;                 /* 鼠标移动时的位置 */
    int state;                  /* 状态 */
    char *buf;                  /* 字符缓冲区 */
    int buflen;                 /* 字符缓冲区长度 */
} clipboard_t;

extern clipboard_t clipboard;

int init_clipboard();
void exit_clipboard();
void clipboard_start_select(int x, int y);
void clipboard_end_select();
void clipboard_move_select(int x, int y);
void clipboard_break_select();
void clipboard_copy_select();

#endif /* _FREETERM_CLIPBOARD_H */
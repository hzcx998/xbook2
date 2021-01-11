#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <uview.h>
#include <assert.h>

#include <pthread.h>
#include <dotfont.h>

void win_thread();

int open_desktop()
{
    /* 创建桌面视图 */
    int screen_fd = uview_open(32, 32);
    if (screen_fd < 0) {
        printf("open view dev failed!\n");
        return -1;
    }
    int screen_w, screen_h;
    uview_set_type(screen_fd, UVIEW_TYPE_FIXED);
    /* 获取并调整桌面大小 */
    uview_get_screensize(screen_fd, &screen_w, &screen_h);
    uview_resize(screen_fd, 0, 0, screen_w, screen_h);
    // 调整大小后重绘
    uview_msg_t msg;
    int get_msg_done = 0;
    if (!uview_get_msg(screen_fd, &msg)) {
        switch (uview_msg_get_type(&msg)) {
        case UVIEW_MSG_RESIZE:
            {
                screen_w = uview_msg_get_resize_width(&msg);
                screen_h = uview_msg_get_resize_height(&msg);
                get_msg_done = 1;
            }
            break;
        default:
            break;
        }
    }
    if (!get_msg_done) {
        close(screen_fd);
        return -1;
    }
    /* 加载背景图片 */
    int iw, ih, ic;
    unsigned char *ibuf = uview_load_image("/background/lack.jpg", &iw, &ih, &ic);
    if (!ibuf) {
        close(screen_fd);
        return -1;
    }
    // 绘制桌面
    uint32_t *screen_bits = malloc(screen_w * screen_h * sizeof(uview_color_t));
    assert(screen_bits);
    uview_bitmap_t screen_vbmp;
    uview_bitmap_init(&screen_vbmp, screen_w, screen_h, screen_bits);
    /* 调整大小为屏幕大小 */
    uview_resize_image(ibuf, iw, ih, 
        (unsigned char *) screen_bits, screen_w, screen_h, 4);
    /* 释放图片的原始数据 */
    free(ibuf);
    uview_bitblt_update(screen_fd, 0, 0, &screen_vbmp);
    /* 释放位图 */
    free(screen_bits);

    // 需要先显示桌面
    uview_show(screen_fd);

    // 创建子进程
    pid_t pid = fork();
    if (!pid) {
        win_thread();
        exit(-1);
    }

    // 消息循环
    while (1) {
        if (uview_get_msg(screen_fd, &msg)) {

        }
    }
    return 0;
}


int main(int argc, char *argv[]) 
{
    
    if (open_desktop() < 0)
        return -1;
    return 0;
}

#define WIN_W 320
#define WIN_H 240

dotfont_lib_t dotflib;

void dotfont_char_to_bitmap(dotfont_t *font, char ch,
        uview_bitmap_t *bmp, int x, int y, uint32_t color)
{
    uint8_t *addr = dotfont_get_addr(font, ch);
    assert(addr);
    int i, j;
	uint8_t d /* data */;
	for (i = 0; i < 16; i++) {
		d = addr[i];
        for (j = (8 - 1); j >= 0; j--) {
            if ((d & (1 << j))) {
                uview_bitmap_putpixel(bmp, x + (8 - 1) - j, y + i, color);
            }
        }
    }
}

int dotfont_text_to_bitmap(char *text, uint32_t color, char *family, uview_bitmap_t *bmp, int x, int y)
{
    dotfont_t *stdfnt =  dotfont_find(&dotflib, family);
    if (!stdfnt)
        return -1;
    char *p = text;
    int _x = x, _y = y;
    while (*p) {
        switch (*p)
        {
        case '\b':
            _x -= dotfont_get_char_width(stdfnt);
            dotfont_char_to_bitmap(stdfnt, ' ', bmp, _x, _y, color);
            break;
        case '\n':
            _x = x;
            _y += dotfont_get_char_height(stdfnt);
            break;
        default:
            dotfont_char_to_bitmap(stdfnt, *p, bmp, _x, _y, color);
            _x += dotfont_get_char_width(stdfnt);
            break;
        }
        p++;
    }
    return 0;
}

int uview_dotfont_text(int uview, int x, int y, uint32_t w, uint32_t h, char *text, uint32_t color, char *family)
{
    if (uview < 0)
        return -1;
    uview_bitmap_t *bmp = uview_bitmap_create(w, h);
    assert(bmp);
    dotfont_text_to_bitmap(text, color, family, bmp, 0, 0);
    uview_bitblt_update(uview, x, y, bmp);
    uview_bitmap_destroy(bmp);
    return 0;
}

void win_thread()
{
    int win_fd = uview_open(WIN_W, WIN_H);
    if (win_fd < 0) {
        printf("open view dev failed!\n");
        return;
    }
    uview_set_type(win_fd, UVIEW_TYPE_WINDOW);
    uview_show(win_fd);

    dotfont_init(&dotflib);

    uview_bitmap_t *bmp = uview_bitmap_create(WIN_W, WIN_H);
    assert(bmp);

    dotfont_t *stdfnt =  dotfont_get_current(&dotflib);
    uview_bitmap_t *fbmp = uview_bitmap_create(8, 16);
    dotfont_char_to_bitmap(stdfnt, 'A', fbmp, 0, 0, UVIEW_RED);
    uview_bitblt_update(win_fd, 0, 0, fbmp);
    uview_bitmap_destroy(fbmp);
    
    fbmp = uview_bitmap_create(400, 300);
    dotfont_text_to_bitmap("hello, world!\nabc\bdef", UVIEW_RED, DOTF_STANDARD_NAME,
        fbmp, 0, 0);
    uview_bitblt_update(win_fd, 100, 100, fbmp);
    uview_bitmap_destroy(fbmp);
    
    uview_dotfont_text(win_fd, 100, 20, 400, 300, "hello, world!\nabc\bdef", UVIEW_RED, DOTF_STANDARD_NAME);

    uview_msg_t msg;
    while (1) {
        if (uview_get_msg(win_fd, &msg) < 0) {
            continue;
        }

        switch (uview_msg_get_type(&msg))
        {
        case UVIEW_MSG_ACTIVATE:
            uview_bitmap_rectfill(bmp, 0, 0, WIN_W, WIN_H, UVIEW_BLUE);
            uview_bitblt_update(win_fd, 0, 0, bmp);
            printf("activate\n");
            break;
        case UVIEW_MSG_INACTIVATE:
            uview_bitmap_rectfill(bmp, 0, 0, WIN_W, WIN_H, UVIEW_GREEN);
            uview_bitblt_update(win_fd, 0, 0, bmp);
            printf("inactivate\n");
            break;
        default:
            break;
        }
    }    
}
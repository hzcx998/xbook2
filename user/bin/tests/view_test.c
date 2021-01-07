#include "test.h"

#include <uview.h>


int view_test(int argc, char *argv[])
{
    printf("start view test\n");

    /* 创建桌面视图 */
    int screen_fd = uview_open(32, 32);
    if (screen_fd < 0) {
        printf("open view dev failed!\n");
        return -1;
    }
    int screen_w, screen_h;
    uview_set_type(screen_fd, UVIEW_TYPE_FIXED);
    uview_get_screensize(screen_fd, &screen_w, &screen_h);
    uview_resize(screen_fd, 0, 0, screen_w, screen_h);
    // 调整大小后重绘
    uview_msg_t msg;
    if (!uview_get_msg(screen_fd, &msg)) {
        switch (uview_msg_get_type(&msg)) {
        case UVIEW_MSG_RESIZE:
            {
                // 绘制桌面
                uint32_t *screen_bits = malloc(screen_w * screen_h * sizeof(uview_color_t));
                assert(screen_bits);
                uview_bitmap_t screen_vbmp;
                uview_bitmap_init(&screen_vbmp, screen_w, screen_h, screen_bits);
                uview_bitmap_rectfill(&screen_vbmp, 0, 0, 
                    screen_vbmp.width, screen_vbmp.height, 
                    UVIEW_GREEN);
                uview_bitblt_update(screen_fd, 0, 0, &screen_vbmp);
                free(screen_bits);
            }
            break;
        default:
            break;
        }
    }
    // 需要先显示桌面
    uview_show(screen_fd);

    // 加载鼠标样式，鼠标样式位图列表，一次性完成加载。


    // 再创建其他的视图
    int fd = uview_open(640, 480);
    if (fd < 0) {
        printf("open view dev failed!\n");
        return -1;
    }
    uview_set_pos(fd, 200, 100);
    uview_set_type(fd, UVIEW_TYPE_WINDOW);
    //uview_set_unresizable(fd);
    //uview_set_unmoveable(fd);
    uview_show(fd);

    int winw = 640, winh = 480;
    uint32_t *colors = malloc(winw * winh * 4);
    uview_bitmap_t vbmp;
    
    int i = 0; 
    //uview_msg_t msg;
    while (1) {
        uview_bitmap_init(&vbmp, winw, winh, colors);
        uview_bitmap_rectfill(&vbmp, 0, 0, vbmp.width, vbmp.height, 
            (0xff << 24) + (((i * 20) & 0xff) << 16) + \
            (((i * 10) & 0xff) << 8) + ((i * 5) & 0xff));
        uview_bitblt_update(fd, 0, 0, &vbmp);
        i++;
        if (!uview_get_msg(fd, &msg)) {
            switch (uview_msg_get_type(&msg)) {
            case UVIEW_MSG_RESIZE:
                free(colors);
                winw = uview_msg_get_resize_width(&msg);
                winh = uview_msg_get_resize_height(&msg);
                colors = malloc(winw * winh * 4);
                break;
            default:
                break;
            }
        }
        
    }

    printf("sleep 3s!\n");
    sleep(3);
    printf("close view!\n");
    close(fd);
    return 0;
}
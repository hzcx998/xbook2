#include "test.h"

#include <uview.h>


int view_test(int argc, char *argv[])
{
    printf("start view test\n");

    int fd = uview_open(640, 480);
    if (fd < 0) {
        printf("open view dev failed!\n");
        return -1;
    }
    uview_set_pos(fd, 200, 100);
    uview_set_type(fd, UVIEW_TYPE_WINDOW);
    uview_show(fd);

    int winw = 640, winh = 480;
    uint32_t *colors = malloc(winw * winh * 4);
    uview_bitmap_t vbmp;
    
    int i = 0; 
    uview_msg_t msg;
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
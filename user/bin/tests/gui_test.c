#include "test.h"
#include <xgui.h>

#include <sys/time.h>

#define W 1920
#define H 1080


int gui_test(int argc, char *argv[])
{
    sleep(1);
    printf("hello, xgui client!\n");

    int vhandle0 = xgui_create_view(0, 0, W, H);
    if (vhandle0 < 0) {
        printf("create view failed!\n");
    }
    printf("vhandle %d!\n", vhandle0);
    xgui_show_view(vhandle0);

    int vhandle = xgui_create_view(200, 200, 320, 240);
    if (vhandle < 0) {
        printf("create view failed!\n");
    }
    printf("vhandle %d!\n", vhandle);
    xgui_show_view(vhandle);

    vhandle = xgui_create_view(300, 300, 320, 240);
    if (vhandle < 0) {
        printf("create view failed!\n");
    }
    printf("vhandle %d!\n", vhandle);

    xgui_show_view(vhandle);

    int x = 100, y = 100;
    while (x <  500) {
        xgui_render_rectfill(vhandle, 0, 0, 320, 240, 
            XGUI_RGB(x / 5, x / 2, x));
        xgui_move_view(vhandle, x, y); 
        x += 100;
    }
    xgui_hide_view(vhandle);

    xgui_update_view(vhandle, 100, 200, 300, 400);

    xgui_destroy_view(vhandle);
    printf("destroy done!\n");

    struct timeval time1 = {0, 0};
    struct timeval time2 = {0, 0};
    int fps = 0;
    gettimeofday(&time1, NULL);

    int c = 0;
    while (1)
    {
        xgui_render_rectfill(vhandle0, 0, 0, W, H, 
            XGUI_RGB(c  + 20, c +  40, c + 60));
        xgui_update_view(vhandle0, 0, 0, W, H);
        c++;

        fps++;
        gettimeofday(&time2, NULL);
        unsigned long long mtime = (time2.tv_sec - time1.tv_sec) * 1000000 + (time2.tv_usec - time1.tv_usec);
        if (mtime > 1000000) {
            printf("fps %d\n", fps);
            fps = 0;
            time1 = time2;
        }
    }
    return 0;
}
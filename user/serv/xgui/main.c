#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/lpc.h>
#include <sys/proc.h>
#include <xgui_core.h>
#include <xgui_view.h>
#include <xgui_bitmap.h>
#include <xgui_vrender.h>
#include <xgui_image.h>

extern bool guiserv_echo_main(uint32_t code, lpc_parcel_t data, lpc_parcel_t reply);


static void *guiserv_thread(void *arg)
{
    /* 创建子进程 */
    char *argv[2] = {"tests", NULL};
    create_process(argv, environ, 0);
    lpc_echo(LPC_ID_GRAPH, guiserv_echo_main);
    return NULL;
}

int main(int argc, char *argv[])
{
    printf("hello xgui!\n");
    xgui_init();

    xgui_view_t *view0 = xgui_view_create(0, 0, 800, 600);
    xgui_vrender_viewfill(view0, XGUI_RED);
    xgui_view_set_z(view0, 0);

    xgui_view_t *view = xgui_view_create(100, 100, 400, 300);
    xgui_vrender_viewfill(view, XGUI_BLUE);
    
    printf("top z:%d\n", xgui_view_get_top()->z);
    xgui_view_move_upper_top(view);

    view = xgui_view_create(300, 300, 400, 300);
    xgui_vrender_viewfill(view, XGUI_GREEN);
    xgui_vrender_line(view, 0, 0, 100, 150, XGUI_YELLOW);
    xgui_vrender_rect(view, 50, 50, 50, 100, XGUI_BLACK);
    xgui_vrender_vline(view, 200, 50, 200, XGUI_RED);
    xgui_vrender_hline(view, 50, 200, 200, XGUI_RED);
    xgui_vrender_rectfill(view, 100, 150, 100, 50, XGUI_WHITE);

    xgui_vrender_text(view, 100, 50, "hello, world!\n", XGUI_GRAY);

    xgui_vrender_text(view, 200, 100, "hello, world!\n", XGUI_BLACK);

    xgui_bitmap_t *bmp = xgui_bitmap_create(32, 32);
    assert(bmp);
    int i, j;
    for (j = 0; j < 32; j++) {
        for (i = 0; i < 32; i++) {
            xgui_bitmap_putpixel(bmp, i, j, XGUI_RGB(i * j, i * 10, j * 10));
        }
    }
    xgui_vrender_bitblt(view, 0, 0, bmp, 10,10,32,32);
    printf("top z:%d\n", xgui_view_get_top()->z);
    xgui_view_move_upper_top(view);

    int iw, ih;
    unsigned char *ibuf = xgui_load_image("/res/cursor.png", &iw, &ih, NULL);
    view = xgui_view_create(400, 300, iw, ih);
    xgui_bitmap_t ibmp;
    xgui_bitmap_init(&ibmp, iw, ih, (xgui_color_t *) ibuf);
    xgui_vrender_bitblt(view, 0, 0, &ibmp, 0,0,iw, ih);
    free(ibuf);

    printf("top z:%d\n", xgui_view_get_top()->z);
    xgui_view_move_upper_top(view);
    
    pthread_t thread;
    pthread_create(&thread, NULL, guiserv_thread, NULL);
    xgui_loop();
    return 0;
}
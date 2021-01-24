#include "xtk.h"
#include <uview.h>
#include <stdlib.h>
#include <assert.h>

#include <dotfont.h>

int xtk_main()
{
    xtk_spirit_t *spirit;
    uview_msg_t msg;
    xtk_view_t *pview;
    while (1) {    
        xtk_view_for_each (pview) {
            uview_set_wait(pview->view, 1);
            if (uview_get_msg(pview->view, &msg) < 0) {
                continue;
            }
            // 遍历每一个视图来获取上面的精灵
            list_for_each_owner (spirit, &pview->spirit_list_head, list) {
                xtk_window_main(spirit, &msg);
                // xtk_xxx_main
            }
        }
    }
    return 0;
}

xtk_spirit_t *btn_root;
xtk_spirit_t *win_root;
void xtk_test(int fd, uview_bitmap_t *wbmp)
{
    xtk_image_t *img = xtk_image_load2("/res/cursor.png", 32, 32);
    assert(img);
    
    uview_bitmap_t *bmp = uview_bitmap_create(img->w, img->h);
    assert(bmp);
    
    uview_bitmap_t srcbmp;
    uview_bitmap_init(&srcbmp, img->w, img->h, (uview_color_t *) img->buf);
    
    uview_bitmap_bitblt(bmp, 0, 0, &srcbmp, 0, 0, srcbmp.width, srcbmp.height);
    uview_bitblt_update(fd, 0, 0, bmp);
    
    uview_bitmap_destroy(bmp);
    xtk_image_destroy(img);

    xtk_spirit_t *spirit = xtk_spirit_create(100, 100, 100, 24);
    assert(spirit);
    spirit->style.background_align = XTK_ALIGN_CENTER;
    spirit->style.background_color = XTK_BLUE;
    xtk_spirit_set_text(spirit, "abcdef!asdasdasd");
    xtk_spirit_set_background_image(spirit, "/res/cursor.png");
    xtk_spirit_auto_size(spirit);
    xtk_spirit_to_surface(spirit, wbmp);
    uview_bitblt_update(fd, 0, 0, wbmp);
    uview_bitmap_t *bmp0 = uview_bitmap_create(32, 32);
    assert(bmp0);
    uview_bitmap_rectfill(bmp0, 0, 0, 32, 32, XTK_BLACK);
    spirit->style.background_color = XTK_GREEN;
    spirit->style.border_color = XTK_YELLOW;
    spirit->style.color = XTK_WHITE;
    spirit->style.align = XTK_ALIGN_CENTER;
    xtk_spirit_set_surface(spirit, bmp0);
    
    xtk_spirit_set_text(spirit, "hello, world!\n");
    xtk_spirit_set_background_image(spirit, NULL);
    xtk_spirit_auto_size(spirit);
    
    uview_bitmap_t *bmp1 = uview_bitmap_create(spirit->width, spirit->height);
    assert(bmp1);
    
    xtk_spirit_set_pos(spirit, 0, 0);
    xtk_spirit_to_surface(spirit, bmp1);

    #if 0
    uview_bitmap_bitblt(wbmp, 100, 200, bmp1, 0, 0, 100, 24);
    uview_bitblt_update(fd, 0, 0, wbmp);
    #else
    uview_bitblt_update(fd, 100, 200, bmp1);
    #endif

    xtk_spirit_destroy(spirit);
    uview_bitmap_destroy(bmp1);

    xtk_spirit_t *label0 = xtk_label_create("hello");
    xtk_spirit_set_pos(label0, 20, 150);
    xtk_spirit_to_surface(label0, wbmp);
    
    xtk_spirit_t *label1 = xtk_label_create("world");
    xtk_spirit_set_pos(label1, 20, 200);
    xtk_spirit_to_surface(label1, wbmp);
    
    xtk_spirit_destroy(label0);
    xtk_spirit_destroy(label1);

    uview_bitblt_update(fd, 0, 0, wbmp);

    win_root = xtk_window_create2("test", 400, 300, 200, 300, XTK_WINDOW_SHOW);
    assert(win_root);
   
    
    btn_root = xtk_button_create_with_label("hello");
    assert(btn_root);

    xtk_spirit_t *btn1 = xtk_button_create_with_label("world");
    assert(btn1);
    xtk_spirit_t *btn2 = xtk_button_create_with_label("xbook2");
    assert(btn2);

    xtk_spirit_t *l0 = xtk_label_create("welcome to me!");
    assert(l0);
    
    xtk_spirit_set_pos(btn_root, 100, 50);    
    xtk_spirit_set_pos(btn1, 100, 100);
    xtk_spirit_set_pos(btn2, 100, 150);
    xtk_spirit_set_pos(l0, 20, 150);
    
    xtk_container_add(XTK_CONTAINER(win_root), btn_root);
    xtk_container_add(XTK_CONTAINER(win_root), btn1);
    xtk_container_add(XTK_CONTAINER(win_root), btn2);
    xtk_container_add(XTK_CONTAINER(win_root), l0);
    
    #if 0
    xtk_spirit_show(btn1);
    xtk_spirit_show(btn2);
    #else
    xtk_spirit_show_all(win_root);
    #endif

    xtk_spirit_t *win0 = xtk_window_create(XTK_WINDOW_TOPLEVEL);
    assert(win0);
    assert(xtk_window_set_title(XTK_WINDOW(win0), "hello, world!") == 0);
    assert(xtk_window_set_title(XTK_WINDOW(win0), "hello, world2345!") == 0);
    assert(xtk_window_set_title(XTK_WINDOW(win0), "hello, !") == 0);
    
    xtk_window_set_resizable(XTK_WINDOW(win0), true);
    xtk_window_set_position(XTK_WINDOW(win0), XTK_WIN_POS_NONE);
    xtk_spirit_set_size_request(win0, 100, 100);

    
    xtk_spirit_t *btn10 = xtk_button_create_with_label("6666");
    assert(btn10);
    xtk_spirit_set_pos(btn10, 0, 50);
    
    xtk_spirit_show(win0);

    xtk_surface_t *surface0 = xtk_window_get_surface(XTK_WINDOW(win0));
    
    xtk_surface_rectfill(surface0, 0, 0, 100, 100, XTK_BLUE);
    xtk_surface_rectfill(surface0, 50, 50, 100, 100, XTK_GREEN);
    
    xtk_surface_t *surface1 = xtk_surface_create(100, 200);
    assert(surface1);

    xtk_surface_rectfill(surface1, 0, 0, surface1->w, surface1->h, XTK_BLACK);
    xtk_surface_blit(surface1, NULL, surface0, NULL);

    xtk_surface_rectfill(surface1, 0, 0, surface1->w, surface1->h, XTK_YELLOW);
    xtk_rect_t dstrect = {100, 50, 50, 50};
    xtk_surface_blit(surface1, NULL, surface0, &dstrect);
    
    xtk_surface_rectfill(surface1, 0, 0, surface1->w, surface1->h, XTK_GRAY);
    xtk_rect_t srcrect = {50, 50, 25, 25};
    
    dstrect.x = 200;
    dstrect.y = 150;
    xtk_surface_blit(surface1, &srcrect, surface0, &dstrect);
    
    //xtk_window_flip(XTK_WINDOW(win0));

    xtk_window_update(XTK_WINDOW(win0), 0, 0, win0->width, win0->height);

    xtk_container_add(XTK_CONTAINER(win0), btn10);
    xtk_spirit_show_all(win0);

    // assert(xtk_spirit_hide_all(win0) == 0);

    //xtk_window_update(XTK_WINDOW(win0), 20, 10, 100, 100);
    //xtk_window_update(XTK_WINDOW(win0), -20, -10, 100, 100);
    //xtk_window_update(XTK_WINDOW(win0), 20, 10, 400, 300);
    
    xtk_main();
}
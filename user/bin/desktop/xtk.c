#include "xtk.h"
#include <uview.h>
#include <stdlib.h>
#include <assert.h>

#include <dotfont.h>

int xtk_main()
{
    // 在根窗口/面板中进行消息获取
    while (1) {
        xtk_window_main();
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
    spirit->style.background_color = UVIEW_BLUE;
    xtk_spirit_set_text(spirit, "abcdef!asdasdasd");
    xtk_spirit_set_background_image(spirit, "/res/cursor.png");
    xtk_spirit_auto_size(spirit);
    xtk_spirit_to_bitmap(spirit, wbmp);
    uview_bitblt_update(fd, 0, 0, wbmp);
    uview_bitmap_t *bmp0 = uview_bitmap_create(32, 32);
    assert(bmp0);
    uview_bitmap_rectfill(bmp0, 0, 0, 32, 32, UVIEW_BLACK);
    spirit->style.background_color = UVIEW_GREEN;
    spirit->style.border_color = UVIEW_YELLOW;
    spirit->style.color = UVIEW_WHITE;
    spirit->style.align = XTK_ALIGN_CENTER;
    xtk_spirit_set_bitmap(spirit, bmp0);
    
    xtk_spirit_set_text(spirit, "hello, world!\n");
    xtk_spirit_set_background_image(spirit, NULL);
    xtk_spirit_auto_size(spirit);
    
    uview_bitmap_t *bmp1 = uview_bitmap_create(spirit->width, spirit->height);
    assert(bmp1);
    
    xtk_collision_t *collision = xtk_collision_create(0, 0, spirit->width, spirit->height);
    assert(collision);
    xtk_spirit_set_collision(spirit, collision);
    xtk_collision_set_visible(collision, XTK_COLLISION_VISIBLE);
    xtk_spirit_set_pos(spirit, 0, 0);
    xtk_spirit_to_bitmap(spirit, bmp1);

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
    xtk_spirit_to_bitmap(label0, wbmp);
    
    xtk_spirit_t *label1 = xtk_label_create("world");
    xtk_spirit_set_pos(label1, 20, 200);
    xtk_spirit_to_bitmap(label1, wbmp);
    
    xtk_spirit_destroy(label0);
    xtk_spirit_destroy(label1);

    uview_bitblt_update(fd, 0, 0, wbmp);

    win_root = xtk_window_create("test", 400, 300, 200, 300, XTK_WINDOW_SHOW);
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


    #if 0
    int win_fd = win_root->view;
    uview_msg_t msg;
    while (1) {
        if (uview_get_msg(win_fd, &msg) < 0) {
            continue;
        }

        switch (uview_msg_get_type(&msg)) {
        case UVIEW_MSG_MOUSE_MOTION:
            {
                int x = uview_msg_get_mouse_x(&msg);
                int y = uview_msg_get_mouse_y(&msg);
                xtk_mouse_motion(x, y);
            }
            break;
        case UVIEW_MSG_MOUSE_LBTN_DOWN:
            {
                int x = uview_msg_get_mouse_x(&msg);
                int y = uview_msg_get_mouse_y(&msg);
                xtk_mouse_lbtn_down(x, y);
            }
            break;
        case UVIEW_MSG_MOUSE_LBTN_UP:
            {
                int x = uview_msg_get_mouse_x(&msg);
                int y = uview_msg_get_mouse_y(&msg);
                xtk_mouse_lbtn_up(x, y);
            }
            break;
        default:
            break;
        }
    }    
    #endif
    xtk_main();
}
#if 0
void xtk_mouse_motion(int x, int y)
{
    //printf("mouse motion: %d, %d\n", x, y);

    /* 检测窗口上的容器树里面的所有内容，直到遇到一个适合的容器，就结束 */
    xtk_button_t *btn = XTK_BUTTON(btn_root); 
    if (XTK_IN_SPIRIT(btn_root, x, y)) {
        if (btn->state == XTK_BUTTON_IDLE) {
            xtk_button_change_state(btn, XTK_BUTTON_TOUCH);
        }
    } else {
        xtk_button_change_state(btn, XTK_BUTTON_IDLE);
    }
    // 更新
    xtk_spirit_to_bitmap(btn_root, win_root->bitmap);
    uview_bitblt_update_ex(win_root->view, btn_root->x, btn_root->y,
        win_root->bitmap, btn_root->x, btn_root->y, btn_root->width, btn_root->height);
    
}

void xtk_mouse_lbtn_down(int x, int y)
{
    printf("mouse down: %d, %d\n", x, y);

    /* 检测窗口上的容器树里面的所有内容，直到遇到一个适合的容器，就结束 */
    xtk_button_t *btn = XTK_BUTTON(btn_root); 
    if (XTK_IN_SPIRIT(btn_root, x, y)) {
        if (btn->state == XTK_BUTTON_TOUCH) {
            xtk_button_change_state(btn, XTK_BUTTON_CLICK);
            // 更新
            xtk_spirit_to_bitmap(btn_root, win_root->bitmap);
            uview_bitblt_update_ex(win_root->view, btn_root->x, btn_root->y,
                win_root->bitmap, btn_root->x, btn_root->y, btn_root->width, btn_root->height);        
        } 
    }
}

void xtk_mouse_lbtn_up(int x, int y)
{
    printf("mouse up: %d, %d\n", x, y);

    xtk_button_t *btn = XTK_BUTTON(btn_root); 
    if (XTK_IN_SPIRIT(btn_root, x, y)) {
        if (btn->state == XTK_BUTTON_CLICK) {
            printf("mouse call signal: %d, %d\n", x, y);
            xtk_button_change_state(btn, XTK_BUTTON_TOUCH);
            // 更新
            xtk_spirit_to_bitmap(btn_root, win_root->bitmap);
            uview_bitblt_update_ex(win_root->view, btn_root->x, btn_root->y,
                win_root->bitmap, btn_root->x, btn_root->y, btn_root->width, btn_root->height);     
        }
    }
}
#endif
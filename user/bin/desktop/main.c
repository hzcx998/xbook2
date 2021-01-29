#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <uview.h>
#include <assert.h>

#include <pthread.h>
#include "xtk.h"

void win_thread();

#define MOUSE_CURSOR_DIR            "/system/cursors"
#define BACKGROUND_IMAGE_NAME       "/system/background/picture.jpg"

void desktop_setup(xtk_spirit_t *spirit)
{
    xtk_spirit_show(spirit);
    // 加载鼠标光标
    xtk_window_load_mouse_cursors(XTK_WINDOW(spirit), MOUSE_CURSOR_DIR);
    
    // 加载壁纸
    xtk_image_t *img = xtk_image_load(BACKGROUND_IMAGE_NAME);
    if (!img)
        return;
    if (xtk_image_resize2(img, spirit->width, spirit->height, sizeof(uint32_t)) < 0) {
        xtk_image_destroy(img);
        img = NULL;
        return;
    }
    xtk_surface_t *surface = xtk_window_get_surface(XTK_WINDOW(spirit));
    assert(surface);
    
    xtk_surface_t img_surface;
    xtk_surface_init(&img_surface, img->w, img->h, (uint32_t *) img->buf);
    xtk_surface_blit(&img_surface, NULL, surface, NULL);
    // 刷新到窗口里面
    xtk_window_flip(XTK_WINDOW(spirit));
    xtk_image_destroy(img);

    // 创建子进程
    pid_t pid = fork();
    if (!pid) {
        win_thread();
        exit(-1);
    }
}

void desktop_proc(xtk_spirit_t *spirit, uview_msg_t *msg)
{
    int type = uview_msg_get_type(msg);
    //printf("msg: %d\n", type);
    switch (type) {
    case UVIEW_MSG_RESIZE: // 收到调整大小消息，再显示精灵
        desktop_setup(spirit);
        break;
    default:
        break;
    }
}

int open_desktop()
{
    if (xtk_init(NULL, NULL) < 0) {
        printf("xtk_init failed!\n");
        return -1;
    }
    xtk_spirit_t *screen_window = xtk_window_create(XTK_WINDOW_POPUP);
    if (!screen_window) {
        printf("xtk create desktop failed!\n");
        return -1;
    }
    // 设置为固定窗口，不能移动
    xtk_window_set_fixed(XTK_WINDOW(screen_window), true);
    // 显示到屏幕上后再调整大小
    assert(xtk_window_resize_to_screen(XTK_WINDOW(screen_window)) == 0);
    // 禁止窗口大小调整
    xtk_window_set_resizable(XTK_WINDOW(screen_window), false);
    xtk_window_set_routine(XTK_WINDOW(screen_window), desktop_proc);
    xtk_main();
    return 0;
}

int main(int argc, char *argv[]) 
{
    if (open_desktop() < 0)
        return -1;
    return 0;
}

void win_proc(xtk_spirit_t *window, uview_msg_t *msg)
{
    //printf("msg %d\n", uview_msg_get_type(msg));
    switch (uview_msg_get_type(msg)) {
    case UVIEW_MSG_MOUSE_MOTION:
        {
            int x = uview_msg_get_mouse_x(msg);
            int y = uview_msg_get_mouse_y(msg);    
            printf("mouse %d, %d\n", x, y);
        }
        break;
    default:
        break;
    }
}

bool btn_event(xtk_spirit_t *spirit, void *data)
{
    printf("btn event: %s\n", (char *) data);
    return true;
}

bool delete_event(xtk_spirit_t *spirit, void *data)
{
    printf("delete window event\n");
    return false;
}

bool destroy_event(xtk_spirit_t *spirit, void *data)
{
    printf("destroy window event\n");
    return false;
}

xtk_spirit_t *btn_root;
xtk_spirit_t *win_root;
void win_thread()
{    
    // xtk start
    xtk_init(NULL, NULL);
    #if 1
    win_root = xtk_window_create(XTK_WINDOW_TOPLEVEL);  
    assert(win_root);
    xtk_window_set_title(XTK_WINDOW(win_root), "test");
    // xtk_window_set_position(XTK_WINDOW(win_root), XTK_WIN_POS_MOUSE);

    xtk_window_set_position(XTK_WINDOW(win_root), XTK_WIN_POS_NONE);
    xtk_window_resize(XTK_WINDOW(win_root), 640, 480);

    btn_root = xtk_button_create_with_label("hello");
    assert(btn_root);
    btn_root->style.cursor = XTK_CURSOR_HAND;

    xtk_spirit_t *btn1 = xtk_button_create_with_label("world");
    assert(btn1);
    btn1->style.cursor = XTK_CURSOR_HAND;
    
    xtk_spirit_t *btn2 = xtk_button_create_with_label("xbook2");
    assert(btn2);
    btn2->style.cursor = XTK_CURSOR_HAND;
    
    xtk_spirit_t *l0 = xtk_label_create("welcome to me!");
    assert(l0);
    l0->style.cursor = XTK_CURSOR_TEXT;
    
    xtk_spirit_set_pos(btn_root, 100, 50);    
    xtk_spirit_set_pos(btn1, 100, 100);
    xtk_spirit_set_pos(btn2, 100, 150);
    xtk_spirit_set_pos(l0, 20, 150);
    
    xtk_container_add(XTK_CONTAINER(win_root), btn_root);
    xtk_container_add(XTK_CONTAINER(win_root), btn1);
    xtk_container_add(XTK_CONTAINER(win_root), btn2);
    xtk_container_add(XTK_CONTAINER(win_root), l0);
    
    xtk_signal_connect(btn2, "enter", btn_event, "enter");
    xtk_signal_connect(btn2, "leave", btn_event, "leave");
    xtk_signal_connect(btn2, "released", btn_event, "released");
    xtk_signal_connect(btn2, "pressed", btn_event, "pressed");

    xtk_spirit_show_all(win_root);
    #endif

    xtk_spirit_t *win0 = xtk_window_create(XTK_WINDOW_TOPLEVEL);
    assert(win0);
    assert(xtk_window_set_title(XTK_WINDOW(win0), "hello, world!") == 0);
    assert(xtk_window_set_title(XTK_WINDOW(win0), "hello, world2345!") == 0);
    assert(xtk_window_set_title(XTK_WINDOW(win0), "hello, !") == 0);
    
    xtk_window_set_resizable(XTK_WINDOW(win0), true);
    xtk_window_set_position(XTK_WINDOW(win0), XTK_WIN_POS_CENTER);
    xtk_spirit_set_size_request(win0, 100, 100);

    xtk_signal_connect(win0, "delete_event", delete_event, NULL);
    xtk_signal_connect(win0, "destroy", destroy_event, NULL);
    
    xtk_spirit_t *btn10 = xtk_button_create_with_label("6666");
    assert(btn10);
    xtk_spirit_set_pos(btn10, 0, 50);
    btn10->style.cursor = XTK_CURSOR_PEN;
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

    xtk_window_update(XTK_WINDOW(win0), 0, 0, win0->width, win0->height);

    xtk_container_add(XTK_CONTAINER(win0), btn10);
    xtk_spirit_show_all(win0);

    xtk_window_set_routine(XTK_WINDOW(win0), win_proc);
    // xtk_main_quit();
    xtk_main();
}

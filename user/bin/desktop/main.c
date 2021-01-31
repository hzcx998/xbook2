#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <uview.h>
#include <assert.h>
#include <sys/ioctl.h>

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
    // 分离当前的tty和键盘
    ioctl(STDIN_FILENO, TTYIO_DETACH, 0);
    if (open_desktop() < 0)
        return -1;
    return 0;
}


uint32_t win_color = 0;
int win_update = 0;
void win_proc(xtk_spirit_t *spirit, uview_msg_t *msg)
{
    //printf("msg %d\n", uview_msg_get_type(msg));
    xtk_window_t *window = XTK_WINDOW(spirit);
    switch (uview_msg_get_type(msg)) {
    case UVIEW_MSG_MOUSE_MOTION:
        {
            int x = uview_msg_get_mouse_x(msg);
            int y = uview_msg_get_mouse_y(msg);    
            printf("mouse %d, %d\n", x, y);
        }
        break;
    case UVIEW_MSG_KEY_DOWN:
        {
            int key = uview_msg_get_key_code(msg);
            int modify = uview_msg_get_key_modify(msg);
            printf("key down key:%d(%c), modify: %x\n", key, key, modify);            
        }   
        break;
    case UVIEW_MSG_KEY_UP:
        {
            int key = uview_msg_get_key_code(msg);
            int modify = uview_msg_get_key_modify(msg);
            printf("key up key:%d(%c), modify: %x\n", key, key, modify);            
        }  
        break;
    default:
        break;
    }
}

bool win_mouse_motion(xtk_spirit_t *spirit, xtk_event_t *event, void *arg)
{
    printf("win mouse motion %d %d\n", event->motion.x, event->motion.y);
    return true;
}

bool win_mouse_press(xtk_spirit_t *spirit, xtk_event_t *event, void *arg)
{
    printf("win mouse press %d %d %d\n", event->button.button, event->motion.x, event->motion.y);
    return true;
}

bool win_mouse_release(xtk_spirit_t *spirit, xtk_event_t *event, void *arg)
{
    printf("win mouse release %d %d %d\n", event->button.button, event->motion.x, event->motion.y);
    return true;
}

bool win_key_press(xtk_spirit_t *spirit, xtk_event_t *event, void *arg)
{
    printf("win key press %d %d\n", event->key.keycode.code, event->key.keycode.modify);
    switch (event->key.keycode.code)
    {
    case UVIEW_KEY_SPACE:
        printf("space down!\n");
        break;
    
    default:
        break;
    }
    
    return true;
}

bool win_key_release(xtk_spirit_t *spirit, xtk_event_t *event, void *arg)
{
    printf("win key release %d %d\n", event->key.keycode.code, event->key.keycode.modify);
    return true;
}

bool win_mouse_wheel(xtk_spirit_t *spirit, xtk_event_t *event, void *arg)
{
    printf("win mouse wheel %d %d %d\n", event->wheel.wheel, event->wheel.x, event->wheel.y);
    return true;
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
static int fps = 0;
void win_paint(xtk_spirit_t *spirit, xtk_rect_t *rect)
{
    xtk_window_t *window = XTK_WINDOW(spirit);
    xtk_surface_t *surface = xtk_window_get_surface(window);
    xtk_surface_rectfill(surface, rect->x, rect->y, rect->w, rect->h, 
        XTK_RGB(win_color, win_color * 2, win_color + 10)); // 重绘窗口
    win_color += 5;
    
    win_update++;
    xtk_window_flip(window); // 刷新窗口
    fps++;
    xtk_window_invalid_window(window); // 重新设置无效区域
    xtk_window_paint(window);   // 发出绘制窗口消息
}

bool win_timeout2(xtk_spirit_t *spirit, uint32_t timer_id, void *data)
{
    static int count = 0;
    printf("timeout %d\n", timer_id);
    count++;
    if (count > 100) {
        xtk_window_remove_timer(XTK_WINDOW(spirit), timer_id);
        return false;
    }
    return true;
}

bool win_timeout(xtk_spirit_t *spirit, uint32_t timer_id, void *data)
{
    char title[32] = {0,};
    sprintf(title, "fps:%d", fps);
    xtk_window_set_title(XTK_WINDOW(spirit), title);
    fps = 0;
    return true;
}

xtk_spirit_t *btn_root;
xtk_spirit_t *win_root;
void win_thread()
{    
    // xtk start
    xtk_init(NULL, NULL);
    #if 0
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
    
    xtk_signal_connect(btn2, "enter_notify", XTK_CALLBACK(btn_event), "enter");
    xtk_signal_connect(btn2, "leave_notify", XTK_CALLBACK(btn_event), "leave");
    xtk_signal_connect(btn2, "button_release", XTK_CALLBACK(btn_event), "released");
    xtk_signal_connect(btn2, "button_presse", XTK_CALLBACK(btn_event), "pressed");

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

    xtk_signal_connect(win0, "delete_event", XTK_CALLBACK(delete_event), NULL);
    xtk_signal_connect(win0, "destroy_event", XTK_CALLBACK(destroy_event), NULL);
    
    xtk_signal_connect(win0, "motion_notify", XTK_CALLBACK(win_mouse_motion), NULL);
    xtk_signal_connect(win0, "button_press", XTK_CALLBACK(win_mouse_press), NULL);
    xtk_signal_connect(win0, "button_release", XTK_CALLBACK(win_mouse_release), NULL);
    xtk_signal_connect(win0, "button_scroll", XTK_CALLBACK(win_mouse_wheel), NULL);
    
    xtk_signal_connect(win0, "key_press", XTK_CALLBACK(win_key_press), NULL);
    xtk_signal_connect(win0, "key_release", XTK_CALLBACK(win_key_release), NULL);
    
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
    
    xtk_window_paint_callback(XTK_WINDOW(win0), win_paint);
    // 设置无效区域，触发绘图消息
    xtk_window_invalid_window(XTK_WINDOW(win0));
    xtk_window_paint(XTK_WINDOW(win0));

    xtk_window_add_timer(XTK_WINDOW(win0), 1000, win_timeout, NULL);
    
    #if 0
    xtk_window_add_timer(XTK_WINDOW(win0), 20, win_timeout2, NULL);
    xtk_window_add_timer(XTK_WINDOW(win0), 20, win_timeout, NULL);
    xtk_window_add_timer(XTK_WINDOW(win0), 20, win_timeout2, NULL);
    xtk_window_add_timer(XTK_WINDOW(win0), 20, win_timeout, NULL);
    xtk_window_add_timer(XTK_WINDOW(win0), 20, win_timeout2, NULL);
    #endif
    // xtk_main_quit();
    xtk_main();
}

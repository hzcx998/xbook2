#include "xtk.h"
#include <stddef.h>
#include <stdio.h>

int xtk_mouse_motion(xtk_spirit_t *spirit, int x, int y)
{
    xtk_container_t *container = spirit->container;
    if (!container)
        return -1;
    xtk_spirit_t *tmp;
    list_for_each_owner (tmp, &container->children_list, list) {
        if (!tmp->visible)
            continue;
        switch (tmp->type)
        {
        case XTK_SPIRIT_TYPE_LABEL:
            {
                
            }
            break;
        case XTK_SPIRIT_TYPE_BUTTON:
            {
                xtk_button_t *btn = XTK_BUTTON(tmp);
                if (XTK_IN_SPIRIT(tmp, x, y)) {
                    uview_set_mouse_state(spirit->view, tmp->style.cursor);                        
                    if (btn->state == XTK_BUTTON_IDLE) {
                        xtk_button_change_state(btn, XTK_BUTTON_TOUCH);
                        xtk_spirit_show(tmp);
                        return 0;
                    }
                } else {
                    if (btn->state != XTK_BUTTON_IDLE) {
                        xtk_button_change_state(btn, XTK_BUTTON_IDLE);
                        xtk_spirit_show(tmp);
                        uview_set_mouse_state(spirit->view, XTK_CURSOR_NORMAL);
                    }
                }
            }
            break;
        default:
            break;
        }
    }
    return -1;
}

int xtk_mouse_lbtn_down(xtk_spirit_t *spirit, int x, int y)
{
    printf("down\n");
    xtk_container_t *container = spirit->container;
    if (!container)
        return -1;
    xtk_spirit_t *tmp;
    list_for_each_owner (tmp, &container->children_list, list) {
        if (!tmp->visible)
            continue;
        switch (tmp->type)
        {
        case XTK_SPIRIT_TYPE_LABEL:
            {

            }
            break;
        case XTK_SPIRIT_TYPE_BUTTON:
            {
                xtk_button_t *btn = XTK_BUTTON(tmp);
                if (XTK_IN_SPIRIT(tmp, x, y)) {
                    uview_set_mouse_state(spirit->view, tmp->style.cursor);                        
                    if (btn->state == XTK_BUTTON_TOUCH) {
                        xtk_button_change_state(btn, XTK_BUTTON_CLICK);
                        xtk_spirit_show(tmp);                     
                        return 0;  
                    }
                }
            }
            break;
        default:
            break;
        }
    }
    return -1;
}

int xtk_mouse_lbtn_up(xtk_spirit_t *spirit, int x, int y)
{
    printf("up\n");
    
    xtk_container_t *container = spirit->container;
    if (!container)
        return -1;
    xtk_spirit_t *tmp;
    list_for_each_owner (tmp, &container->children_list, list) {
        if (!tmp->visible)
            continue;
        switch (tmp->type)
        {
        case XTK_SPIRIT_TYPE_LABEL:
            {

            }
            break;
        case XTK_SPIRIT_TYPE_BUTTON:
            {
                xtk_button_t *btn = XTK_BUTTON(tmp);
                if (XTK_IN_SPIRIT(tmp, x, y)) {
                    uview_set_mouse_state(spirit->view, tmp->style.cursor);                        
                    if (btn->state == XTK_BUTTON_CLICK) {
                        //printf("mouse call signal: %d, %d\n", x, y);
                        xtk_button_change_state(btn, XTK_BUTTON_TOUCH);
                        xtk_spirit_show(tmp);
                        return 0;
                    }
                }
            }
            break;
        default:
            break;
        }
    }
    return -1;
}

int xtk_mouse_load_cursor(int view, char *path, uview_mouse_state_t state, int offx, int offy)
{
    xtk_image_t *img = xtk_image_load2(path, 32, 32);
    if (!img)
        return -1;
    uview_bitmap_t bmp;
    uview_bitmap_init(&bmp, img->w, img->h, (uview_color_t *) img->buf);
    uview_mouse_state_info_t info;
    info.bmp = &bmp;
    info.off_x = offx;
    info.off_y = offy;
    info.state = state;
    if (uview_set_mouse_state_info(view, &info) < 0) {
        free(img);
        return -1;
    }
    free(img);
    return 0;
}

typedef struct {
    char *pathname;
    int off_x;
    int off_y;
} xtk_cursor_info_t;


static xtk_cursor_info_t cursor_info[] = {
    {"normal", -7, -7},
    {"hold", -14, -14},
    {"help", -7, -7},
    {"pen", -14, -14},
    {"busy", -7, -7},
    {"accurate", -14, -14},
    {"text", 0, 0},
    {"unusable", -14, -14},
    {"vresize", -14, -14},
    {"hresize", -14, -14},
    {"dresize1", -14, -14},
    {"dresize2", -14, -14},
    {"resizeall", -14, -14},
    {"hand", -11, -8},
};

void xtk_mouse_load_cursors(int view, char *pathname)
{
    uview_mouse_state_t i;
    char filename[MAX_PATH];
    for (i = 0; i < UVIEW_MOUSE_STATE_NR; i++) {
        memset(filename, 0, MAX_PATH);
        strcpy(filename, pathname);
        strcat(filename, "/");
        strcat(filename, cursor_info[i].pathname);
        strcat(filename, ".png");
        printf("mouse cursor: %s.\n", filename);
        if (xtk_mouse_load_cursor(view, filename, i, cursor_info[i].off_x, cursor_info[i].off_y) < 0)
            printf("mouse load cursor %s failed!\n", filename);
    }
}
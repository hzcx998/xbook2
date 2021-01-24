#include "xtk.h"

int xtk_mouse_motion(xtk_spirit_t *spirit, int x, int y)
{
    xtk_container_t *container = spirit->container;
    if (!container)
        return -1;
    int off_x = x - spirit->x;
    int off_y = y - spirit->y;
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
                if (XTK_IN_SPIRIT(tmp, off_x, off_y)) {
                    if (btn->state == XTK_BUTTON_IDLE) {
                        xtk_button_change_state(btn, XTK_BUTTON_TOUCH);
                        xtk_spirit_show(tmp);
                        return 0;
                    }
                } else {
                    if (btn->state != XTK_BUTTON_IDLE) {
                        xtk_button_change_state(btn, XTK_BUTTON_IDLE);
                        xtk_spirit_show(tmp);
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
    
    int off_x = x - spirit->x;
    int off_y = y - spirit->y;
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
                if (XTK_IN_SPIRIT(tmp, off_x, off_y)) {
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
    int off_x = x - spirit->x;
    int off_y = y - spirit->y;
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
                if (XTK_IN_SPIRIT(tmp, off_x, off_y)) {
                    if (btn->state == XTK_BUTTON_CLICK) {
                        printf("mouse call signal: %d, %d\n", x, y);
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

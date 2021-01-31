#include "xtk_keyboard.h"
#include <stdio.h>

int xtk_keyboard_key_down(xtk_spirit_t *spirit, int keycode, int modify)
{
    printf("key down %d %d\n", keycode, modify);
    xtk_container_t *container = spirit->container;
    if (!container)
        return -1;
    xtk_spirit_t *tmp;
    list_for_each_owner (tmp, &container->children_list, list) {
        if (!tmp->visible)
            continue;
        switch (tmp->type) {
        case XTK_SPIRIT_TYPE_LABEL:
            {

            }
            break;
        case XTK_SPIRIT_TYPE_BUTTON:
            {
                
            }
            break;
        default:
            break;
        }
    }
    return -1;
}

int xtk_keyboard_key_up(xtk_spirit_t *spirit, int keycode, int modify)
{
    printf("key up %d %d\n", keycode, modify);
    xtk_container_t *container = spirit->container;
    if (!container)
        return -1;
    xtk_spirit_t *tmp;
    list_for_each_owner (tmp, &container->children_list, list) {
        if (!tmp->visible)
            continue;
        switch (tmp->type) {
        case XTK_SPIRIT_TYPE_LABEL:
            {

            }
            break;
        case XTK_SPIRIT_TYPE_BUTTON:
            {
                
            }
            break;
        default:
            break;
        }
    }
    return -1;
}

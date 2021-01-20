#include "xtk_container.h"
#include "xtk_spirit.h"
#include "xtk_box.h"
#include <stdlib.h>
#include <assert.h>

xtk_container_t *xtk_container_create(xtk_container_type_t type, xtk_spirit_t *spirit)
{
    xtk_container_t *container = malloc(sizeof(xtk_container_t));
    if (!container)
        return NULL;
    list_init(&container->children_list);
    container->type = type;
    container->spirit = spirit;
    return container;
}

int xtk_container_destroy(xtk_container_t *container)
{
    if (!container)
        return -1;
    container->spirit = NULL;
    free(container);
    return 0;
}

int xtk_container_child_size_all(xtk_container_t *container, xtk_orientation_t orientation, int *out_width, int *out_height)
{
    if (!container)
        return -1;
    xtk_spirit_t *child;
    int width = 0, height = 0;
    int child_count = 0;
    list_for_each_owner (child, &container->children_list, list) {
        if (orientation == XTK_ORIENTATION_HORIZONTAL) {
            width += child->width;
            if (height < child->height)
                height = child->height;
        } else if (orientation == XTK_ORIENTATION_VERTICAL) {
            height += child->height;
            if (width < child->width)
                width = child->width;
        } else {
            width += child->width;
            height += child->height;
        }
        child_count++;
    }
    return child_count;
}


int xtk_container_add2(xtk_container_t *container, xtk_spirit_t *spirit)
{
    if (!container)
        return -1;
    // 防止自己添加自己
    if (container->spirit == spirit)
        return -1;

    /* 单容器就要删除以前的子精灵 */
    if (container->type == XTK_CONTAINER_SINGAL) {
        /* 单容器只能有一个最后添加的容器 */
        xtk_spirit_t *child =   list_first_owner_or_null(&container->children_list,
                                xtk_spirit_t, list);
        if (child) {
            list_del_init(&child->list);
            spirit->attached_container = NULL;
            xtk_spirit_set_view(child, -1);
        }
    }
    // 添加到容器中
    list_add_tail(&spirit->list, &container->children_list);
    
    // 资源绑定: 附加容器、视图
    xtk_spirit_t *attached_spirit = (xtk_spirit_t *)container->spirit;
    spirit->attached_container = container;
    xtk_spirit_set_view(spirit, attached_spirit->view);

    // 根据容器的类型设置位置信息
    if (spirit->type == XTK_SPIRIT_TYPE_BOX) {
        xtk_spirit_set_size(spirit, attached_spirit->width, attached_spirit->height);
        xtk_spirit_set_bitmap(spirit, uview_bitmap_create(spirit->width, spirit->height));
    }
    
    // 调整位置，大小等
    int start_x = 0, start_y = 0;
    if (container->type == XTK_CONTAINER_SINGAL) {
        xtk_spirit_calc_aligin_pos(attached_spirit, spirit->width, spirit->height, &start_x, &start_y);
        xtk_spirit_set_pos(spirit, start_x, start_y);
    } else {
        // 计算所有子精灵占用的宽度和高度，重新布局
        int children_width = 0, children_height = 0;
        if (attached_spirit->type == XTK_SPIRIT_TYPE_BOX) {
            xtk_box_t *box = XTK_BOX(attached_spirit);
            int child_count = xtk_container_child_size_all(container, box->orientation, &children_width, &children_height);
            // 加上间隙
            if (box->orientation == XTK_ORIENTATION_HORIZONTAL) {
                children_width += (child_count - 1) * box->spacing;
            } else if (box->orientation == XTK_ORIENTATION_VERTICAL) {
                children_height += (child_count - 1) * box->spacing;
            } else {

            }
            // 获取整体位置
            xtk_spirit_calc_aligin_pos(attached_spirit, children_width, children_height, &start_x, &start_y);

            int off_x = 0, off_y = 0;
            // 调整位置
            if (box->orientation == XTK_ORIENTATION_HORIZONTAL) {
                xtk_spirit_t *child;
                list_for_each_owner (child, &container->children_list, list) {
                    xtk_spirit_set_pos(child, start_x + off_x, start_y + off_y);
                    off_x += child->width + box->spacing;
                }
            } else if (box->orientation == XTK_ORIENTATION_VERTICAL) {
                xtk_spirit_t *child;
                list_for_each_owner (child, &container->children_list, list) {
                    xtk_spirit_set_pos(child, start_x + off_x, start_y + off_y);
                    off_y += child->height + box->spacing;
                }
            } else {

            }
        }
    }
    return 0;
}

int xtk_container_add(xtk_container_t *container, xtk_spirit_t *spirit)
{
    if (!container)
        return -1;
    // 防止自己添加自己
    if (container->spirit == spirit)
        return -1;
    
    // 添加到容器中
    list_add_tail(&spirit->list, &container->children_list);
    
    // 资源绑定: 附加容器、视图
    xtk_spirit_t *attached_spirit = (xtk_spirit_t *)container->spirit;
    spirit->attached_container = container;
    xtk_spirit_set_view(spirit, attached_spirit->view);
    return 0;
}

int xtk_container_remove(xtk_container_t *container, xtk_spirit_t *spirit)
{
    if (!container)
        return -1;
    xtk_spirit_t *tmp;
    list_for_each_owner (tmp, &container->children_list, list) {
        if (tmp == spirit) {
            list_del_init(&spirit->list);
            spirit->attached_container = NULL;
            xtk_spirit_set_view(spirit, -1);
            return 0;
        }
    }
    return -1;
}

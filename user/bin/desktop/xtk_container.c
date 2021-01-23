#include "xtk_container.h"
#include "xtk_spirit.h"
#include "xtk_box.h"
#include <stdlib.h>
#include <assert.h>

xtk_container_t *xtk_container_create(xtk_spirit_t *spirit)
{
    xtk_container_t *container = malloc(sizeof(xtk_container_t));
    if (!container)
        return NULL;
    list_init(&container->children_list);
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

int xtk_container_remove_and_destroy_all(xtk_container_t *container)
{
    if (!container)
        return -1;
    xtk_spirit_t *spirit, *next;
    list_for_each_owner_safe (spirit, next, &container->children_list, list) {
        list_del_init(&spirit->list);
        spirit->attached_container = NULL;
        xtk_spirit_set_view(spirit, -1);
        xtk_spirit_destroy(spirit);
    }
    return 0;
}
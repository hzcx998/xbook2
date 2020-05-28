#include <window/layer.h>
#include <drivers/screen.h>
#include <graph/draw.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

LIST_HEAD(layer_list_head);
LIST_HEAD(layer_show_list_head);

int top_layer_z = -1;    /* 顶层图层的Z轴 */
/* 顶层图层和底层图层 */
layer_t *layer_topest = NULL;

/**
 * create_layer - 创建一个图层
 * @width: 图层宽度
 * @height: 图层高度
 * 
 * @成功返回图层的指针，失败返回NULL
 */
layer_t *create_layer(int width, int height)
{
    // (GUI_COLOR *) malloc(width * height * sizeof(GUI_COLOR));
    
    GUI_COLOR *buffer = sbrk(0);
    if (buffer == (void *) -1)
        return NULL;
    if (sbrk(width * height * sizeof(GUI_COLOR)) == (void *) -1) {
        return NULL;
    }
    memset(buffer, 0, width * height * sizeof(GUI_COLOR));
    layer_t *layer = sbrk(0);
    if (layer == (void *) -1) {
        sbrk(-(width * height * sizeof(GUI_COLOR)));
        return NULL;
    }
    if (sbrk(sizeof(layer_t)) == (void *) -1) {
        sbrk(-(width * height * sizeof(GUI_COLOR)));
        
        return NULL;
    }
    /*layer_t *layer = (layer_t *) malloc(sizeof(layer_t));
    if (layer == NULL) {
        free(buffer);
        return NULL;
    }*/
    memset(layer, 0, sizeof(layer_t));
    layer->buffer = buffer;
    layer->width = width;
    layer->height = height;
    layer->z = -1;          /* 不显示的图层 */
    init_list(&layer->list);
    /* 添加到链表末尾 */
    list_add_tail(&layer->global_list, &layer_list_head);
    return layer;
}

/**
 * layer_set_z - 设置图层的Z轴深度
 * @layer: 图层
 * @z: z轴
 * 
 * Z轴轴序：
 *      可显示图层的轴序从0开始排序，一直递增。
 *      设置图层Z轴时，如果小于0，就把图层隐藏。
 *      如果图层没有存在于链表中，就是插入一个新图层。
 *      如果图层已经在链表中，那么就是调整一个图层的位置。
 *      如果设置图层小于0，为负，那么就是要隐藏图层
 * @return: 成功返回0，失败返回-1
 */
void layer_set_z(layer_t *layer, int z)
{
    layer_t *tmp = NULL;
    layer_t *old_layer = NULL;
                
    /* 已经存在与现实链表中，就说明只是调整一下高度而已。 */
    if (list_find(&layer->list, &layer_show_list_head)) {
        printf("layer z:%d set new z:%d\n", layer->z, z);
        /* 设置为正，就是要调整高度 */
        if (z >= 0) {
            /* 修复Z轴 */
            if (z > top_layer_z) {
                printf("layer z:%d set new z:%d but above top %d\n", layer->z, z, top_layer_z);
        
                z = top_layer_z;
                
            }
            /* 如果要调整到最高的图层位置 */
            if (z == top_layer_z) {
                printf("layer z:%d set new z:%d same with top %d\n", layer->z, z, top_layer_z);
        
                /* 先从链表中移除 */
                list_del_init(&layer->list);

                /* 把图层后面的图层往下面降 */
                list_for_each_owner (tmp, &layer_show_list_head, list) {
                    if (tmp->z > layer->z) {
                        tmp->z--;
                    }
                }
                layer->z = z;
                /* 添加到链表末尾 */
                list_add_tail(&layer->list, &layer_show_list_head);
            } else {    /* 不是最高图层，那么就和其它图层交换 */
                
                if (z > layer->z) { /* 如果新高度比原来的高度高 */
                    printf("layer z:%d < new z:%d \n", layer->z, z, top_layer_z);
                    /* 先从链表中移除 */
                    list_del_init(&layer->list);

                    /* 把位于旧图层高度和新图层高度之间（不包括旧图层，但包括新图层高度）的图层下降1层 */
                    list_for_each_owner (tmp, &layer_show_list_head, list) {
                        if (tmp->z > layer->z && tmp->z <= z) {
                            if (tmp->z == z) {  /* 记录原来为与新图层这个位置的图层 */
                                old_layer = tmp;
                            }
                            tmp->z--; /* 等上一步判断图层高度后，再减小图层高度 */
                        }
                    }
                    /* 添加到新图层高度的位置 */
                    if (old_layer) {
                        printf("find old layer:%x z%d\n", old_layer, old_layer->z + 1);
                        
                        layer->z = z;
                        list_add_after(&layer->list, &old_layer->list);
                    } else {
                        printf("[error ] not found the old layer on %d\n", z);
                    }
                } else if (z < layer->z) { /* 如果新高度比原来的高度低 */
                    printf("layer z:%d > new z:%d \n", layer->z, z, top_layer_z);
                    /* 先从链表中移除 */
                    list_del_init(&layer->list);
                        
                    /* 把位于旧图层高度和新图层高度之间（不包括旧图层，但包括新图层高度）的图层上升1层 */
                    list_for_each_owner (tmp, &layer_show_list_head, list) {
                        if (tmp->z < layer->z && tmp->z >= z) {
                            if (tmp->z == z) {  /* 记录原来为与新图层这个位置的图层 */
                                old_layer = tmp;
                            }
                            tmp->z++; /* 等上一步判断图层高度后，再增加图层高度 */
                        }
                    }
                    /* 添加到新图层高度的位置 */
                    if (old_layer) {
                        printf("find old layer:%x z%d\n", old_layer, old_layer->z - 1);
                        
                        layer->z = z;
                        list_add_before(&layer->list, &old_layer->list);
                    } else {
                        printf("[error ] not found the old layer on %d\n", z);
                    }
                }
            }
        } else { /* 小于0就是要隐藏起来的图层 */
            printf("layer z:%d will be hided.\n", layer->z);
            
            /* 先从链表中移除 */
            list_del_init(&layer->list);
            
            /* 把位于当前图层后面的图层的高度都向下降1 */
            list_for_each_owner (tmp, &layer_show_list_head, list) {
                if (tmp->z > layer->z) {
                    tmp->z--;
                }
            }
            
            /* 由于隐藏了一个图层，那么，图层顶层的高度就需要减1 */
            top_layer_z--;

            layer->z = -1;  /* 隐藏图层后，高度变为-1 */
        }
    } else {    /* 插入新图层 */
        /* 设置为正，就要显示，那么会添加到显示队列 */
        if (z >= 0) {
            /* 修复Z轴 */
            if (z > top_layer_z) {
                top_layer_z++;      /* 图层顶增加 */
                z = top_layer_z;
                printf("insert a layer at top z %d\n", z);
            } else {
                top_layer_z++;      /* 图层顶增加 */
            }

            /* 如果新高度就是最高的图层，就直接插入到图层队列末尾 */
            if (z == top_layer_z) {
                printf("add a layer %d to tail\n", z);
                layer->z = z;
                /* 直接添加到显示队列 */
                list_add_tail(&layer->list, &layer_show_list_head);
                
            } else {
                printf("add a layer %d to midlle or head\n", z);
                
                /* 查找和当前图层一样高度的图层 */
                list_for_each_owner(tmp, &layer_show_list_head, list) {
                    if (tmp->z == z) { 
                        old_layer = tmp;
                        break;
                    }
                }
                tmp = NULL;
                if (old_layer) {    /* 找到一个旧图层 */
                    printf("found a old layer %d\n", z);

                    /* 把后面的图层的高度都+1 */
                    list_for_each_owner(tmp, &layer_show_list_head, list) {
                        if (tmp->z >= old_layer->z) { 
                            tmp->z++;
                        }
                    }
                    
                    layer->z = z;
                    /* 插入到旧图层前面 */
                    list_add_before(&layer->list, &old_layer->list);
                        
                } else {    /* 没有找到旧图层 */
                    printf("[error ] not found old layer!\n");
                }
            }
        }
        /* 小于0就是要隐藏起来的图层，但是由于不在图层链表中，就不处理 */
    }
}

/**
 * destroy_layer - 销毁图层
 * @layer: 图层
 * 
 * @成功返回0，失败返回-1
 */
int destroy_layer(layer_t *layer)
{
    if (layer == NULL)
        return -1;
    /* 先从链表中删除 */
    list_del_init(&layer->global_list);
    /* 释放缓冲区 */
    //free(layer->buffer);
    /* 释放图层 */
    //free(layer);
    return 0;
}

/**
 * flush_layer - 把图层刷新到显存
 * @layer: 图层
 * 
 * @成功返回0，失败返回-1
 */
int flush_layer(layer_t *layer)
{
    if (layer == NULL)
        return -1;

    graph_draw_bitmap(layer->x, layer->y, layer->width, layer->height, layer->buffer);

    return 0;
}

int flush_layers()
{
    layer_t *layer;
    list_for_each_owner (layer, &layer_show_list_head, list) {
        flush_layer(layer);
    }
    return 0;
}

/**
 * print_layers - 打印所有图层信息
 * 
 */
void print_layers()
{
    printf("layer top z:%d\n", top_layer_z);
    layer_t *layer;
    list_for_each_owner (layer, &layer_show_list_head, list) {
        printf("layer addr:%x buffer:%x width:%d height:%d x:%d y:%d z:%d\n",
            layer, layer->buffer, layer->width, layer->height, layer->x, layer->y, layer->z);
    }
}

/**
 * layer_refresh - 刷新图层
 * 
 * 刷新某个区域的[z0-z1]之间的图层，相当于在1一个3d空间中刷新
 * 某个矩形区域。（有点儿抽象哦，铁汁~）
 * 
 */
void layer_refresh(int left, int top, int right, int buttom, int z0, int z1)
{
    /* 在图层中的位置 */
    int layer_left, layer_top, layer_right, layer_buttom;

    /* 在屏幕上的位置 */
    int screen_x, screen_y;

    /* 在图层上的位置 */
    int layer_x, layer_y;

    /* 修复需要刷新的区域 */
    if (left < 0)
        left = 0;
	if (top < 0)
        top = 0;
	if (right > screen.width)
        right = screen.width;
	if (buttom > screen.height)
        buttom = screen.height;
    
    layer_t *layer;
    GUI_COLOR color;
    /* 刷新高度为[Z0-Z1]区间的图层 */
    list_for_each_owner (layer, &layer_show_list_head, list) {
        if (layer->z >= z0 && layer->z <= z1) {
            /* 获取刷新范围 */
            layer_left = left - layer->x;
            layer_top = top - layer->y;
            layer_right = right - layer->x;
            layer_buttom = buttom - layer->y;
            /* 修复范围 */
            if (layer_left < 0)
                layer_left = 0;
            if (layer_top < 0)
                layer_top = 0;
            if (layer_right > layer->width) 
                layer_right = layer->width;
            if (layer_buttom > layer->height)
                layer_buttom = layer->height;
            
            for(layer_y = layer_top; layer_y < layer_buttom; layer_y++){
                screen_y = layer->y + layer_y;
                for(layer_x = layer_left; layer_x < layer_right; layer_x++){
                    screen_x = layer->x + layer_x;
                    /* 获取图层中的颜色 */
                    color = layer->buffer[layer_y * layer->width + layer_x];
                    /* 写入到显存 */
                    graph_put_point(screen_x, screen_y, color);
                }
            }

        }
    }

}

/**
 * layer_set_xy - 设置图层的位置
 * @layer: 图层
 * @x: 横坐标
 * @y: 纵坐标
 */
void layer_set_xy(layer_t *layer, int x, int y)
{
    int old_x = layer->x;
    int old_y = layer->y;
    /* 显示中的图层才可以刷新 */
    layer->x = x;
    layer->y = y;
    if (layer->z >= 0) {
        layer_refresh(old_x, old_y, old_x + layer->width, old_y + layer->height, 0, layer->z - 1);
        layer_refresh(x, y, x + layer->width, y + layer->height, layer->z, layer->z);
    }
}

layer_t *layer_get_by_z(int z)
{
    if (z > top_layer_z || z < 0) {
        return NULL;
    }
    layer_t *layer;
    list_for_each_owner (layer, &layer_show_list_head, list) {
        if (layer->z == z)
            return layer;
    }
    return NULL;
}


int guisrv_init_layer()
{
    layer_t *layer1, *layer2, *layer3, *layer4;


    layer1 = create_layer(screen.width, screen.height);

    if (layer1 == NULL) {
        printf("create layer failed!\n");
        return -1;
    }
    printf("create layer: addr %x, buffer %x, width %d, height %d\n",
        layer1, layer1->buffer, layer1->width, layer1->height);
    memset(layer1->buffer, 0x99, screen.width * screen.height * sizeof(GUI_COLOR));
    layer_set_z(layer1, 0);

    layer2 = create_layer(50, 50);
    if (layer2 == NULL) {
        printf("create layer failed!\n");
        return -1;
    }
    printf("create layer: addr %x, buffer %x, width %d, height %d\n",
        layer2, layer2->buffer, layer2->width, layer2->height);
    memset(layer2->buffer, 0xff, 50 * 50 * sizeof(GUI_COLOR));
    layer_set_z(layer2, 1);
    print_layers();

    layer3 = create_layer(100, 100);
    if (layer3 == NULL) {
        printf("create layer failed!\n");
        return -1;
    }
    
    printf("create layer: addr %x, buffer %x, width %d, height %d\n",
        layer3, layer3->buffer, layer3->width, layer3->height);
    printf("create layer done!\n");
    memset(layer3->buffer, 0x55, 100 * 100 * sizeof(GUI_COLOR));
    layer_set_z(layer3, 1);

    /* 调整已有图层高度 */
    layer_set_z(layer2, 2);

    print_layers();

    layer_set_z(layer3, 2);

    print_layers();

    layer_set_z(layer1, 1);

    print_layers();

    layer_set_z(layer3, 0);
    
    print_layers();

    layer_set_z(layer3, 3);
    
    print_layers();

    /* 隐藏图层 */
    layer_set_z(layer3, -1);
    
    print_layers();

    layer_set_z(layer2, -1);
    
    print_layers();

    /* 恢复图层 */
    layer_set_z(layer3, 1);
    
    print_layers();

    layer_set_z(layer2, 1);
    
    print_layers();

    layer4 = create_layer(32, 32);
    if (layer4 == NULL) {
        printf("create layer failed!\n");
        return -1;
    }
    printf("create layer: addr %x, buffer %x, width %d, height %d\n",
        layer4, layer4->buffer, layer4->width, layer4->height);
    printf("create layer done!\n");
    memset(layer4->buffer, 0xdd, 32 * 32 * sizeof(GUI_COLOR));
    layer_set_z(layer4, 10);

    print_layers();
    layer_topest = layer_get_by_z(top_layer_z);

    /* 刷新所有图层 */
    layer_refresh(0, 0, screen.width, screen.height, 0, top_layer_z);

    return 0;
}

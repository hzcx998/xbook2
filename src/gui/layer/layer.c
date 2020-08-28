#include <string.h>
#include <unistd.h>
#include <xbook/kmalloc.h>
#include <xbook/debug.h>
#include <xbook/msgpool.h>
#include <xbook/task.h>

#include <arch/page.h>

#include <gui/layer.h>
#include <gui/screen.h>
#include <gui/color.h>
#include <gui/draw.h>
#include <gui/mouse.h>

#define DEBUG_LOCAL 0

//#define LAYER_DEBUG

/* 所有图层都挂载到该链表 */
LIST_HEAD(layer_list_head);
/* 所有显示中的图层挂载到该链表上 */
LIST_HEAD(layer_show_list_head);
/* 顶层图层的Z轴 */
int top_layer_z = -1;    

layer_t *layer_wintop = NULL;   /* win top layer */
layer_t *layer_desktop = NULL;   /* desktop layer */
layer_t *layer_focused = NULL;   /* focused layer */
uint16_t *layer_map = NULL; /* layer z map */
int layer_next_id = 0;  /* 图层id */

mem_cache_t layer_buffer_memcache;  /* 图层缓冲区内存分配器 */

/* 图层链表管理的自旋锁 */
DEFINE_SPIN_LOCK(layer_list_spin_lock);

/* 图层全局变量自旋锁 */
DEFINE_SPIN_LOCK(layer_val_lock);

/**
 * create_layer - 创建一个图层
 * @width: 图层宽度
 * @height: 图层高度
 * 
 * @成功返回图层的指针，失败返回NULL
 */
layer_t *create_layer(int width, int height)
{
    size_t bufsz = width * height * sizeof(GUI_COLOR);
    printk("buf size: %d\n", bufsz);
    uint32_t flags = 0;
    GUI_COLOR *buffer = kmalloc(bufsz);
    if (buffer == NULL) {
        flags |= LAYER_EXT_BUF;
        buffer = mem_cache_alloc_object(&layer_buffer_memcache);
        if (buffer == NULL) {
            return NULL;
        }
    }
        

    memset(buffer, 0, width * height * sizeof(GUI_COLOR));
    layer_t *layer = kmalloc(sizeof(layer_t));

    if (layer == NULL) {
        kfree(buffer);
        return NULL;
    }

    memset(layer, 0, sizeof(layer_t));
    spin_lock(&layer_val_lock);
    layer->id = layer_next_id;
    layer_next_id++;
    spin_unlock(&layer_val_lock);
    layer->buffer = buffer;
    layer->x = 0;
    layer->y = 0;
    layer->width = width;
    layer->height = height;
    layer->z = -1;          /* 不显示的图层 */
    layer->flags = flags;
    layer->extension = NULL;
    
    gui_region_init(&layer->drag_rg);
    gui_region_init(&layer->resize_rg);
    gui_region_init(&layer->resizemin_rg);
    
    INIT_LIST_HEAD(&layer->list);

    spinlock_init(&layer->mutex);

    spin_lock(&layer_list_spin_lock);
    /* 添加到链表末尾 */
    list_add_tail(&layer->global_list, &layer_list_head);
    spin_unlock(&layer_list_spin_lock);
    
    INIT_LIST_HEAD(&layer->widget_list_head);
    return layer;
}

/**
 * 通过图层的唯一id返回图层结构的内存地址
 */
layer_t *layer_find_by_id(int id)
{
    layer_t *l;
    spin_lock(&layer_list_spin_lock);
    list_for_each_owner (l, &layer_list_head, global_list) {
        if (l->id == id) {
            spin_unlock(&layer_list_spin_lock);
            return l;
        }
    }
    spin_unlock(&layer_list_spin_lock);
    return NULL;
}
/**
 * 通过图层的唯一id返回图层结构的内存地址
 * 
 * 没有加锁的版本，需要在关闭中断的情况下调用
 */
layer_t *layer_find_by_id_without_lock(int id)
{
    layer_t *l;
    list_for_each_owner (l, &layer_list_head, global_list) {
        if (l->id == id) {
            return l;
        }
    }
    return NULL;
}

/**
 * 查找z时只在显示队列中查找
 */
layer_t *layer_find_by_z(int z)
{
    spin_lock(&layer_val_lock);
    if (z > top_layer_z || z < 0) {
        spin_unlock(&layer_val_lock);
        return NULL;
    }
    spin_unlock(&layer_val_lock);
    
    layer_t *l;
    spin_lock(&layer_list_spin_lock);
    list_for_each_owner (l, &layer_show_list_head, list) {
        if (l->z == z) {
            spin_unlock(&layer_list_spin_lock);
            return l;
        }
    }
    spin_unlock(&layer_list_spin_lock);
    return NULL;
}

/**
 * 把图层的id刷新到图层地图中，绘制时根据地图来绘制像素数据
 * 
 */
static void layer_refresh_map(int left, int top, int right, int buttom, int z0)
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
	if (right > gui_screen.width)
        right = gui_screen.width;
	if (buttom > gui_screen.height)
        buttom = gui_screen.height;
    
    layer_t *layer;
    GUI_COLOR color;
    spin_lock(&layer_list_spin_lock);
    /* 刷新高度为[Z0-top]区间的图层 */
    list_for_each_owner (layer, &layer_show_list_head, list) {
        if (layer->z >= z0) {
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
                    if ((color >> 24) & 0xff) {   /* 不是全透明的，就把高度写入到地图中 */
                        layer_map[(screen_y * gui_screen.width + screen_x)] = layer->z;
                    }
                }
            }
        }
    }
    spin_unlock(&layer_list_spin_lock);
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
 * 
 * 调整Z序时需要刷新图层：
 *      当调整到更低的图层时，如果是隐藏图层，那么就刷新最底层到原图层Z的下一层。
 *      不是隐藏图层的话，刷新当前图层上一层到原来图层。
 *      当调整到更高的图层时，就只刷新新图层的高度那一层。
 * @return: 成功返回0，失败返回-1
 */
void layer_set_z(layer_t *layer, int z)
{
    layer_t *tmp = NULL;
    layer_t *old_layer = NULL;
    int old_z = layer->z;
    
    layer_mutex_lock(layer);

    spin_lock(&layer_list_spin_lock);
    /* 已经存在与现实链表中，就说明只是调整一下高度而已。 */
    if (list_find(&layer->list, &layer_show_list_head)) {
        spin_unlock(&layer_list_spin_lock);
#if DEBUG_LOCAL == 1    
        printk("layer z:%d set new z:%d\n", layer->z, z);
#endif
        /* 设置为正，就是要调整高度 */
        if (z >= 0) {
            spin_lock(&layer_val_lock);
    
            /* 修复Z轴 */
            if (z > top_layer_z) {
#if DEBUG_LOCAL == 1                
                printk("layer z:%d set new z:%d but above top %d\n", layer->z, z, top_layer_z);
#endif        
                z = top_layer_z;
                
            }
            /* 如果要调整到最高的图层位置 */
            if (z == top_layer_z) {
                spin_unlock(&layer_val_lock);
    
#if DEBUG_LOCAL == 1                
                printk("layer z:%d set new z:%d same with top %d\n", layer->z, z, top_layer_z);
#endif
                spin_lock(&layer_list_spin_lock);

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
                spin_unlock(&layer_list_spin_lock);

                /* 刷新新图层[z, z] */
                layer_refresh_map(layer->x, layer->y, layer->x + layer->width, layer->y + layer->height, z);
                layer_refresh_by_z(layer->x, layer->y, layer->x + layer->width, layer->y + layer->height, z, z);

            } else {    /* 不是最高图层，那么就和其它图层交换 */
                spin_unlock(&layer_val_lock);
    
                if (z > layer->z) { /* 如果新高度比原来的高度高 */
#if DEBUG_LOCAL == 1
                    printk("layer z:%d < new z:%d \n", layer->z, z, top_layer_z);
#endif
                    spin_lock(&layer_list_spin_lock);

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
#if DEBUG_LOCAL == 1
                        printk("find old layer:%x z%d\n", old_layer, old_layer->z + 1);
#endif                        
                        layer->z = z;
                        list_add_after(&layer->list, &old_layer->list);
                        spin_unlock(&layer_list_spin_lock);

                        /* 刷新新图层[z, z] */
                        layer_refresh_map(layer->x, layer->y, layer->x + layer->width, layer->y + layer->height, z);
                        layer_refresh_by_z(layer->x, layer->y, layer->x + layer->width, layer->y + layer->height, z, z);
                
                    } else {
                        printk("[error ] not found the old layer on %d\n", z);
                        spin_unlock(&layer_list_spin_lock);

                    }
                } else if (z < layer->z) { /* 如果新高度比原来的高度低 */
#if DEBUG_LOCAL == 1                
                    printk("layer z:%d > new z:%d \n", layer->z, z, top_layer_z);
#endif  
                    spin_lock(&layer_list_spin_lock);

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
#if DEBUG_LOCAL == 1
                        printk("find old layer:%x z%d\n", old_layer, old_layer->z - 1);
#endif                        
                        layer->z = z;
                        list_add_before(&layer->list, &old_layer->list);
                        spin_unlock(&layer_list_spin_lock);

                        /* 刷新新图层[z + 1, old z] */
                        layer_refresh_map(layer->x, layer->y, layer->x + layer->width, layer->y + layer->height, z + 1);
                        layer_refresh_by_z(layer->x, layer->y, layer->x + layer->width, layer->y + layer->height, z + 1, old_z);
                    } else {
                        printk("[error ] not found the old layer on %d\n", z);
                        spin_unlock(&layer_list_spin_lock);

                    }
                }
            }
        } else { /* 小于0就是要隐藏起来的图层 */
#if DEBUG_LOCAL == 1
            printk("layer z:%d will be hided.\n", layer->z);
#endif
            spin_lock(&layer_list_spin_lock);

            /* 先从链表中移除 */
            list_del_init(&layer->list);
            spin_unlock(&layer_val_lock);
    
            if (top_layer_z > old_z) {  /* 旧图层必须在顶图层下面 */
                /* 把位于当前图层后面的图层的高度都向下降1 */
                list_for_each_owner (tmp, &layer_show_list_head, list) {
                    if (tmp->z > layer->z) {
                        tmp->z--;
                    }
                }   
            }
            spin_unlock(&layer_list_spin_lock);

            /* 由于隐藏了一个图层，那么，图层顶层的高度就需要减1 */
            top_layer_z--;
            spin_unlock(&layer_val_lock);
    
            /* 刷新图层, [0, layer->z - 1] */
            layer_refresh_map(layer->x, layer->y, layer->x + layer->width, layer->y + layer->height, 0);
            layer_refresh_by_z(layer->x, layer->y, layer->x + layer->width, layer->y + layer->height, 0, old_z - 1);

            layer->z = -1;  /* 隐藏图层后，高度变为-1 */
        }
    } else {    /* 插入新图层 */
        spin_unlock(&layer_list_spin_lock);
        /* 设置为正，就要显示，那么会添加到显示队列 */
        if (z >= 0) {
            spin_lock(&layer_val_lock);
    
            /* 修复Z轴 */
            if (z > top_layer_z) {
                top_layer_z++;      /* 图层顶增加 */
                z = top_layer_z;
#if DEBUG_LOCAL == 1
                printk("insert a layer at top z %d\n", z);
#endif
            } else {
                top_layer_z++;      /* 图层顶增加 */
            }
            
            /* 如果新高度就是最高的图层，就直接插入到图层队列末尾 */
            if (z == top_layer_z) {
                spin_unlock(&layer_val_lock);
    
#if DEBUG_LOCAL == 1
                printk("add a layer %d to tail\n", z);
#endif
                layer->z = z;
                spin_lock(&layer_list_spin_lock);

                /* 直接添加到显示队列 */
                list_add_tail(&layer->list, &layer_show_list_head);
                spin_unlock(&layer_list_spin_lock);

                /* 刷新新图层[z, z] */
                layer_refresh_map(layer->x, layer->y, layer->x + layer->width, layer->y + layer->height, z);
                layer_refresh_by_z(layer->x, layer->y, layer->x + layer->width, layer->y + layer->height, z, z);

            } else {
                spin_unlock(&layer_val_lock);
    
#if DEBUG_LOCAL == 1
                printk("add a layer %d to midlle or head\n", z);
#endif               
                spin_lock(&layer_list_spin_lock);

                /* 查找和当前图层一样高度的图层 */
                list_for_each_owner(tmp, &layer_show_list_head, list) {
                    if (tmp->z == z) {
                        old_layer = tmp;
                        break;
                    }
                }
                tmp = NULL;
                if (old_layer) {    /* 找到一个旧图层 */
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
                    printk("[error ] not found old layer!\n");
                }
                spin_unlock(&layer_list_spin_lock);

                /* 刷新新图层[z, z] */
                layer_refresh_map(layer->x, layer->y, layer->x + layer->width, layer->y + layer->height, z);
                layer_refresh_by_z(layer->x, layer->y, layer->x + layer->width, layer->y + layer->height, z, z);

            }
        }
        /* 小于0就是要隐藏起来的图层，但是由于不在图层链表中，就不处理 */
    }
    layer_mutex_unlock(layer);

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
    spin_lock(&layer_list_spin_lock);
    /* 先从链表中删除 */
    list_del_init(&layer->global_list);
    spin_unlock(&layer_list_spin_lock);
    layer_mutex_lock(layer);
    
    if (layer->flags & LAYER_EXT_BUF) {
        mem_cache_free_object(&layer_buffer_memcache, layer->buffer);
    } else {
        /* 释放缓冲区 */
        kfree(layer->buffer);
    }
    layer->buffer = NULL;
    layer_mutex_unlock(layer);

    /* 释放图层 */
    kfree(layer);
    return 0;
}

/**
 * print_layers - 打印所有图层信息
 * 
 */
void print_layers()
{
#if DEBUG_LOCAL == 1
    printk("layer top z:%d\n", top_layer_z);
#endif
    layer_t *layer;
    list_for_each_owner (layer, &layer_show_list_head, list) {
#if DEBUG_LOCAL == 1
        printk("layer addr:%x buffer:%x width:%d height:%d x:%d y:%d z:%d\n",
            layer, layer->buffer, layer->width, layer->height, layer->x, layer->y, layer->z);
#endif
    }
}

/**
 * layer_refresh_by_z - 刷新图层
 * 
 * 刷新某个区域的[z0-z1]之间的图层，相当于在1一个3d空间中刷新
 * 某个矩形区域。（有点儿抽象哦，铁汁~）
 * 
 * 参照layer_map中的图层编号，进行像素写入
 */
void layer_refresh_by_z(int left, int top, int right, int buttom, int z0, int z1)
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
	if (right > gui_screen.width)
        right = gui_screen.width;
	if (buttom > gui_screen.height)
        buttom = gui_screen.height;
    
    layer_t *layer;
    GUI_COLOR color;

    spin_lock(&layer_list_spin_lock);
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
                    /* 照着map中的z进行刷新 */			
                    if (layer_map[(screen_y * gui_screen.width + screen_x)] == layer->z) {
                        /* 获取图层中的颜色 */
                        color = layer->buffer[layer_y * layer->width + layer_x];
                        /* 写入到显存 */
                        gui_screen.output_pixel(screen_x, screen_y, gui_screen.gui_to_screen_color(color));
                    }
                }
            }
        }
    }
    spin_unlock(&layer_list_spin_lock);
}

/**
 * layer_refresh - 刷新图层
 * 
 * 刷新某个区域的图层
 * 
 */
void layer_refresh(layer_t *layer, int left, int top, int right, int buttom)
{
    layer_mutex_lock(layer);
    if (layer->z >= 0) {
        layer_refresh_map(layer->x + left, layer->y + top, layer->x + right,
            layer->y + buttom, layer->z);
        layer_refresh_by_z(layer->x + left, layer->y + top, layer->x + right,
            layer->y + buttom, layer->z, layer->z);
    }
    layer_mutex_unlock(layer);
}

/**
 * 刷新图层以及其下面的所有图层
 */
void layer_refresh_under(layer_t *layer, int left, int top, int right, int buttom)
{
    layer_mutex_lock(layer);
    if (layer->z >= 0) {
        layer_refresh_map(layer->x + left, layer->y + top, layer->x + right,
            layer->y + buttom, 0);
        layer_refresh_by_z(layer->x + left, layer->y + top, layer->x + right,
            layer->y + buttom, 0, layer->z);
    }
    layer_mutex_unlock(layer);
}

void layer_refresh_under_rect(layer_t *layer, int x, int y, uint32_t width, uint32_t height)
{
    layer_refresh_under(layer, x, y, x + width, y + height);
}

void layer_refresh_rect(layer_t *layer, int x, int y, uint32_t width, uint32_t height)
{
    layer_refresh(layer, x, y, x + width, y + height);
}

void layer_refresh_all()
{
    /* 从底刷新到顶 */
    layer_t *layer;
    list_for_each_owner (layer, &layer_show_list_head, list) {
        layer_refresh(layer, 0, 0, layer->width, layer->height);
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
    layer_mutex_lock(layer);

    /* 显示中的图层才可以刷新 */
    layer->x = x;
    layer->y = y;
    if (layer->z >= 0) {
        /* 刷新原来位置 */
        layer_refresh_map(old_x, old_y, old_x + layer->width, old_y + layer->height, 0);
        /* 刷新新位置 */
        layer_refresh_map(x, y, x + layer->width, y + layer->height, layer->z);

        /* 刷新原来位置 */
        layer_refresh_by_z(old_x, old_y, old_x + layer->width, old_y + layer->height, 0, layer->z - 1);
        /* 刷新新位置 */
        layer_refresh_by_z(x, y, x + layer->width, y + layer->height, layer->z, layer->z);
    }
    layer_mutex_unlock(layer);

}

/**
 * 重新设置图层的大小，并擦除之前的显示内容
 * 
 */
int layer_reset_size(layer_t *layer, int x, int y, uint32_t width, uint32_t height)
{
    if (!layer || !width || !height)
        return -1;

    if (!layer->buffer) {
        return -1;
    }
    char ext_buf = 0;
    size_t bufsz = width * height * sizeof(GUI_COLOR);
    uint32_t *buffer = kmalloc(bufsz);
    if (buffer == NULL) {
        ext_buf = 1;
        buffer = mem_cache_alloc_object(&layer_buffer_memcache);
        if (buffer == NULL) {
            return -1;
        }
    }

    /* 先将原来位置里面的内容绘制成透明 */
    layer_draw_rect_fill(layer, 0, 0, layer->width, layer->height, COLOR_NONE);
    /* 重新设置位置才能完整刷新图层 */
    layer_set_xy(layer, x, y);
    layer_mutex_lock(layer);
    if (layer->flags & LAYER_EXT_BUF) {
        mem_cache_free_object(&layer_buffer_memcache, layer->buffer);
        layer->flags &= ~LAYER_EXT_BUF;
    } else {
        /* 重新绑定缓冲区 */
        kfree(layer->buffer);
    }
    layer->buffer = NULL;
    if (ext_buf) {
        layer->flags |= LAYER_EXT_BUF;
    }
    layer->buffer = buffer;
    layer->width = width;
    layer->height = height;
    layer_mutex_unlock(layer);

    return 0;
}

/**
 * 获取顶层窗口，就是一个非窗口图层，但是又是最高窗口图层的上一个图层
 * 往往通过这个图层来确定下一个新插入的窗口图层的高度
 */
layer_t *layer_get_win_top()
{
    return layer_wintop;
}
/**
 * 这是一个非窗口图层，但是又是最高窗口图层的上一个图层
 * 当初始化完一般图层后，需要设置这个图层，才能进行后续工作。
 * 这个需要用户态程序配合。
 */
int layer_set_win_top(layer_t *layer)
{
    if (!layer)
        return -1;
    spin_lock(&layer_val_lock);
    layer_wintop = layer;
    spin_unlock(&layer_val_lock);
    
    return 0;
}

/**
 * 获取桌面图层，桌面图层是用来显示桌面内容的，并且是窗口的统领者
 */
layer_t *layer_get_desktop()
{
    return layer_desktop;
}
/**
 * 设置桌面图层，桌面图层是用来显示桌面内容的，并且是窗口的统领者
 */
int layer_set_desktop(layer_t *layer)
{
    if (!layer)
        return -1;
    spin_lock(&layer_val_lock);
    layer_desktop = layer;
    spin_unlock(&layer_val_lock);
    
    return 0;
}

/**
 * 设置图层为聚焦图层
 */
void layer_set_focus(layer_t *layer)
{
    spin_lock(&layer_val_lock);
    
    layer_focused = layer;
    spin_unlock(&layer_val_lock);
    
}

/**
 * 获取聚焦图层
 */
layer_t *layer_get_focus()
{
    return layer_focused;
}

/**
 * 设置图层的标志
 */
int layer_set_flags(layer_t *layer, uint32_t flags)
{
    if (!layer)
        return -1;
    layer_mutex_lock(layer);
    layer->flags |= flags;
    layer_mutex_unlock(layer);
    return 0;
}

/**
 * 尝试聚焦到某个图层
 * 
 * 如果有之前聚焦的图层，那么就需要对它丢焦
 * 如果新聚焦图层是窗口，那么就需要将它切换到窗口顶部
 * 再发送消息给新聚焦图层
 */
int layer_try_focus(layer_t *layer)
{
    layer_t *focused = layer_get_focus();
    int val = -1;
    if (focused != layer) {    /* 旧聚焦的图层 */
        g_msg_t m;
        memset(&m, 0, sizeof(g_msg_t));
        task_t *task;
        
        if (focused) {  /* 有聚焦图层才进行聚焦 */
            task = (task_t *)focused->extension;
            if (task) {
                /* 发送失焦消息给旧聚焦图层 */
                m.id        = GM_LOST_FOCUS;
                m.target    = focused->id;
                m.data0     = 0;    /* 0表示发送者为内核 */
                val = msgpool_try_push(task->gmsgpool, &m);
            }
        }

        layer_set_focus(layer);
        /* 将聚焦图层设置为窗口最高 */
        if (layer->flags & LAYER_WINDOW) {   /* 是窗口图层 */
            /* 把图层切换到最高窗口图层的下面 */
            layer_set_z(layer, sys_layer_get_win_top()-1);
        }
        task = (task_t *)layer->extension;
        if (task) {
            /* 发送消息给新聚焦图层 */
            m.id        = GM_GET_FOCUS;
            m.target    = layer->id;
            m.data0     = 0;    /* 0表示发送者为内核 */
            val = msgpool_try_push(task->gmsgpool, &m);
        }
    }
    return val;
}
/**
 * 尝试调整图层大小，并发送RESIZE消息给指定图层
 * 
*/
int layer_try_resize(layer_t *layer, gui_rect_t *prect)
{
    if (!layer || !prect)
        return -1;
    if (layer_reset_size(layer, prect->x, prect->y, prect->width, prect->height) < 0)
        return -1;
    task_t *task = (task_t *)layer->extension;
    if (task) {
        /* 发送消息 */
        g_msg_t m;
        memset(&m, 0, sizeof(g_msg_t));
        m.id        = GM_RESIZE;
        m.target    = layer->id;
        m.data0     = prect->x;
        m.data1     = prect->y;
        m.data2     = prect->width;
        m.data3     = prect->height;
        return msgpool_try_push(task->gmsgpool, &m);
    }
    return -1;
}

/**
 * 计算重新调整大小后的图层，并把结果返回到一个矩形区域中
 * 
 * 有调整返回0，没有返回-1
 */
int layer_calc_resize(layer_t *layer, int mx, int my, gui_rect_t *out_rect)
{
    if (!layer)
        return -1;

    gui_point_t point;
    point.x = mx - gui_mouse.click_point.x;
    point.y = my - gui_mouse.click_point.y;
    /* 没有移动，没有调整 */
    if (!point.x && !point.y) {
        return -1;
    }
    
    gui_point_t center;
    center.x = layer->width / 2 + layer->x;
    center.y = layer->height / 2 + layer->y;
    
    /* 由于调整后宽高可能为负，所以要用支持负数的矩形 */
    gui_rect_ex_t rect;
    memset(&rect, 0, sizeof(gui_rect_t));
    
    switch (gui_mouse.state)
    {
    case MOUSE_VRESIZE:
        /* 上边调整 */
        if (gui_mouse.click_point.y < center.y) {
            rect.x = layer->x;
            rect.y = layer->y + point.y;
            rect.width = layer->width;
            rect.height = layer->height + -point.y;
        } else {    /* 下边调整 */
            rect.x = layer->x;
            rect.y = layer->y;
            rect.width = layer->width;
            rect.height = layer->height + point.y;
        }
        break;
    case MOUSE_HRESIZE:
        /* 左边调整 */
        if (gui_mouse.click_point.x < center.x) {
            rect.x = layer->x + point.x;
            rect.y = layer->y;
            rect.width = layer->width + -point.x;
            rect.height = layer->height;
        } else {    /* 右边调整 */
            rect.x = layer->x;
            rect.y = layer->y;
            rect.width = layer->width + point.x;
            rect.height = layer->height;
        }
        break;
    case MOUSE_DRESIZE1:
        /* 左边调整 */
        if (gui_mouse.click_point.x < center.x) {
            rect.x = layer->x + point.x;
            rect.y = layer->y;
            rect.width = layer->width + -point.x;
            rect.height = layer->height + point.y;
        } else {    /* 右边调整 */
            rect.x = layer->x;
            rect.y = layer->y + point.y;
            rect.width = layer->width + point.x;
            rect.height = layer->height + -point.y;
        }
        break;
    case MOUSE_DRESIZE2:
        /* 左边调整 */
        if (gui_mouse.click_point.x < center.x) {
            rect.x = layer->x + point.x;
            rect.y = layer->y + point.y;
            rect.width = layer->width + -point.x;
            rect.height = layer->height + -point.y;
        } else {    /* 右边调整 */
            rect.x = layer->x;
            rect.y = layer->y;
            rect.width = layer->width + point.x;
            rect.height = layer->height + point.y;
        }
        break;        
    default:
        return -1;
    }
    /* 对调整后的矩形进行判断，看是否符合要求 */
    if (rect.width <= 0 || rect.height <= 0)
    {
        return -1;  /* invalid rect */
    }
    
    if (gui_region_valid(&layer->resizemin_rg)) {
        if (rect.width < layer->resizemin_rg.right) {
            rect.width = layer->resizemin_rg.right;
        }
        if (rect.height < layer->resizemin_rg.bottom) {
            rect.height = layer->resizemin_rg.bottom;
        }
    }
    *out_rect = *((gui_rect_t *)&rect); /* 转换矩形 */
    return 0;
}

/**
 * 派发鼠标的过滤消息
 * 
 * 如果成功过滤返回0，没有过滤就返回-1，表示消息还需要进一步处理
 */
static int gui_dispatch_mouse_filter_msg(g_msg_t *msg)
{
    /* 按下鼠标左键，并且在调整图层大小或者拖拽图层，就要截断消息 */
    if (msg->id == GM_MOUSE_LBTN_DOWN) {
        if (gui_mouse.resize_layer) /* 有调整大小图层时不允许产生鼠标左键单击消息 */
            return 0;
        if (gui_mouse.drag_layer) /* 有拖拽图层时不允许产生鼠标左键单击消息 */
            return 0;
    }
    
    if (msg->id == GM_MOUSE_LBTN_UP) {
        if (gui_mouse.resize_layer) {
            gui_rect_t rect;
            if (!layer_calc_resize(gui_mouse.resize_layer, msg->data0, msg->data1, &rect)) {
                /* 发送一个调整大小消息 */
                #ifdef LAYER_DEBUG
                printk("[gui]: up -> layer resize from (%d, %d), (%d, %d)",
                    gui_mouse.resize_layer->x, gui_mouse.resize_layer->y,
                    gui_mouse.resize_layer->width, gui_mouse.resize_layer->height);
                printk(" to (%d, %d), (%d, %d)\n",
                    rect.x, rect.y, rect.width, rect.height);
                #endif /* LAYER_DEBUG */
                layer_try_resize(gui_mouse.resize_layer, &rect);
                
                #ifdef CONFIG_SHADE_LAYER
                layer_set_z(gui_mouse.shade, -1); /* 隐藏遮罩图层 */
                #endif /* CONFIG_SHADE_LAYER */
            }
            gui_mouse.resize_layer = NULL;
            /* 设置鼠标状态 */
            gui_mouse_set_state(MOUSE_NORMAL);
            gui_mouse.click_point.x = -1;
            gui_mouse.click_point.y = -1;      
            return 0;
        }

        if (gui_mouse.drag_layer) { /* 处于抓取时弹起 */
            if (gui_mouse.state == MOUSE_HOLD) {
                #ifdef CONFIG_SHADE_LAYER
                layer_set_z(gui_mouse.shade, -1); /* 隐藏遮罩图层 */
                /* 设置抓取窗口的位置 */
                layer_set_xy(gui_mouse.drag_layer, gui_mouse.shade_rect.x,
                    gui_mouse.shade_rect.y);
                #endif /* CONFIG_SHADE_LAYER */

                /* 发送窗口移动消息 */
                task_t *task = (task_t *)gui_mouse.drag_layer->extension;
                if (task) {
                    g_msg_t m;
                    memset(&m, 0, sizeof(g_msg_t));
                    m.id = GM_MOVE;
                    m.target = gui_mouse.drag_layer->id;
                    m.data0 = gui_mouse.drag_layer->x;
                    m.data1 = gui_mouse.drag_layer->y;
                    msgpool_try_push(task->gmsgpool, &m);
                }
                
            }
            gui_mouse.drag_layer = NULL;

            /* 设置鼠标状态 */
            gui_mouse_set_state(MOUSE_NORMAL);
            return -1; /* 消息还需要进一步处理 */
        }
    }
    if (msg->id == GM_MOUSE_MOTION) {
        if (gui_mouse.resize_layer) {
            #ifdef CONFIG_SHADE_LAYER
            /* 擦除上一次绘制的内容 */
            if (gui_rect_valid(&gui_mouse.shade_rect)) {
                gui_draw_shade_layer(gui_mouse.shade, &gui_mouse.shade_rect, 0);
            }
            #endif
            
            /* 实时调整大小 */
            gui_rect_t rect;
            if (!layer_calc_resize(gui_mouse.resize_layer, msg->data0, msg->data1, &rect)) {
                #ifdef LAYER_DEBUG
                printk("[gui]: layer resize from (%d, %d), (%d, %d)",
                    gui_mouse.resize_layer->x, gui_mouse.resize_layer->y,
                    gui_mouse.resize_layer->width, gui_mouse.resize_layer->height);
                printk(" to (%d, %d), (%d, %d)\n",
                    rect.x, rect.y, rect.width, rect.height);     
                #endif /* LAYER_DEBUG */
                #ifdef CONFIG_SHADE_LAYER
                gui_draw_shade_layer(gui_mouse.shade, &rect, 1); /* 绘制新内容 */
                gui_rect_copy(&gui_mouse.shade_rect, &rect); /* 保存新区域 */
                if (gui_mouse.shade->z < 0) { /* 没显示就显示 */
                    layer_set_z(gui_mouse.shade, sys_layer_get_win_top());
                }
                #endif /* CONFIG_SHADE_LAYER */

                return 0;
            }
        }

        if (gui_mouse.drag_layer) {
            int wx = gui_mouse.x - gui_mouse.local_x;
            int wy = gui_mouse.y - gui_mouse.local_y;
            #ifdef CONFIG_SHADE_LAYER
            /* 擦除上一次绘制的内容 */
            if (gui_rect_valid(&gui_mouse.shade_rect)) {
                gui_draw_shade_layer(gui_mouse.shade, &gui_mouse.shade_rect, 0);
            }
            #endif

            gui_rect_t rect;
            rect.x = wx;
            rect.y = wy;
            rect.width = gui_mouse.drag_layer->width;
            rect.height = gui_mouse.drag_layer->height;
            #ifdef CONFIG_SHADE_LAYER
            gui_draw_shade_layer(gui_mouse.shade, &rect, 1); /* 绘制新内容 */
            gui_rect_copy(&gui_mouse.shade_rect, &rect); /* 保存新区域 */
            if (gui_mouse.shade->z < 0) { /* 没显示就显示 */
                layer_set_z(gui_mouse.shade, sys_layer_get_win_top());
            }
            #endif /* CONFIG_SHADE_LAYER */

            gui_mouse_set_state(MOUSE_HOLD);

            #ifdef CONFIG_WAKER_LAYER
            /* 移动的是游走窗口 */
            layer_set_xy(gui_mouse.walker, wx, wy);
            #else
            //layer_set_xy(gui_mouse.drag_layer, wx, wy);
            #endif /* CONFIG_WAKER_LAYER */
            
            return 0;
        }
    }
    return -1;  /* 该消息需要进一步处理 */
}

static void gui_mouse_hover_action(layer_t *layer, g_msg_t *msg, int lcmx, int lcmy)
{
    /* 进入某个图层 */
    if (gui_mouse.hover != layer && gui_mouse.hover) { /* 从其他图层进入当前图层 */
        /* 对于layer来说，就是进入*/
        task_t *task = (task_t *)layer->extension;
        if (task) {
            g_msg_t m;
            m.id        = GM_LAYER_ENTER;
            m.target    = layer->id;
            m.data0     = lcmx;
            m.data1     = lcmy;
            m.data2     = msg->data0;
            m.data3     = msg->data1;
            msgpool_try_push(task->gmsgpool, &m);

            task = (task_t *) gui_mouse.hover->extension;
            if (task) {/* 对于hover来说，就是离开 */
                m.id        = GM_LAYER_LEAVE;
                m.target    = gui_mouse.hover->id;
                m.data0     = msg->data0 - gui_mouse.hover->x;
                m.data1     = msg->data1 - gui_mouse.hover->y;
                msgpool_try_push(task->gmsgpool, &m);

                /* 如果进入了不同的图层，那么，当为调整图层大小时，就需要取消这个状态 */
                if (gui_mouse.state != MOUSE_NORMAL) {
                    gui_mouse_set_state(MOUSE_NORMAL);
                    //printk("resize layer %d out!\n", layer->id);
                    gui_mouse.click_point.x = -1;
                    gui_mouse.click_point.y = -1;
                }
            }
        }
    }
    gui_mouse.hover = layer;    /* 悬停在某个图层 */
    
}

static int gui_mouse_resize_action(layer_t *layer, g_msg_t *msg, int lcmx, int lcmy)
{
    /* 图层调整大小处理 */
    if (gui_region_valid(&layer->resize_rg)) {
        /* 不在区域里面才能拖拽 */
        if (!gui_region_in(&layer->resize_rg, lcmx, lcmy)) {
            /* 记录调整大小的图层 */
            if (msg->id == GM_MOUSE_LBTN_DOWN) {
                gui_mouse.resize_layer = layer;
                gui_mouse.click_point.x = msg->data0;
                gui_mouse.click_point.y = msg->data1;
                //printk("resize layer %d in!\n", layer->id);
            }
            /* 根据鼠标位置设置不同的方向 */
            if (lcmx < layer->resize_rg.left) {
                if (lcmy < layer->resize_rg.top)
                    gui_mouse_set_state(MOUSE_DRESIZE2);
                else if (lcmy >= layer->resize_rg.bottom)
                    gui_mouse_set_state(MOUSE_DRESIZE1);
                else
                    gui_mouse_set_state(MOUSE_HRESIZE);
            } else if (lcmx >= layer->resize_rg.right) {
                if (lcmy < layer->resize_rg.top)
                    gui_mouse_set_state(MOUSE_DRESIZE1);
                else if (lcmy >= layer->resize_rg.bottom)
                    gui_mouse_set_state(MOUSE_DRESIZE2);
                else
                    gui_mouse_set_state(MOUSE_HRESIZE);
            } else if (lcmy < layer->resize_rg.top) {
                gui_mouse_set_state(MOUSE_VRESIZE);
            } else if (lcmy >= layer->resize_rg.bottom) {
                gui_mouse_set_state(MOUSE_VRESIZE);
            }
            return 0;
        } else {
            if (gui_mouse.state != MOUSE_NORMAL) {
                gui_mouse_set_state(MOUSE_NORMAL);
                //printk("resize layer %d out! #2\n", layer->id);
                gui_mouse.click_point.x = -1;
                gui_mouse.click_point.y = -1;
            }
            
        }
    }
    return -1;
}

static void gui_mouse_move_action(layer_t *layer, g_msg_t *msg, int lcmx, int lcmy)
{
    /* 图层移动处理 */
    if (gui_region_valid(&layer->drag_rg)) {
        if (gui_region_in(&layer->drag_rg, lcmx, lcmy)) {
            if (msg->id == GM_MOUSE_LBTN_DOWN) {
                /* 开始拖拽 */
                gui_mouse.local_x = lcmx;
                gui_mouse.local_y = lcmy;
                gui_mouse.drag_layer = layer;
                /* 设置为普通状态 */
                gui_mouse_set_state(MOUSE_NORMAL);
            }
        }
    }
}

/**
 * 派发鼠标消息
 */
int gui_dispatch_mouse_msg(g_msg_t *msg)
{
    if (!gui_dispatch_mouse_filter_msg(msg))
        return 0;

    layer_t *layer;
    int local_mx, local_my;
    /* 从上往下检测鼠标所在图层，并进行对应的事件处理 */
    list_for_each_owner_reverse (layer, &layer_show_list_head, list) {
        if (layer == gui_mouse.layer)
            continue;
        if (layer->extension == NULL)
            continue;
        local_mx = msg->data0 - layer->x;
        local_my = msg->data1 - layer->y;
        if (local_mx >= 0 && local_mx < layer->width && 
            local_my >= 0 && local_my < layer->height) {
            
            /* 如果是在图层上点击了鼠标左键，那么就进行聚焦 */
            if (msg->id == GM_MOUSE_LBTN_DOWN) {
                layer_try_focus(layer);
            }
            
            gui_mouse_hover_action(layer, msg, local_mx, local_my);
            if (!gui_mouse_resize_action(layer, msg, local_mx, local_my))
                return 0;
            gui_mouse_move_action(layer, msg, local_mx, local_my);
            
            /* 发送消息给目标图层 */
            g_msg_t m;
            task_t *task = (task_t *)layer->extension;
            m.id        = msg->id;
            m.target    = layer->id;
            m.data0     = local_mx;
            m.data1     = local_my;
            m.data2     = msg->data0;
            m.data3     = msg->data1;
            msgpool_try_push(task->gmsgpool, &m);
            break;
        }
    }
    return 0;
}

int gui_dispatch_key_msg(g_msg_t *msg)
{
    spin_lock(&layer_val_lock);
    layer_t *layer = layer_focused;
    spin_unlock(&layer_val_lock);

    int val = -1;
    /* 发送给聚焦图层 */
    if (layer) {
        if (layer->extension == NULL)
            return -1;
        
        /* 发送消息 */
        g_msg_t m;
        memset(&m, 0, sizeof(g_msg_t));
        m.id        = msg->id;
        m.target    = layer->id;
        m.data0     = msg->data0;
        m.data1     = msg->data1;
        task_t *task = (task_t *)layer->extension;
        val = msgpool_try_push(task->gmsgpool, &m);
    }
    return val;
}

int gui_dispatch_target_msg(g_msg_t *msg)
{
    /* 检测目标是否存在 */
    layer_t *layer = layer_find_by_id(msg->target);
    if (layer == NULL) {
        return -1;
    }
    int val = -1;
    if (layer) {
        if (layer->extension == NULL)
            return -1;
        /* 转发-发送消息 */
        task_t *task = (task_t *)layer->extension;
        val = msgpool_try_push(task->gmsgpool, msg);
    }
    return val;
}

int layer_focus_win_top()
{
    /* 聚焦到最高的窗口图层 */
    layer_t *top = layer_get_win_top();
    if (top) {    
        layer_t *focus = layer_find_by_z(top->z - 1);
        if (focus) {
            return layer_try_focus(focus);
        }
    }
    return -1;
}

int gui_init_layer()
{
    size_t maxsz = gui_screen.width * gui_screen.height * sizeof(uint16_t);
    /* 分配地图空间 */
    layer_map = kmalloc(maxsz);
    if (layer_map == NULL) {
        return -1;
    }
    memset(layer_map, 0, maxsz);

    INIT_LIST_HEAD(&layer_show_list_head);
    INIT_LIST_HEAD(&layer_list_head);

    layer_focused = NULL;
    
    maxsz = gui_screen.width * gui_screen.height * sizeof(uint32_t) + PAGE_SIZE;
    mem_cache_init(&layer_buffer_memcache, "layer_buffer", maxsz, 0);
    
    if (init_mouse_layer() < 0) {
        kfree(layer_map);
        return -1;
    }
    /*
    layer_t *test = create_layer(300, 200);
    layer_set_z(test, 0);
    layer_draw_rect_fill(test, 10, 10, 1, 1, COLOR_RED);
    layer_draw_rect(test, 10, 20, 1, 1, COLOR_RED);
    layer_refresh_rect(test, 10, 10, 1, 1);
    spin("test");*/

    return 0;
}

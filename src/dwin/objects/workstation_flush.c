#include <dwin/objects.h>
#include <dwin/dwin.h>
#include <dwin/hal.h>
#include <dwin/workstation.h>

static void flush_map(dwin_workstation_t *station, int left, int top, int right, int buttom, int z0)
{
    int layer_left, layer_top, layer_right, layer_buttom; // 视图内部坐标位置
    int screen_x, screen_y;
    int layer_x, layer_y;

    if (left < 0)
        left = 0;
    if (top < 0)
        top = 0;
    if (right > station->width)
        right = station->width;
    if (buttom > station->height)
        buttom = station->height;
    
    dwin_layer_t *layer;
    uint32_t *colors;

    uint32_t *src;
    dwin_layer_id_map_t *map;
    
    dwin_critical_t crit;
    dwin_enter_critical(crit);

    /* 刷新高度为[z0-top]区间的视图 */
    list_for_each_owner (layer, &station->show_list_head, list)
    {
        if (layer->z >= z0)
        {
            layer_left = left - layer->x;
            layer_top = top - layer->y;
            layer_right = right - layer->x;
            layer_buttom = buttom - layer->y;
            if (layer_left < 0)
            {
                layer_left = 0;
            }
            if (layer_top < 0)
            {
                layer_top = 0;
            }
            if (layer_right > layer->width)
            {
                layer_right = layer->width;
            } 
            if (layer_buttom > layer->height)
            {
                layer_buttom = layer->height;
            }

            colors = (uint32_t *)layer->buffer;
            /* 进入循环前进行位置预判，然后调整位置 */
            // layer_top
            screen_y = layer->y + layer_top;
            if (screen_y < 0)
            {
                layer_top += -screen_y;
            }
            if (screen_y >= station->height)
            {
                continue;
            }
            // layer_buttom
            screen_y = layer->y + layer_buttom;
            if (screen_y >= station->height)
            {
                layer_buttom -= screen_y - station->height;
            }

            // layer_left
            screen_x = layer->x + layer_left;
            if (screen_x < 0)
            {
                layer_left += -screen_x;
            }
            if (screen_x >= station->width)
            {
                continue;
            }
            // layer_right
            screen_x = layer->x + layer_right;
            if (screen_x < 0)
            {
                layer_right -= screen_x - station->width;
            }
            
            for(layer_y = layer_top; layer_y < layer_buttom; layer_y++)
            {
                screen_y = layer->y + layer_y;
                src = &((uint32_t *) colors)[layer_y * layer->width]; 
                map = &station->id_map[(screen_y * station->width + layer->x)];
                for(layer_x = layer_left; layer_x < layer_right; layer_x++)
                {
                    /* 不是全透明的，就把视图标识写入到映射表中 */
                    if ((src[layer_x] >> 24) & 0xff)
                    {
                        map[layer_x] = layer->z;
                    }
                }
            }
        }
    }

    dwin_leave_critical(crit);
}

static inline void flush_bits32(dwin_layer_t *layer, int layer_left, int layer_top, int layer_right, int layer_buttom)
{
    /* 在屏幕上的位置 */
    int screen_y;
    /* 在图层上的位置 */
    int layer_x, layer_y;
    uint32_t *dst;
    uint32_t *src;
    dwin_layer_id_map_t *map;
    dwin_workstation_t *station = layer->workstation;
    
    /* 优化整个块 */
    for (layer_y = layer_top; layer_y < layer_buttom; layer_y++)
    {
        screen_y = layer->y + layer_y;
        dst = &((uint32_t *)dwin_hal->lcd->parent.vram_start)[station->width * screen_y + layer->x];
        src = &((uint32_t *)layer->buffer)[layer_y * layer->width]; 
        map = &station->id_map[(screen_y * station->width + layer->x)];

        for (layer_x = layer_left; layer_x < layer_right; layer_x++)
        {
            /* 照着map中的z进行刷新 */
            if (map[layer_x] == layer->z)
            {
                /* 获取图层中的颜色 */
                dst[layer_x] = src[layer_x];
                
            }
        }
    }
}

static inline void flush_bits24(dwin_layer_t *layer, int layer_left, int layer_top, int layer_right, int layer_buttom)
{
    /* 在屏幕上的位置 */
    int screen_y;
    /* 在图层上的位置 */
    int layer_x, layer_y;
    uint8_t *dst;
    uint32_t *src;
    dwin_layer_id_map_t *map;
    dwin_workstation_t *station = layer->workstation;
    
    /* 优化整个块 */
    for (layer_y = layer_top; layer_y < layer_buttom; layer_y++)
    {
        screen_y = layer->y + layer_y;
        dst = &((uint8_t *)dwin_hal->lcd->parent.vram_start)[(station->width * screen_y + layer->x) * 3];
        src = &((uint32_t *) layer->buffer)[layer_y * layer->width]; 
        map = &station->id_map[(screen_y * station->width + layer->x)];
        
        for (layer_x = layer_left; layer_x < layer_right; layer_x++)
        {
            /* 照着map中的z进行刷新 */
            if (map[layer_x] == layer->z)
            {
                /* 获取图层中的颜色 */
                dst[layer_x * 3 + 0] = src[layer_x] & 0xFF;
                dst[layer_x * 3 + 1] = (src[layer_x] & 0xFF00) >> 8;
                dst[layer_x * 3 + 2] = (src[layer_x] & 0xFF0000) >> 16;
            }
        }
    }
}

static inline void flush_bits16(dwin_layer_t *layer, int layer_left, int layer_top, int layer_right, int layer_buttom)
{
    /* 在屏幕上的位置 */
    int screen_y;
    /* 在图层上的位置 */
    int layer_x, layer_y;
    uint16_t *dst;
    uint32_t *src;
    dwin_layer_id_map_t *map;
    dwin_workstation_t *station = layer->workstation;

    /* 优化整个块 */
    for (layer_y = layer_top; layer_y < layer_buttom; layer_y++)
    {
        screen_y = layer->y + layer_y;
        dst = &((uint16_t *)dwin_hal->lcd->parent.vram_start)[(station->width * screen_y + layer->x)];
        src = &((uint32_t *) layer->buffer)[layer_y * layer->width]; 
        map = &station->id_map[(screen_y * station->width + layer->x)];
        
        for (layer_x = layer_left; layer_x < layer_right; layer_x++)
        {
            /* 照着map中的z进行刷新 */
            if (map[layer_x] == layer->z) {
                /* 获取图层中的颜色 */
                dst[layer_x] = (uint16_t)((src[layer_x] &0xF8) >> 3) | ((src[layer_x] &0xFC00) >> 5) | ((src[layer_x] &0xF80000) >> 8);
            }
        }
    }
}

static inline void flush_bits15(dwin_layer_t *layer, int layer_left, int layer_top, int layer_right, int layer_buttom)
{
    /* 在屏幕上的位置 */
    int screen_y;
    /* 在图层上的位置 */
    int layer_x, layer_y;
    uint16_t *dst;
    uint32_t *src;
    dwin_layer_id_map_t *map;
    dwin_workstation_t *station = layer->workstation;

    /* 优化整个块 */
    for (layer_y = layer_top; layer_y < layer_buttom; layer_y++)
    {
        screen_y = layer->y + layer_y;
        dst = &((uint16_t *)dwin_hal->lcd->parent.vram_start)[(station->width * screen_y + layer->x)];
        src = &((uint32_t *) layer->buffer)[layer_y * layer->width]; 
        map = &station->id_map[(screen_y * station->width + layer->x)];
        
        for (layer_x = layer_left; layer_x < layer_right; layer_x++)
        {
            
            /* 照着map中的z进行刷新 */
            if (map[layer_x] == layer->z)
            {
                /* 获取图层中的颜色 */
                dst[layer_x] = (uint16_t)((src[layer_x] &0xF8) >> 3) | ((src[layer_x] &0xF800) >> 6) | ((src[layer_x] &0xF80000) >> 9);
            }
        }
    }
}

static inline void flush_bits8(dwin_layer_t *layer, int layer_left, int layer_top, int layer_right, int layer_buttom)
{
    /* 在屏幕上的位置 */
    int screen_y;
    /* 在图层上的位置 */
    int layer_x, layer_y;
    uint8_t *dst;
    uint32_t *src;
    dwin_layer_id_map_t *map;
    dwin_workstation_t *station = layer->workstation;

    /* 优化整个块 */
    for (layer_y = layer_top; layer_y < layer_buttom; layer_y++)
    {
        screen_y = layer->y + layer_y;
        dst = &((uint8_t *)dwin_hal->lcd->parent.vram_start)[(station->width * screen_y + layer->x)];
        src = &((uint32_t *) layer->buffer)[layer_y * layer->width]; 
        map = &station->id_map[(screen_y * station->width + layer->x)];
        
        for (layer_x = layer_left; layer_x < layer_right; layer_x++)
        {
            /* 照着map中的z进行刷新 */
            if (map[layer_x] == layer->z)
            {
                /* 获取图层中的颜色 */
                dst[layer_x] = (uint8_t)((src[layer_x] &0xC0) >> 6) | ((src[layer_x] &0xE000) >> 11) | ((src[layer_x] &0xE00000) >> 16);
            }
        }
    }
}

static void flush_by_z(dwin_workstation_t *workstation, int left, int top, int right, int buttom, int z0, int z1)
{
    int view_left, view_top, view_right, view_buttom;

    if (left < 0)
        left = 0;
    if (top < 0)
        top = 0;
    if (right > workstation->width)
        right = workstation->width;
    if (buttom > workstation->height)
        buttom = workstation->height;

    dwin_layer_t *layer;

    dwin_critical_t crit;
    dwin_enter_critical(crit);

    list_for_each_owner (layer, &workstation->show_list_head, list)
    {
        /* 全部图层都要进行计算 */
        if (layer->z >= z0 && layer->z <= z1)
        {
            view_left = left - layer->x;
            view_top = top - layer->y;
            view_right = right - layer->x;
            view_buttom = buttom - layer->y;
            if (view_left < 0)
                view_left = 0;
            if (view_top < 0)
                view_top = 0;
            if (view_right > layer->width) 
                view_right = layer->width;
            if (view_buttom > layer->height)
                view_buttom = layer->height;
            
            workstation->flush_bits(layer, view_left, view_top, view_right, view_buttom);
        }
    }

    dwin_leave_critical(crit);
}

void dwin_workstation_init_flush(dwin_workstation_t *workstation)
{
    workstation->id_map = dwin_malloc(workstation->width * workstation->height * sizeof(dwin_layer_id_map_t));
    dwin_assert(workstation->id_map != NULL);
    memset(workstation->id_map, 0, workstation->width * workstation->height * sizeof(dwin_layer_id_map_t));

    workstation->flush_map = flush_map;
    workstation->flush_z = flush_by_z;

    switch (dwin_hal->lcd->parent.bpp) {
    case 8:
        workstation->flush_bits = flush_bits8;
        break;
    case 15:
        workstation->flush_bits = flush_bits15;
        break;
    case 16:
        workstation->flush_bits = flush_bits16;
        break;
    case 24:
        workstation->flush_bits = flush_bits24;
        break;
    case 32:
        workstation->flush_bits = flush_bits32;
        break;
    default:
        workstation->flush_bits = NULL;
        break;
    }
}

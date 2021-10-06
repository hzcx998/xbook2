#include <dwin/objects.h>
#include <dwin/dwin.h>
#include <dwin/layer.h>
#include <dwin/workstation.h>
#include <dwin/hal.h>

void dwin_layer_flush(dwin_layer_t *layer, int left, int top, int right, int buttom)
{
    dwin_workstation_t *station = (dwin_workstation_t *)layer->workstation;
    if (layer->z >= 0)
    {
        station->flush_map(station, layer->x + left, layer->y + top, layer->x + right,
                            layer->y + buttom, layer->z, layer->priority);
        station->flush_z(station, layer->x + left, layer->y + top, layer->x + right,
                            layer->y + buttom, layer->z, layer->z, layer->priority);
    }
}

static void adjust_by_z(dwin_workstation_t *station, dwin_layer_t *layer, int z)
{
    dwin_layer_t *tmp;
    dwin_layer_t *old_layer = NULL;
    int old_z = layer->z;
    int lv = layer->priority;

    if (z > station->priority_topz[lv])
    {
        z = station->priority_topz[lv];
    }
    
    dwin_critical_t crit;
    dwin_enter_critical(crit);

    /* 先从链表中移除 */
    list_del_init(&layer->list);
    if (z == station->priority_topz[lv])
    {
        /* 其它图层降低高度 */
        list_for_each_owner (tmp, &station->priority_list_head[lv], list)
        {
            if (tmp->z > layer->z)
            {
                tmp->z--;
            }
        }
        layer->z = z;
        list_add_tail(&layer->list, &station->priority_list_head[lv]);

        dwin_leave_critical(crit);
        
        /* 刷新新图层[z, z] */
        station->flush_map(station, layer->x, layer->y, layer->x + layer->width, layer->y + layer->height, z, lv);
        station->flush_z(station, layer->x, layer->y, layer->x + layer->width, layer->y + layer->height, z, z, lv);
    } else {    /* 不是最高图层，那么就和其它图层交换 */
        
        if (z > layer->z) { /* 如果新高度比原来的高度高 */
            /* 把位于旧图层高度和新图层高度之间（不包括旧图层，但包括新图层高度）的图层下降1层 */
            list_for_each_owner (tmp, &station->priority_list_head[lv], list) {
                if (tmp->z > layer->z && tmp->z <= z) {
                    if (tmp->z == z) {
                        old_layer = tmp;
                    }
                    tmp->z--;
                }
            }
            dwin_assert(old_layer);
            layer->z = z;
            list_add_after(&layer->list, &old_layer->list);
            
            dwin_leave_critical(crit);

            /* 刷新新图层[z, z] */
            station->flush_map(station, layer->x, layer->y, layer->x + layer->width, layer->y + layer->height, z, lv);
            station->flush_z(station, layer->x, layer->y, layer->x + layer->width, layer->y + layer->height, z, z, lv);
        } else if (z < layer->z) { /* 如果新高度比原来的高度低 */
            /* 把位于旧图层高度和新图层高度之间（不包括旧图层，但包括新图层高度）的图层上升1层 */
            list_for_each_owner (tmp, &station->priority_list_head[lv], list) {
                if (tmp->z < layer->z && tmp->z >= z) {
                    if (tmp->z == z) {  /* 记录原来为与新图层这个位置的图层 */
                        old_layer = tmp;
                    }
                    tmp->z++; /* 等上一步判断图层高度后，再增加图层高度 */
                }
            }
            dwin_assert(old_layer);  
            layer->z = z;
            list_add_before(&layer->list, &old_layer->list);
            
            dwin_leave_critical(crit);
        
            /* 刷新新图层[z + 1, old z] */
            station->flush_map(station, layer->x, layer->y, layer->x + layer->width, layer->y + layer->height, z + 1, lv);
            station->flush_z(station, layer->x, layer->y, layer->x + layer->width, layer->y + layer->height, z + 1, old_z, lv);
        }
    }
}

static void hiden_by_z(dwin_workstation_t *station, dwin_layer_t *layer, int z)
{
    int old_z = layer->z;
    int lv = layer->priority;

    dwin_critical_t crit;
    dwin_enter_critical(crit);

    list_del_init(&layer->list);
    if (station->priority_topz[lv] > old_z) {  /* 旧图层必须在顶图层下面 */
        /* 把位于当前图层后面的图层的高度都向下降1 */
        dwin_layer_t *tmp;
        list_for_each_owner (tmp, &station->priority_list_head[lv], list) {
            if (tmp->z > layer->z)
            {
                tmp->z--;
            }
        }   
    }
    /* 由于隐藏了一个图层，那么，图层顶层的高度就需要减1 */
    station->priority_topz[lv]--;
    layer->z = -1;  /* 隐藏图层后，高度变为-1 */

    dwin_leave_critical(crit);
        
    /* 刷新图层, [0, layer->z - 1] */
    station->flush_map(station, layer->x, layer->y, layer->x + layer->width, layer->y + layer->height, 0, lv);
    station->flush_z(station, layer->x, layer->y, layer->x + layer->width, layer->y + layer->height, 0, old_z - 1, lv);
}

static void show_by_z(dwin_workstation_t *station, dwin_layer_t *layer, int z)
{
    dwin_layer_t *tmp;
    dwin_layer_t *old_layer = NULL;
    int lv = layer->priority;

    dwin_critical_t crit;
    dwin_enter_critical(crit);

    if (z > station->priority_topz[lv])
    {
        station->priority_topz[lv]++;
        z = station->priority_topz[lv];
    }
    else
    {
        station->priority_topz[lv]++;
    }
    
    /* 如果新高度就是最高的图层，就直接插入到图层队列末尾 */
    if (z == station->priority_topz[lv])
    {
        layer->z = z;
        list_add_tail(&layer->list, &station->priority_list_head[lv]);
        
        dwin_leave_critical(crit);
    
        /* 刷新新图层[z, z] */
        station->flush_map(station, layer->x, layer->y, layer->x + layer->width, layer->y + layer->height, z, lv);
        station->flush_z(station, layer->x, layer->y, layer->x + layer->width, layer->y + layer->height, z, z, lv);
    }
    else
    {
        /* 查找和当前图层一样高度的图层 */
        list_for_each_owner(tmp, &station->priority_list_head[lv], list) {
            if (tmp->z == z) {
                old_layer = tmp;
                break;
            }
        }
        tmp = NULL;
        dwin_assert(old_layer);
        /* 把后面的图层的高度都+1 */
        list_for_each_owner(tmp, &station->priority_list_head[lv], list) {
            if (tmp->z >= old_layer->z) { 
                tmp->z++;
            }
        }
        layer->z = z;
        /* 插入到旧图层前面 */
        list_add_before(&layer->list, &old_layer->list);

        dwin_leave_critical(crit);

        /* 刷新新图层[z, z] */
        station->flush_map(station, layer->x, layer->y, layer->x + layer->width, layer->y + layer->height, z, lv);
        station->flush_z(station, layer->x, layer->y, layer->x + layer->width, layer->y + layer->height, z, z, lv);
    }
}

/**
 * dwin_layer_zorder - 设置图层的Z轴深度
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
void dwin_layer_zorder(dwin_layer_t *layer, int z)
{
    if (layer == NULL)
        return;

    if (layer->z == z)
        return;

    if (layer->workstation == NULL)
        return;

    dwin_workstation_t *station = layer->workstation;

    if (dwin_workstation_is_layer_shown(station, layer))
    {
        if (z >= 0)
        {
            adjust_by_z(station, layer, z);
        }
        else
        { 
            /* 小于0就是要隐藏起来的图层 */
            hiden_by_z(station, layer, z);
        }
    }
    else
    {    /* 插入新图层 */
        if (z >= 0)
        {
            show_by_z(station, layer, z);
        }
    }
}

int dwin_layer_move(dwin_layer_t *layer, int x, int y)
{
    if (layer == NULL)
    {
        return -1;
    }

    if (layer->workstation == NULL)
    {
        return -1;
    }
    dwin_workstation_t *station = layer->workstation;

    int old_x = layer->x;
    int old_y = layer->y;
    layer->x = x;
    layer->y = y;
    if (layer->z >= 0)
    {
        int x0, y0, x1, y1;
        x0 = dwin_min(old_x, x);
        y0 = dwin_min(old_y, y);
        x1 = dwin_max(old_x + layer->width, x + layer->width);
        y1 = dwin_max(old_y + layer->height, y + layer->height);
        station->flush_map(station, x0, y0, x1, y1, 0, layer->priority);
        station->flush_z(station, x0, y0, x1, y1, 0, layer->z, layer->priority);
    }

    return 0;
}

void dwin_layer_max_size_repair(uint32_t *width, uint32_t *height)
{
    if (*width > DWIN_LAYER_WIDTH_MAX) {
        dwin_log("layer: width repair from %d to %d.\n", *width, DWIN_LAYER_WIDTH_MAX);
        *width = DWIN_LAYER_WIDTH_MAX;
    }
    if (*height > DWIN_LAYER_HEIGHT_MAX) {
        dwin_log("layer: height repair from %d to %d.\n", *height, DWIN_LAYER_HEIGHT_MAX);
        *height = DWIN_LAYER_HEIGHT_MAX;
    }
}

int dwin_layer_resize(dwin_layer_t *layer, int x, int y, uint32_t width, uint32_t height)
{
    if (layer == NULL || !width || !height)
    {
        return -1;
    }
    
    if (layer->workstation == NULL)
    {
        return -1;
    }
    
    dwin_layer_max_size_repair((uint32_t *) &width, (uint32_t *) &height);
    
    uint8_t *new_buffer = dwin_malloc(DWIN_LAYER_BUF_SZ(width, height));
    if (new_buffer == NULL)
    {
        dwin_log("layer %d resize create new buffer failed!\n", layer->id);
        return -1;
    }
    
    dwin_critical_t crit;
    dwin_enter_critical(crit);

    /* 设置为透明窗口，并刷新 */
    memset(layer->buffer, 0, DWIN_LAYER_BUF_SZ(layer->width, layer->height));
    dwin_layer_move(layer, x, y);

    /* 销毁旧的缓冲区 */
    dwin_free(layer->buffer);

    /* 重新绑定缓冲区 */
    layer->buffer = new_buffer;
    layer->width = width;
    layer->height = height;
    
    dwin_leave_critical(crit);

    return 0;
}

int dwin_layer_change_priority(dwin_layer_t *layer, dwin_layer_priority_t priority)
{
    if (layer == NULL || priority >= DWIN_LAYER_PRIO_NR)
    {
        return -1;
    }
    
    int old_z = layer->z;
    /* on the wrokstation, hide layer first */
    if (layer->workstation)
    {
        dwin_layer_zorder(layer, -1);
    }

    layer->priority = priority;
    
    /* restore z order */
    if (layer->workstation)
    {
        dwin_layer_zorder(layer, old_z);
    }
    return 0;
}

int dwin_layer_draw_rect(dwin_layer_t *layer, int x, int y, uint32_t w, uint32_t h, uint32_t color)
{
    int i, j;
    uint32_t *p = (uint32_t *)layer->buffer;
    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            p[(y + j) * layer->width + (x + i)] = color;
        }
    }
    return 0;
}

void dwin_layer_bitblt(dwin_layer_t *layer, int x, int y, 
        struct dwin_buffer *buf, dwin_rect_t *rect)
{
    if (layer == NULL || !buf)
        return;
    if (layer->buffer == NULL)
        return;

    int bx;
    int by;
    int width;
    int height;

    if (rect == NULL)
    {
        bx = 0;
        by = 0;
        width = buf->width;
        height = buf->width;
    }
    else
    {
        bx = rect->x;
        by = rect->y;
        width = rect->w;
        height = rect->h;
    }

    if (width > buf->width)
        width = buf->width;
    if (height > buf->height)
        height = buf->height;
    /* 宽度剪裁 */
    int w = dwin_min(width, buf->width - bx);
    int h = dwin_min(height, buf->height - by);
    if (w <= 0 || h <= 0)
        return;
    
    uint32_t color;
    int vy, by2;
    int vx, bx2;
    /* 右下位置剪裁 */
    int vy2 = min((y + h), layer->height);
    int vx2 = min((x + w), layer->width);
    for (vy = y, by2 = by; vy < vy2; vy++, by2++)
    {
        for (vx = x, bx2 = bx; vx < vx2; vx++, bx2++)
        {
            color = dwin_buffer_getpixel(buf, bx2, by2);
            if (((color >> 24) & 0xff)) {
                dwin_layer_putpixel(layer, vx, vy, color);
            }
        }
    }
}

int dwin_layer_add_flags(dwin_layer_t *layer, uint32_t flags)
{
    if (layer == NULL)
    {
        return -1;
    }
    layer->flags |= flags;
    return 0;
}

int dwin_layer_del_flags(dwin_layer_t *layer, uint32_t flags)
{
    if (layer == NULL)
    {
        return -1;
    }
    layer->flags &= ~flags;
    return 0;
}

int dwin_layer_recv_message(dwin_layer_t *layer, void *msg, int flags)
{
    if (layer == NULL)
    {
        return -1;
    }
    if (layer->msgpool == NULL)
    {
        return -1;
    }
    return dwin_hal->msgpool->recv(layer->msgpool, msg, flags);
}

int dwin_layer_send_message(dwin_layer_t *layer, void *msg, int flags)
{
    if (layer == NULL)
    {
        return -1;
    }
    if (layer->msgpool == NULL)
    {
        return -1;
    }
    return dwin_hal->msgpool->send(layer->msgpool, msg, flags);
}

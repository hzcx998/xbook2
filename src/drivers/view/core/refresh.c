#include <drivers/view/view.h>
#include <drivers/view/screen.h>
#include <xbook/memalloc.h>
#include <string.h>

extern list_t view_show_list_head;
extern list_t view_global_list_head;
extern uint16_t *view_id_map;

typedef void (*view_refresh_block_t) (view_t *, int , int , int, int);
static view_refresh_block_t view_refresh_block = NULL;

#ifdef CONFIG_VIEW_ALPAH
uint32_t *screen_backup_buffer; /*  */
#endif

void view_refresh_map(int left, int top, int right, int buttom, int z0)
{
    #ifdef CONFIG_VIEW_ALPAH  /* 透明图层不需要map */
    return;
    #endif         
    
    int view_left, view_top, view_right, view_buttom;
    int screen_x, screen_y;
    int view_x, view_y;

    if (left < 0)
        left = 0;
	if (top < 0)
        top = 0;
	if (right > view_screen.width)
        right = view_screen.width;
	if (buttom > view_screen.height)
        buttom = view_screen.height;
    
    view_t *view;
    view_color_t *colors;

    /* 刷新高度为[z0-top]区间的视图 */
    list_for_each_owner (view, &view_show_list_head, list) {
        if (view->z >= z0) {
            view_left = left - view->x;
            view_top = top - view->y;
            view_right = right - view->x;
            view_buttom = buttom - view->y;
            if (view_left < 0)
                view_left = 0;
            if (view_top < 0)
                view_top = 0;
            if (view_right > view->width) 
                view_right = view->width;
            if (view_buttom > view->height)
                view_buttom = view->height;
            colors = (view_color_t *)view->section->addr;
            /* 进入循环前进行位置预判，然后调整位置 */
            // view_top
            screen_y = view->y + view_top;
            if (screen_y < 0) {
                view_top += -screen_y;
            }
            if (screen_y >= view_screen.height)
                continue;
            // view_buttom
            screen_y = view->y + view_buttom;
            if (screen_y >= view_screen.height) {
                view_buttom -= screen_y - view_screen.height;
            }

            // view_left
            screen_x = view->x + view_left;
            if (screen_x < 0) {
                view_left += -screen_x;
            }
            if (screen_x >= view_screen.width)
                continue;
            // view_right
            screen_x = view->x + view_right;
            if (screen_x < 0) {
                view_right -= screen_x - view_screen.width;
            }
            
            for(view_y = view_top; view_y < view_buttom; view_y++){
                screen_y = view->y + view_y;
                for(view_x = view_left; view_x < view_right; view_x++){
                    /* 不是全透明的，就把视图标识写入到映射表中 */
                    if ((colors[view_y * view->width + view_x] >> 24) & 0xff) {
                        view_id_map[(screen_y * view_screen.width + (view->x + view_x))] = view->z;
                    }
                }
            }
        }
    }
}

static inline void view_refresh_block32(view_t *view, int view_left, int view_top, int view_right, int view_buttom)
{
    /* 在屏幕上的位置 */
    int screen_y;
    /* 在图层上的位置 */
    int view_x, view_y;
    #ifdef CONFIG_VIEW_ALPAH
    view_argb_t *src_rgb, *dst_rgb;
    #endif
    /* 优化整个块 */
    for (view_y = view_top; view_y < view_buttom; view_y++)
    {
        screen_y = view->y + view_y;
        #ifndef CONFIG_VIEW_ALPAH
        uint32_t *dst = &((uint32_t *)view_screen.vram_start)[view_screen.width * screen_y + view->x];
        uint32_t *src = &((uint32_t *) view->section->addr)[view_y * view->width]; 
        uint16_t *map = &view_id_map[(screen_y * view_screen.width + view->x)];
        #else
        dst_rgb = (view_argb_t *) (screen_backup_buffer + (screen_y * view_screen.width + view->x));
        src_rgb = (view_argb_t *) view->section->addr;
        src_rgb += view_y * view->width;
        #endif
        
        for (view_x = view_left; view_x < view_right; view_x++)
        {
            #ifndef CONFIG_VIEW_ALPAH
            /* 照着map中的z进行刷新 */
            if (map[view_x] == view->z)
            {
            #endif
                /* 获取图层中的颜色 */
                #ifndef CONFIG_VIEW_ALPAH
                dst[view_x] = src[view_x];
                #else   /* 根据透明度计算rgb值，算法：AlphaBlend */
                dst_rgb[view_x].red = (((src_rgb[view_x].red) * src_rgb[view_x].alpha + 
                    (dst_rgb[view_x].red) *(0xff - src_rgb[view_x].alpha)) >> 8)&0xffU;
                dst_rgb[view_x].green = (((src_rgb[view_x].green) * src_rgb[view_x].alpha + 
                    (dst_rgb[view_x].green) *(0xff - src_rgb[view_x].alpha)) >> 8)&0xffU;
                dst_rgb[view_x].blue = (((src_rgb[view_x].blue) * src_rgb[view_x].alpha + 
                    (dst_rgb[view_x].blue) *(0xff - src_rgb[view_x].alpha)) >> 8)&0xffU;
                dst_rgb[view_x].alpha = (((src_rgb[view_x].alpha) * src_rgb[view_x].alpha + 
                    (dst_rgb[view_x].alpha) *(0xff - src_rgb[view_x].alpha)) >> 8)&0xffU;
                
                #endif /* CONFIG_VIEW_ALPAH */

            #ifndef CONFIG_VIEW_ALPAH
            }
            #endif /* CONFIG_VIEW_ALPAH */
        }
    }
}

static inline void view_refresh_block24(view_t *view, int view_left, int view_top, int view_right, int view_buttom)
{
    /* 在屏幕上的位置 */
    int screen_y;
    /* 在图层上的位置 */
    int view_x, view_y;
    #ifdef CONFIG_VIEW_ALPAH
    view_argb_t *src_rgb, *dst_rgb;
    #endif
    /* 优化整个块 */
    for (view_y = view_top; view_y < view_buttom; view_y++)
    {
        screen_y = view->y + view_y;
        #ifndef CONFIG_VIEW_ALPAH
        uint8_t *dst = &((uint8_t *)view_screen.vram_start)[(view_screen.width * screen_y + view->x) * 3];
        uint32_t *src = &((uint32_t *) view->section->addr)[view_y * view->width]; 
        uint16_t *map = &view_id_map[(screen_y * view_screen.width + view->x)];
        #else
        dst_rgb = (view_argb_t *) (screen_backup_buffer + (screen_y * view_screen.width + view->x));
        src_rgb = (view_argb_t *) view->section->addr;
        src_rgb += view_y * view->width;
        #endif
        for (view_x = view_left; view_x < view_right; view_x++)
        {
            #ifndef CONFIG_VIEW_ALPAH
            /* 照着map中的z进行刷新 */
            if (map[view_x] == view->z)
            {
            #endif
                /* 获取图层中的颜色 */
                #ifndef CONFIG_VIEW_ALPAH
                dst[view_x * 3 + 0] = src[view_x] & 0xFF;
                dst[view_x * 3 + 1] = (src[view_x] & 0xFF00) >> 8;
                dst[view_x * 3 + 2] = (src[view_x] & 0xFF0000) >> 16;
                #else   /* 根据透明度计算rgb值，算法：AlphaBlend */
                dst_rgb[view_x].red = (((src_rgb[view_x].red) * src_rgb[view_x].alpha + 
                    (dst_rgb[view_x].red) *(0xff - src_rgb[view_x].alpha)) >> 8)&0xffU;
                dst_rgb[view_x].green = (((src_rgb[view_x].green) * src_rgb[view_x].alpha + 
                    (dst_rgb[view_x].green) *(0xff - src_rgb[view_x].alpha)) >> 8)&0xffU;
                dst_rgb[view_x].blue = (((src_rgb[view_x].blue) * src_rgb[view_x].alpha + 
                    (dst_rgb[view_x].blue) *(0xff - src_rgb[view_x].alpha)) >> 8)&0xffU;
                dst_rgb[view_x].alpha = (((src_rgb[view_x].alpha) * src_rgb[view_x].alpha + 
                    (dst_rgb[view_x].alpha) *(0xff - src_rgb[view_x].alpha)) >> 8)&0xffU;
                #endif
            #ifndef CONFIG_VIEW_ALPAH
            }
            #endif
        }
    }
}

static inline void view_refresh_block16(view_t *view, int view_left, int view_top, int view_right, int view_buttom)
{
    // keprint("%d %d %d %d\n", view_left, view_top, view_right, view_buttom);
    /* 在屏幕上的位置 */
    int screen_y;
    /* 在图层上的位置 */
    int view_x, view_y;
    #ifdef CONFIG_VIEW_ALPAH
    view_argb_t *src_rgb, *dst_rgb;
    #endif
    /* 优化整个块 */
    for (view_y = view_top; view_y < view_buttom; view_y++)
    {
        screen_y = view->y + view_y;
        #ifndef CONFIG_VIEW_ALPAH
        uint16_t *dst = &((uint16_t *)view_screen.vram_start)[(view_screen.width * screen_y + view->x)];
        uint32_t *src = &((uint32_t *) view->section->addr)[view_y * view->width]; 
        uint16_t *map = &view_id_map[(screen_y * view_screen.width + view->x)];
        #else
        dst_rgb = (view_argb_t *) (screen_backup_buffer + (screen_y * view_screen.width + view->x));
        src_rgb = (view_argb_t *) view->section->addr;
        src_rgb += view_y * view->width;
        #endif
        for (view_x = view_left; view_x < view_right; view_x++)
        {
            #ifndef CONFIG_VIEW_ALPAH
            /* 照着map中的z进行刷新 */
            if (map[view_x] == view->z) {
            #endif
                /* 获取图层中的颜色 */
                #ifndef CONFIG_VIEW_ALPAH
                dst[view_x] = (uint16_t)((src[view_x] &0xF8) >> 3) | ((src[view_x] &0xFC00) >> 5) | ((src[view_x] &0xF80000) >> 8);
                
                #else   /* 根据透明度计算rgb值，算法：AlphaBlend */
                dst_rgb[view_x].red = (((src_rgb[view_x].red) * src_rgb[view_x].alpha + 
                    (dst_rgb[view_x].red) *(0xff - src_rgb[view_x].alpha)) >> 8)&0xffU;
                dst_rgb[view_x].green = (((src_rgb[view_x].green) * src_rgb[view_x].alpha + 
                    (dst_rgb[view_x].green) *(0xff - src_rgb[view_x].alpha)) >> 8)&0xffU;
                dst_rgb[view_x].blue = (((src_rgb[view_x].blue) * src_rgb[view_x].alpha + 
                    (dst_rgb[view_x].blue) *(0xff - src_rgb[view_x].alpha)) >> 8)&0xffU;
                dst_rgb[view_x].alpha = (((src_rgb[view_x].alpha) * src_rgb[view_x].alpha + 
                    (dst_rgb[view_x].alpha) *(0xff - src_rgb[view_x].alpha)) >> 8)&0xffU;
                #endif
            #ifndef CONFIG_VIEW_ALPAH
            }
            #endif
        }
    }
}

static inline void view_refresh_block15(view_t *view, int view_left, int view_top, int view_right, int view_buttom)
{
    /* 在屏幕上的位置 */
    int screen_y;
    /* 在图层上的位置 */
    int view_x, view_y;
    #ifdef CONFIG_VIEW_ALPAH
    view_argb_t *src_rgb, *dst_rgb;
    #endif
    /* 优化整个块 */
    for (view_y = view_top; view_y < view_buttom; view_y++)
    {
        screen_y = view->y + view_y;
        #ifndef CONFIG_VIEW_ALPAH
        uint16_t *dst = &((uint16_t *)view_screen.vram_start)[(view_screen.width * screen_y + view->x)];
        uint32_t *src = &((uint32_t *) view->section->addr)[view_y * view->width]; 
        uint16_t *map = &view_id_map[(screen_y * view_screen.width + view->x)];
        #else
        dst_rgb = (view_argb_t *) (screen_backup_buffer + (screen_y * view_screen.width + view->x));
        src_rgb = (view_argb_t *) view->section->addr;
        src_rgb += view_y * view->width;
        #endif
        for (view_x = view_left; view_x < view_right; view_x++)
        {
            #ifndef CONFIG_VIEW_ALPAH
            /* 照着map中的z进行刷新 */
            if (map[view_x] == view->z)
            {
            #endif
                /* 获取图层中的颜色 */
                #ifndef CONFIG_VIEW_ALPAH
                dst[view_x] = (uint16_t)((src[view_x] &0xF8) >> 3) | ((src[view_x] &0xF800) >> 6) | ((src[view_x] &0xF80000) >> 9);
                #else   /* 根据透明度计算rgb值，算法：AlphaBlend */
                dst_rgb[view_x].red = (((src_rgb[view_x].red) * src_rgb[view_x].alpha + 
                    (dst_rgb[view_x].red) *(0xff - src_rgb[view_x].alpha)) >> 8)&0xffU;
                dst_rgb[view_x].green = (((src_rgb[view_x].green) * src_rgb[view_x].alpha + 
                    (dst_rgb[view_x].green) *(0xff - src_rgb[view_x].alpha)) >> 8)&0xffU;
                dst_rgb[view_x].blue = (((src_rgb[view_x].blue) * src_rgb[view_x].alpha + 
                    (dst_rgb[view_x].blue) *(0xff - src_rgb[view_x].alpha)) >> 8)&0xffU;
                dst_rgb[view_x].alpha = (((src_rgb[view_x].alpha) * src_rgb[view_x].alpha + 
                    (dst_rgb[view_x].alpha) *(0xff - src_rgb[view_x].alpha)) >> 8)&0xffU;
                #endif
            #ifndef CONFIG_VIEW_ALPAH
            }
            #endif
        }
    }
}

static inline void view_refresh_block8(view_t *view, int view_left, int view_top, int view_right, int view_buttom)
{
    /* 在屏幕上的位置 */
    int screen_y;
    /* 在图层上的位置 */
    int view_x, view_y;
    #ifdef CONFIG_VIEW_ALPAH
    view_argb_t *src_rgb, *dst_rgb;
    #endif
    /* 优化整个块 */
    for (view_y = view_top; view_y < view_buttom; view_y++)
    {
        screen_y = view->y + view_y;
        #ifndef CONFIG_VIEW_ALPAH
        uint8_t *dst = &((uint8_t *)view_screen.vram_start)[(view_screen.width * screen_y + view->x)];
        uint32_t *src = &((uint32_t *) view->section->addr)[view_y * view->width]; 
        uint16_t *map = &view_id_map[(screen_y * view_screen.width + view->x)];
        #else
        dst_rgb = (view_argb_t *) (screen_backup_buffer + (screen_y * view_screen.width + view->x));
        src_rgb = (view_argb_t *) view->section->addr;
        src_rgb += view_y * view->width;
        #endif
        for (view_x = view_left; view_x < view_right; view_x++)
        {
            #ifndef CONFIG_VIEW_ALPAH
            /* 照着map中的z进行刷新 */
            if (map[view_x] == view->z)
            {
            #endif
                /* 获取图层中的颜色 */
                #ifndef CONFIG_VIEW_ALPAH
                dst[view_x] = (uint8_t)((src[view_x] &0xC0) >> 6) | ((src[view_x] &0xE000) >> 11) | ((src[view_x] &0xE00000) >> 16);
                #else   /* 根据透明度计算rgb值，算法：AlphaBlend */
                dst_rgb[view_x].red = (((src_rgb[view_x].red) * src_rgb[view_x].alpha + 
                    (dst_rgb[view_x].red) *(0xff - src_rgb[view_x].alpha)) >> 8)&0xffU;
                dst_rgb[view_x].green = (((src_rgb[view_x].green) * src_rgb[view_x].alpha + 
                    (dst_rgb[view_x].green) *(0xff - src_rgb[view_x].alpha)) >> 8)&0xffU;
                dst_rgb[view_x].blue = (((src_rgb[view_x].blue) * src_rgb[view_x].alpha + 
                    (dst_rgb[view_x].blue) *(0xff - src_rgb[view_x].alpha)) >> 8)&0xffU;
                dst_rgb[view_x].alpha = (((src_rgb[view_x].alpha) * src_rgb[view_x].alpha + 
                    (dst_rgb[view_x].alpha) *(0xff - src_rgb[view_x].alpha)) >> 8)&0xffU;
                #endif
            #ifndef CONFIG_VIEW_ALPAH
            }
            #endif
        }
    }
}

#ifdef CONFIG_VIEW_ALPAH
static void __layer_refresh_copy(int left, int top, int right, int buttom)
{
    int screen_x, screen_y;

    switch (view_screen.bpp) {
    case 8:
        for (screen_y = top; screen_y < buttom; screen_y++) {
            uint8_t *dst = &((uint8_t *)view_screen.vram_start)[(view_screen.width * screen_y)];
            uint32_t *src = &screen_backup_buffer[view_screen.width * screen_y];
            for (screen_x = left; screen_x < right; screen_x++) {
                dst[screen_x] = (uint8_t)((src[screen_x] &0xC0) >> 6) | ((src[screen_x] &0xE000) >> 11) | ((src[screen_x] &0xE00000) >> 16);
            }
        }
        break;
    case 15:
        for (screen_y = top; screen_y < buttom; screen_y++) {
            uint16_t *dst = &((uint16_t *)view_screen.vram_start)[(view_screen.width * screen_y)];
            uint32_t *src = &screen_backup_buffer[view_screen.width * screen_y];
            for (screen_x = left; screen_x < right; screen_x++) {
                dst[screen_x] = (uint16_t)((src[screen_x] &0xF8) >> 3) | ((src[screen_x] &0xF800) >> 6) | ((src[screen_x] &0xF80000) >> 9);
            }
        }
        break;
    case 16:
        for (screen_y = top; screen_y < buttom; screen_y++) {
            uint16_t *dst = &((uint16_t *)view_screen.vram_start)[(view_screen.width * screen_y)];
            uint32_t *src = &screen_backup_buffer[view_screen.width * screen_y];
            for (screen_x = left; screen_x < right; screen_x++) {
                dst[screen_x] = (uint16_t)((src[screen_x] &0xF8) >> 3) | ((src[screen_x] &0xFC00) >> 5) | ((src[screen_x] &0xF80000) >> 8);
            }
        }
        break;
    case 24:
        for (screen_y = top; screen_y < buttom; screen_y++) {
            uint8_t *dst = &((uint8_t *)view_screen.vram_start)[(view_screen.width * screen_y) * 3];
            uint32_t *src = &screen_backup_buffer[view_screen.width * screen_y];
            for (screen_x = left; screen_x < right; screen_x++) {
                dst[screen_x * 3 + 0] = src[screen_x] & 0xFF;
                dst[screen_x * 3 + 1] = (src[screen_x] & 0xFF00) >> 8;
                dst[screen_x * 3 + 2] = (src[screen_x] & 0xFF0000) >> 16;
            }
        }
        break;  
    case 32:
        for (screen_y = top; screen_y < buttom; screen_y++) {
            uint32_t *dst = &((uint32_t *)view_screen.vram_start)[view_screen.width * screen_y];
            uint32_t *src = &screen_backup_buffer[view_screen.width * screen_y];
            for (screen_x = left; screen_x < right; screen_x++) {
                dst[screen_x] = src[screen_x];
            }
        }
        break;
    default:
        break;
    }
}
#endif

void view_refresh_by_z(int left, int top, int right, int buttom, int z0, int z1)
{
    int view_left, view_top, view_right, view_buttom;

    if (left < 0)
        left = 0;
	if (top < 0)
        top = 0;
	if (right > view_screen.width)
        right = view_screen.width;
	if (buttom > view_screen.height)
        buttom = view_screen.height;
    view_t *view;
    list_for_each_owner (view, &view_show_list_head, list) {
        #ifndef CONFIG_VIEW_ALPAH /* 全部图层都要进行计算 */
        if (view->z >= z0 && view->z <= z1) {
        #endif
            view_left = left - view->x;
            view_top = top - view->y;
            view_right = right - view->x;
            view_buttom = buttom - view->y;
            if (view_left < 0)
                view_left = 0;
            if (view_top < 0)
                view_top = 0;
            if (view_right > view->width) 
                view_right = view->width;
            if (view_buttom > view->height)
                view_buttom = view->height;
            
            view_refresh_block(view, view_left, view_top, view_right, view_buttom);
        #ifndef CONFIG_VIEW_ALPAH
        }
        #endif
    }
    
    #ifdef CONFIG_VIEW_ALPAH  /* 将指定区域刷新到屏幕 */
    __layer_refresh_copy(left, top, right, buttom);
    #endif
}

void view_refresh(view_t *view, int left, int top, int right, int buttom)
{
    if (view->z >= 0) {
        view_refresh_map(view->x + left, view->y + top, view->x + right,
            view->y + buttom, view->z);
        view_refresh_by_z(view->x + left, view->y + top, view->x + right,
            view->y + buttom, view->z, view->z);
    }
}

void view_refresh_rect(view_t *view, int x, int y, uint32_t width, uint32_t height)
{
    view_refresh(view, x, y, x + width, y + height);
}

/**
 * 刷新图层以及其下面的所有图层
 */
void view_refresh_from_bottom(view_t *view, int left, int top, int right, int buttom)
{
    if (view->z >= 0) {
        view_refresh_map(view->x + left, view->y + top, view->x + right,
            view->y + buttom, 0);
        view_refresh_by_z(view->x + left, view->y + top, view->x + right,
            view->y + buttom, 0, view->z);
    }
}

void view_refresh_rect_from_bottom(view_t *view, int x, int y, uint32_t width, uint32_t height)
{
    view_refresh_from_bottom(view, x, y, x + width, y + height);
}

int view_init_refresh()
{
    #ifdef CONFIG_VIEW_ALPAH /* 分配屏幕缓冲区 */
    int memsize = view_screen.width * view_screen.height * sizeof(uint32_t);
    screen_backup_buffer = mem_alloc(memsize);
    if (screen_backup_buffer == NULL) {
        return -1;
    }
    memset(screen_backup_buffer, 0, memsize);
    #endif

    switch (view_screen.bpp) {
    case 8:
        view_refresh_block = view_refresh_block8;
        break;
    case 15:
        view_refresh_block = view_refresh_block15;
        break;
    case 16:
        view_refresh_block = view_refresh_block16;
        break;
    case 24:
        view_refresh_block = view_refresh_block24;
        break;
    case 32:
        view_refresh_block = view_refresh_block32;
        break;
    default:
        #ifdef CONFIG_VIEW_ALPAH /* 分配屏幕缓冲区 */
        mem_free(screen_backup_buffer);
        screen_backup_buffer = NULL;
        #endif
        return -1;
    }
    return 0;
}

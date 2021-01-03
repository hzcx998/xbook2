#include <drivers/view/env.h>
#include <drivers/view/msg.h>
#include <drivers/view/mouse.h>
#include <drivers/view/screen.h>
#include <drivers/view/render.h>

/*
视图环境被分成3个层级
最底层：用于实现桌面系统
中层：用于实现窗口系统
高层：用于实现任务栏，鼠标，悬浮窗
*/

/* 获取活动中的视图，也就是当前获得按键操作权的视图 */
static view_t *view_activity = NULL;
/* 高层视图的最低视图 */
static view_t *view_high_level_lower = NULL;

/* 鼠标悬停的视图 */
static view_t *mouse_hover_view = NULL;
/* 调整大小时的视图 */
static view_t *resize_view = NULL;
/* 拖拽时的视图 */
static view_t *drag_view = NULL;

#define CONFIG_SHADE_VIEW
#ifdef CONFIG_SHADE_VIEW
/* 拖拽或者调整视图大小时的视图 */
static view_t *shade_view = NULL;
static view_rect_t shade_rect; /* 遮罩图层矩形区域 */
#endif

view_t *view_env_get_activity()
{
    return view_activity;
}

void view_env_set_activity(view_t *view)
{
    view_activity = view;
}

view_t *view_env_get_high_level_lower()
{
    return view_high_level_lower;
}

void view_env_set_high_level_lower(view_t *view)
{
    view_high_level_lower = view;
}

/**
 * 尝试聚焦到某个图层
 * 
 * 如果有之前聚焦的图层，那么就需要对它丢焦
 * 如果新聚焦图层是窗口，那么就需要将它切换到窗口顶部
 * 再发送消息给新聚焦图层
 */
int view_env_try_activate(view_t *view)
{
    view_t *activity = view_activity;
    int val = -1;
    if (activity != view) {
        view_msg_t m;
        view_msg_reset(&m);
        if (activity) {
            view_msg_header(&m, VIEW_MSG_INACTIVATE, activity->id);
            val = view_try_put_msg(activity, &m);
        }
        view_env_set_activity(view);
        if (view->type == VIEW_TYPE_WINDOW) {
            /* 把图层切换到最高等级图层的下面 */
            assert(view_high_level_lower);
            view_set_z(view, view_high_level_lower->z - 1);
        }
        view_msg_header(&m, VIEW_MSG_ACTIVATE, view->id);
        val = view_try_put_msg(view, &m);
    }
    return val;
}

void view_env_do_mouse_hover(view_t *view, view_msg_t *msg, int lcmx, int lcmy)
{
    if (mouse_hover_view && mouse_hover_view != view ) { /* 从其他图层进入当前图层 */
        /* enter view */
        view_msg_t m;
        view_msg_reset(&m);
        view_msg_header(&m, VIEW_MSG_ENTER, view->id);
        view_msg_data(&m, lcmx, lcmy, msg->data0, msg->data1);
        view_try_put_msg(view, &m);
        /* leave hover */
        view_msg_header(&m, VIEW_MSG_LEAVE, mouse_hover_view->id);
        view_msg_data(&m, msg->data0 - mouse_hover_view->x, msg->data1 - mouse_hover_view->y, 0, 0);
        view_try_put_msg(mouse_hover_view, &m);

        /* 如果进入了不同的图层，那么，当为调整图层大小时，就需要取消调整行为 */
        if (view_mouse.state != VIEW_MOUSE_NORMAL) {
            view_mouse_set_state(VIEW_MOUSE_NORMAL);
            view_mouse.click_x = -1;
            view_mouse.click_y = -1;
            #ifdef CONFIG_SHADE_VIEW
            if (shade_view->z > 0) { /* 隐藏遮罩图层 */
                view_set_z(shade_view, -1); 
            }
            #endif /* CONFIG_SHADE_VIEW */
        }
    }
    mouse_hover_view = view;
}

void view_env_do_drag(view_t *view, view_msg_t *msg, int lcmx, int lcmy)
{
    if (view->type != VIEW_TYPE_FIXED) {        
        /* 检测可移动区域 */
        if (!view_drag_rect_check(view, lcmx, lcmy))
            return;
        if (view_msg_get_type(msg) == VIEW_MSG_MOUSE_LBTN_DOWN) {
            view_mouse.local_x = lcmx;
            view_mouse.local_y = lcmy;
            drag_view = view;
            view_mouse_set_state(VIEW_MOUSE_NORMAL);
        }
    }
}

/**
 * 计算重新调整大小后的图层，并把结果返回到一个矩形区域中
 * 
 * 有调整返回0，没有返回-1
 */
int view_calc_resize(view_t *view, int mx, int my, view_rect_t *out_rect)
{
    if (!view)
        return -1;

    int sx, sy; // screen x, y
    sx = mx - view_mouse.click_x;
    sy = my - view_mouse.click_y;
    if (!sx && !sy) { /* 没有移动，没有调整 */
        return -1;
    }
    
    int cx, cy; // center x, y
    cx = view->width / 2 + view->x;
    cy = view->height / 2 + view->y;
    
    /* 由于调整后宽高可能为负，所以要用支持负数的矩形 */
    view_rect_t rect;
    view_rect_reset(&rect);
    
    switch (view_mouse.state)
    {
    case VIEW_MOUSE_VRESIZE:
        /* 上边调整 */
        if (view_mouse.click_y < cy) {
            rect.x = view->x;
            rect.y = view->y + sy;
            rect.w.sw = view->width;
            rect.h.sh = view->height + -sy;
        } else {    /* 下边调整 */
            rect.x = view->x;
            rect.y = view->y;
            rect.w.sw = view->width;
            rect.h.sh = view->height + sy;
        }
        break;
    case VIEW_MOUSE_HRESIZE:
        /* 左边调整 */
        if (view_mouse.click_x < cx) {
            rect.x = view->x + sx;
            rect.y = view->y;
            rect.w.sw = view->width + -sx;
            rect.h.sh = view->height;
        } else {    /* 右边调整 */
            rect.x = view->x;
            rect.y = view->y;
            rect.w.sw = view->width + sx;
            rect.h.sh = view->height;
        }
        break;
    case VIEW_MOUSE_DRESIZE1:
        /* 左边调整 */
        if (view_mouse.click_x < cx) {
            rect.x = view->x + sx;
            rect.y = view->y;
            rect.w.sw = view->width + -sx;
            rect.h.sh = view->height + sy;
        } else {    /* 右边调整 */
            rect.x = view->x;
            rect.y = view->y + sy;
            rect.w.sw = view->width + sx;
            rect.h.sh = view->height + -sy;
        }
        break;
    case VIEW_MOUSE_DRESIZE2:
        /* 左边调整 */
        if (view_mouse.click_x < cx) {
            rect.x = view->x + sx;
            rect.y = view->y + sy;
            rect.w.sw = view->width + -sx;
            rect.h.sh = view->height + -sy;
        } else {    /* 右边调整 */
            rect.x = view->x;
            rect.y = view->y;
            rect.w.sw = view->width + sx;
            rect.h.sh = view->height + sy;
        }
        break;        
    default:
        return -1;
    }
    /* 对调整后的矩形进行判断，看是否符合要求 */
    if (rect.w.sw <= 0 || rect.h.sh <= 0) {
        //errprint("calc size invalid!\n");
        return -1;  /* invalid rect */
    }
    if (view_region_valid(&view->min_resize_region)) {
        if (rect.w.sw < view->min_resize_region.right) {
            rect.w.sw = view->min_resize_region.right;
        }
        if (rect.h.sh < view->min_resize_region.bottom) {
            rect.h.sh = view->min_resize_region.bottom;
        }
    }
    *out_rect = rect; /* 转换矩形 */
    return 0;
}


/**
 * 尝试调整图层大小，并发送RESIZE消息给指定图层
 * 
 */
int view_try_resize(view_t *view, view_rect_t *rect)
{
    if (!view || !rect)
        return -1;
    if (view_resize(view, rect->x, rect->y, rect->w.uw, rect->h.uh) < 0)
        return -1;
    view_msg_t m;
    view_msg_header(&m, VIEW_MSG_RESIZE, view->id);
    view_msg_data(&m, rect->x, rect->y, rect->w.uw, rect->h.uh);
    return view_try_put_msg(view, &m);
}

/**
 * 派发鼠标的过滤消息
 * 如果成功过滤返回0，没有过滤就返回-1，表示消息还需要进一步处理
 */
int view_env_filter_mouse_msg(view_msg_t *msg)
{
    /* 按下鼠标左键，并且在调整图层大小或者拖拽图层，就要截断消息 */
    if (view_msg_get_type(msg) == VIEW_MSG_MOUSE_LBTN_DOWN) {
        if (resize_view) /* 有调整大小图层时不允许产生鼠标左键单击消息 */
            return 0;
        if (drag_view) /* 有拖拽图层时不允许产生鼠标左键单击消息 */
            return 0;
    }
    /* 鼠标左键弹起时进行释放 */
    if (view_msg_get_type(msg) == VIEW_MSG_MOUSE_LBTN_UP) {
        if (resize_view) {
            view_rect_t rect;
            if (!view_calc_resize(resize_view, msg->data0, msg->data1, &rect)) {
                /* 发送一个调整大小消息 */
                #ifdef DEBUG_GUI_LAYER
                keprint("[gui]: up -> view resize from (%d, %d), (%d, %d)",
                    resize_view->x, resize_view->y,
                    resize_view->width, resize_view->height);
                keprint(" to (%d, %d), (%d, %d)\n",
                    rect.x, rect.y, rect.w.sw, rect.h.sh);
                #endif /* DEBUG_GUI_LAYER */
                view_try_resize(resize_view, &rect);
                
                #ifdef CONFIG_SHADE_VIEW
                view_set_z(shade_view, -1); /* 隐藏遮罩图层 */
                #endif /* CONFIG_SHADE_VIEW */
            }
            resize_view = NULL;
            /* 设置鼠标状态 */
            view_mouse_set_state(VIEW_MOUSE_NORMAL);
            view_mouse.click_x = -1;
            view_mouse.click_y = -1;
            return 0;
        }
        if (drag_view) {
            if (view_mouse_is_state(VIEW_MOUSE_HOLD)) {
                #ifdef CONFIG_SHADE_VIEW
                /* 设置抓取窗口的位置 */
                view_set_xy(drag_view, shade_rect.x,
                    shade_rect.y);
                view_set_z(shade_view, -1); /* 隐藏遮罩图层 */
                #else
                int wx = view_mouse.x - view_mouse.local_x;
                int wy = view_mouse.y - view_mouse.local_y; 
                view_set_xy(drag_view, wx, wy);
                #endif /* CONFIG_SHADE_VIEW */
                view_msg_t m;
                view_msg_reset(&m);
                view_msg_header(&m, VIEW_MSG_MOVE, drag_view->id);
                view_msg_data(&m, drag_view->x, drag_view->y, 0, 0);
                view_try_put_msg(drag_view, &m);
            }
            drag_view = NULL;
            view_mouse_set_state(VIEW_MOUSE_NORMAL);
            return 0;
        }
    }
    if (msg->id == VIEW_MSG_MOUSE_MOTION) {
        if (resize_view) {
            #ifdef CONFIG_SHADE_VIEW
            /* 擦除上一次绘制的内容 */
            if (view_rect_valid(&shade_rect)) {
                view_render_draw_shade(shade_view, &shade_rect, 0);
            }
            #endif
            
            /* 实时调整大小 */
            view_rect_t rect;
            if (!view_calc_resize(resize_view, msg->data0, msg->data1, &rect)) {
                #if 0
                keprint("view resize from (%d, %d), (%d, %d)",
                    resize_view->x, resize_view->y,
                    resize_view->width, resize_view->height);
                keprint(" to (%d, %d), (%d, %d)\n",
                    rect.x, rect.y, rect.w.sw, rect.h.sh);     
                #endif /* DEBUG_GUI_LAYER */
                #ifdef CONFIG_SHADE_VIEW
                view_render_draw_shade(shade_view, &rect, 1); /* 绘制新内容 */
                view_rect_copy(&shade_rect, &rect); /* 保存新区域 */
                if (shade_view->z < 0) {
                    view_set_z(shade_view, view_env_get_high_level_lower()->z);
                }
                #endif /* CONFIG_SHADE_VIEW */

                return 0;
            }
        }
        if (drag_view) {
            int wx = view_mouse.x - view_mouse.local_x;
            int wy = view_mouse.y - view_mouse.local_y;
            #ifdef CONFIG_SHADE_VIEW
            /* 擦除上一次绘制的内容 */
            if (view_rect_valid(&shade_rect)) {
                view_render_draw_shade(shade_view, &shade_rect, 0);
            }
            view_rect_t rect;
            rect.x = wx;
            rect.y = wy;
            rect.w.uw = drag_view->width;
            rect.h.uh = drag_view->height;
            view_render_draw_shade(shade_view, &rect, 1); /* 绘制新内容 */
            view_rect_copy(&shade_rect, &rect); /* 保存新区域 */
            if (shade_view->z < 0) { /* 没显示就显示 */
                view_clear(shade_view);
                view_set_z(shade_view, view_env_get_high_level_lower()->z);
            }
            #else
            //view_set_xy(drag_view, wx, wy);
            #endif /* CONFIG_SHADE_VIEW */
            view_mouse_set_state(VIEW_MOUSE_HOLD);
            return 0;
        }
    }
    return -1;  /* 该消息需要进一步处理 */
}

int view_env_do_resize(view_t *view, view_msg_t *msg, int lcmx, int lcmy)
{
    if (view->type == VIEW_TYPE_FIXED) {
        return -1;    
    }
    if (view_region_valid(&view->resize_region)) {
        /* 不在区域里面才能拖拽 */
        if (!view_region_in_range(&view->resize_region, lcmx, lcmy)) {
            if (view_msg_get_type(msg) == VIEW_MSG_MOUSE_LBTN_DOWN) {
                resize_view = view;
                view_mouse.click_x = msg->data0;
                view_mouse.click_y = msg->data1;
            }
            /* 根据鼠标在视图的位置来设置不同的状态 */
            if (lcmx < view->resize_region.left) {
                if (lcmy < view->resize_region.top)
                    view_mouse_set_state(VIEW_MOUSE_DRESIZE2);
                else if (lcmy >= view->resize_region.bottom)
                    view_mouse_set_state(VIEW_MOUSE_DRESIZE1);
                else
                    view_mouse_set_state(VIEW_MOUSE_HRESIZE);
            } else if (lcmx >= view->resize_region.right) {
                if (lcmy < view->resize_region.top)
                    view_mouse_set_state(VIEW_MOUSE_DRESIZE1);
                else if (lcmy >= view->resize_region.bottom)
                    view_mouse_set_state(VIEW_MOUSE_DRESIZE2);
                else
                    view_mouse_set_state(VIEW_MOUSE_HRESIZE);
            } else if (lcmy < view->resize_region.top) {
                view_mouse_set_state(VIEW_MOUSE_VRESIZE);
            } else if (lcmy >= view->resize_region.bottom) {
                view_mouse_set_state(VIEW_MOUSE_VRESIZE);
            }
            return 0;
        } else {
            if (view_mouse.state != VIEW_MOUSE_NORMAL) {
                view_mouse_set_state(VIEW_MOUSE_NORMAL);
                view_mouse.click_x = -1;
                view_mouse.click_y = -1;
            }
            
        }
    }
    return -1;
}

int view_env_init()
{
    
    #ifdef CONFIG_SHADE_VIEW
    shade_view = view_create(0, 0, view_screen.width, view_screen.height);
    if (shade_view == NULL) {
        keprint("create window shade layer failed!\n");
        return -1;
    }
    view_clear(shade_view);
    /* 隐藏图层 */
    view_set_type(shade_view, VIEW_TYPE_FLOAT);
    view_set_z(shade_view, -1);
    view_rect_reset(&shade_rect);
    #endif /* CONFIG_SHADE_VIEW */
    return 0;
}

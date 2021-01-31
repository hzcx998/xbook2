#ifndef _XBOOK_DRIVERS_VIEW_ENV_H
#define _XBOOK_DRIVERS_VIEW_ENV_H

#include "view.h"
#include "msg.h"

view_t *view_env_get_activity();
void view_env_set_activity(view_t *view);

view_t *view_env_get_middle();
void view_env_set_middle(view_t *view);

void view_env_set_hover(view_t *view);

int view_env_try_activate(view_t *view);
void view_env_do_mouse_hover(view_t *view, view_msg_t *msg, int lcmx, int lcmy);
void view_env_do_drag(view_t *view, view_msg_t *msg, int lcmx, int lcmy);
int view_env_do_resize(view_t *view, view_msg_t *msg, int lcmx, int lcmy);
int view_env_filter_mouse_msg(view_msg_t *msg);
int view_env_try_resize(view_t *view, view_rect_t *rect);
int view_env_try_resize_ex(view_t *view, int width, int height);
uint32_t view_env_get_screensize();
uint32_t view_env_get_mousepos();
uint32_t view_env_get_lastpos();

int view_env_reset_hover_and_activity();

typedef struct {
    unsigned long timer_id;
    unsigned long interval;
} viewio_timer_t;

int view_env_add_timer(view_t *view, uint32_t interval);
int view_env_del_timer(view_t *view, unsigned long timer_id);
int view_env_restart_timer(view_t *view, unsigned long timer_id, uint32_t interval);

int view_env_init();
int view_env_exit();

#endif /* _XBOOK_DRIVERS_VIEW_ENV_H */

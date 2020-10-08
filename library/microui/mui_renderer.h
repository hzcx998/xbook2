#ifndef _MUI_RENDERER_H
#define _MUI_RENDERER_H

#include "microui.h"

mu_Context *mu_render_init(const char *title, int x, int y, int width, int height);
void mu_render_draw_rect(mu_Rect rect, mu_Color color);
void mu_render_draw_text(const char *text, mu_Vec2 pos, mu_Color color);
void mu_render_draw_icon(int id, mu_Rect rect, mu_Color color);
 int mu_render_get_text_width(const char *text, int len);
 int mu_render_get_text_height(void);
void mu_render_set_clip_rect(mu_Rect rect);
void mu_render_draw_custom(mu_Rect rect, mu_Color *color);
void mu_render_clear(mu_Color color);
void mu_render_present(void);

int mu_render_exit();
void mu_render_set_frame(void (*frame)(mu_Context *));
void mu_render_set_handler(void (*handler)(mu_Context *));
void mu_render_set_bgcolor(mu_Color color);
void mu_render_loop(mu_Context *ctx);


#endif /* _MUI_RENDERER_H */
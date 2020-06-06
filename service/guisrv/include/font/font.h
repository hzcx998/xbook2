
#ifndef __GUISRV_FONT_FONT_H__
#define __GUISRV_FONT_FONT_H__

#include <types.h>
#include <stdint.h>
#include <srvconfig.h>


/* 字体数量 */
#define GUI_MAX_FONT_NR 10

#define GUI_FONT_NAME_LEN 24
#define GUI_FONT_COPYRIGHT_NAME_LEN 24

typedef struct _gui_font {
	char    name[GUI_FONT_NAME_LEN];	            /* 字体名字 */
	char    copyright[GUI_FONT_COPYRIGHT_NAME_LEN];	    /* 字体版权 */
	uint8_t *addr;		/* 字体数据地址 */
	int     width;      /* 单字宽度 */
    int     height;		/* 单字宽度 */
} gui_font_t;

extern gui_font_t *font_current;

#define current_font    font_current

void gui_init_font();
int gui_register_font(gui_font_t *font);
gui_font_t *gui_get_font(char *name);
int gui_unregister_font(gui_font_t *font);
gui_font_t *gui_select_font(char *name);

/* 导入字体注册 */
#ifdef CONFIG_FONT_STANDARD
extern void gui_register_font_standard();
#endif

#ifdef CONFIG_FONT_SIMSUN
extern void gui_register_font_simsun();
#endif


#endif   /* __GUISRV_FONT_FONT_H__ */

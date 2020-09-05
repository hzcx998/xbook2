
#ifndef _G_FONT_H
#define _G_FONT_H

#include <types.h>
#include <stdint.h>

/* 字体数量 */
#define G_MAX_FONT_NR 2

#define G_FONT_NAME_LEN 24
#define G_FONT_COPYRIGHT_NAME_LEN 24

typedef struct {
	char name[G_FONT_NAME_LEN];	            /* 字体名字 */
	char copyright[G_FONT_COPYRIGHT_NAME_LEN];	    /* 字体版权 */
	unsigned char *addr;		/* 字体数据地址 */
	unsigned int char_width;  /* 单字宽度 */
    unsigned int char_height; /* 单字宽度 */
    unsigned int word_space;  /* 字间距 */
    unsigned int line_space;  /* 行间距 */
} g_font_t;

extern g_font_t *_g_font_current;

#define g_current_font    _g_font_current

int g_init_font();
int g_register_font(g_font_t *font);
g_font_t *g_get_font(char *name);
int g_unregister_font(g_font_t *font);
g_font_t *g_select_font(char *name);

#define G_CONFIG_FONT_STANDARD
#define G_CONFIG_FONT_SIMSUN

/* 导入字体注册 */
#ifdef G_CONFIG_FONT_STANDARD
extern int g_register_font_standard();
#endif

#ifdef G_CONFIG_FONT_SIMSUN
extern int g_register_font_simsun();
#endif

#endif /* _G_FONT_H */

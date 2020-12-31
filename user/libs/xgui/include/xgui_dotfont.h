
#ifndef _XGUI_DOTFONT_H
#define _XGUI_DOTFONT_H

#include <types.h>
#include <stdint.h>

/* 字体数量 */
#define XGUI_MAX_DOTFONT_NR 2

#define XGUI_DOTFONT_NAME_LEN 24
#define XGUI_DOTFONT_COPYRIGHT_NAME_LEN 24

typedef struct {
	char name[XGUI_DOTFONT_NAME_LEN];	            /* 字体名字 */
	char copyright[XGUI_DOTFONT_COPYRIGHT_NAME_LEN];	    /* 字体版权 */
	unsigned char *addr;		/* 字体数据地址 */
	unsigned int char_width;  /* 单字宽度 */
    unsigned int char_height; /* 单字宽度 */
    unsigned int word_space;  /* 字间距 */
    unsigned int line_space;  /* 行间距 */
} xgui_dotfont_t;

xgui_dotfont_t *xgui_dotfont_current();

int xgui_dotfont_init();
int xgui_register_dotfont(xgui_dotfont_t *font);
xgui_dotfont_t *xgui_get_dotfont(char *name);
int xgui_unregister_dotfont(xgui_dotfont_t *font);
xgui_dotfont_t *xgui_select_dotfont(char *name);

extern int xgui_register_dotfont_standard();

#endif /* _XGUI_DOTFONT_H */

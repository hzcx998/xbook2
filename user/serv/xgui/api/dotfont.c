#include <string.h>
#include "xgui_dotfont.h"

/* 字体表，存放字体信息 */
static xgui_dotfont_t xgui_dotfont_table[XGUI_MAX_DOTFONT_NR];

static xgui_dotfont_t *__xgui_dotfont_current;

xgui_dotfont_t *xgui_dotfont_current()
{
    return __xgui_dotfont_current;
}

xgui_dotfont_t *xgui_get_dotfont(char *name)
{
    int i;
	for (i = 0; i < XGUI_MAX_DOTFONT_NR; i++) {
		if (!strcmp(xgui_dotfont_table[i].name, name)) {
			return &xgui_dotfont_table[i];
		}
	}
	return NULL;
}

int xgui_register_dotfont(xgui_dotfont_t *font)
{
    int i;
	for (i = 0; i < XGUI_MAX_DOTFONT_NR; i++) {
		if (xgui_dotfont_table[i].addr == NULL) {
			/* 注册数据 */
            xgui_dotfont_table[i] = *font;
			return 0;
		}
	}
	return -1;
}

int xgui_unregister_dotfont(xgui_dotfont_t *font)
{
    int i;
	for (i = 0; i < XGUI_MAX_DOTFONT_NR; i++) {
		if (&xgui_dotfont_table[i] == font) {
			xgui_dotfont_table[i].char_width = 0;
			xgui_dotfont_table[i].char_height = 0;
			xgui_dotfont_table[i].addr = NULL;		
			return 0;
		}
	}
	return -1;
}

xgui_dotfont_t *xgui_select_dotfont(char *name)
{
    xgui_dotfont_t *font = xgui_get_dotfont(name);
    if (font)
        __xgui_dotfont_current = font;
    return font;
}

int xgui_dotfont_init()
{
    int i;
	for (i = 0; i < XGUI_MAX_DOTFONT_NR; i++) {
		xgui_dotfont_table[i].addr = NULL;
		xgui_dotfont_table[i].char_width = xgui_dotfont_table[i].char_height = 0;
        xgui_dotfont_table[i].word_space = xgui_dotfont_table[i].line_space = 0;
	}
	__xgui_dotfont_current = NULL;
    if (xgui_register_dotfont_standard())
        return -1;
    
    /* 选择一个默认字体 */
    if (xgui_select_dotfont("standard") == NULL)
        return -1;
    return 0;
}


#include <string.h>

#include <font/font.h>
#include <drivers/screen.h>

/* 字体表，存放字体信息 */
gui_font_t font_table[GUI_MAX_FONT_NR];

gui_font_t *font_current;

gui_font_t *gui_get_font(char *name)
{
    int i;
	for (i = 0; i < GUI_MAX_FONT_NR; i++) {
		if (!strcmp(font_table[i].name, name)) {
			return &font_table[i];
		}
	}
	return NULL;
}

int gui_register_font(gui_font_t *font)
{
    int i;
	for (i = 0; i < GUI_MAX_FONT_NR; i++) {
		if (font_table[i].addr == NULL) {
			/* 注册数据 */
            font_table[i] = *font;
			return 0;
		}
	}
	return -1;
}

int gui_unregister_font(gui_font_t *font)
{
    int i;
	for (i = 0; i < GUI_MAX_FONT_NR; i++) {
		if (&font_table[i] == font) {
			font_table[i].width = 0;
			font_table[i].height = 0;
			font_table[i].addr = NULL;		
			return 0;
		}
	}
	return -1;
}

gui_font_t *gui_select_font(char *name)
{
    gui_font_t *font = gui_get_font(name);
    if (font)
        font_current = font;
    return font;
}

void gui_init_font()
{
    int i;
	for (i = 0; i < GUI_MAX_FONT_NR; i++) {
		font_table[i].addr = NULL;
		font_table[i].width = font_table[i].height = 0;
	}
	font_current = NULL;
    /* 注册字体 */
    
#ifdef CONFIG_FONT_SIMSUN
    gui_register_font_simsun();
#endif /* CONFIG_FONT_SIMSUN */

#ifdef CONFIG_FONT_STANDARD
    gui_register_font_standard();
#endif /* CONFIG_FONT_STANDARD */

    /* 选择一个默认字体 */
    gui_select_font("standard");
}


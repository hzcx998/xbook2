#include <string.h>
#include <gfont.h>

/* 字体表，存放字体信息 */
g_font_t _g_font_table[G_MAX_FONT_NR];

g_font_t *_g_font_current;

g_font_t *g_get_font(char *name)
{
    int i;
	for (i = 0; i < G_MAX_FONT_NR; i++) {
		if (!strcmp(_g_font_table[i].name, name)) {
			return &_g_font_table[i];
		}
	}
	return NULL;
}

int g_register_font(g_font_t *font)
{
    int i;
	for (i = 0; i < G_MAX_FONT_NR; i++) {
		if (_g_font_table[i].addr == NULL) {
			/* 注册数据 */
            _g_font_table[i] = *font;
			return 0;
		}
	}
	return -1;
}

int g_unregister_font(g_font_t *font)
{
    int i;
	for (i = 0; i < G_MAX_FONT_NR; i++) {
		if (&_g_font_table[i] == font) {
			_g_font_table[i].char_width = 0;
			_g_font_table[i].char_height = 0;
			_g_font_table[i].addr = NULL;		
			return 0;
		}
	}
	return -1;
}

g_font_t *g_select_font(char *name)
{
    g_font_t *font = g_get_font(name);
    if (font)
        _g_font_current = font;
    return font;
}

int g_init_font()
{
    int i;
	for (i = 0; i < G_MAX_FONT_NR; i++) {
		_g_font_table[i].addr = NULL;
		_g_font_table[i].char_width = _g_font_table[i].char_height = 0;
        _g_font_table[i].word_space = _g_font_table[i].line_space = 0;
	}
	_g_font_current = NULL;
    /* 注册字体 */
    
#ifdef G_CONFIG_FONT_SIMSUN
    if (g_register_font_simsun() < 0)
        return -1;
#endif /* CONFIG_FONT_SIMSUN */

#ifdef G_CONFIG_FONT_STANDARD
    if (g_register_font_standard())
        return -1;
#endif /* CONFIG_FONT_STANDARD */

    /* 选择一个默认字体 */
    if (g_select_font("standard") == NULL)
        return -1;
    return 0;
}


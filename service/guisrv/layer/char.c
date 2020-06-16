#include <layer/point.h>
#include <layer/char.h>
#include <drivers/screen.h>

#include <stdio.h>

static void layer_draw_word_bit(
    layer_t *layer,
    int x,
    int y,
    GUI_COLOR color,
    uint8_t *data
) {
	unsigned int i;
	uint8_t d /* data */;
	for (i = 0; i < 16; i++) {
		d = data[i];
		if ((d & 0x80) != 0)
            layer_put_point(layer, x + 0, y + i, color);
		if ((d & 0x40) != 0)
            layer_put_point(layer, x + 1, y + i, color);
		if ((d & 0x20) != 0)
             layer_put_point(layer, x + 2, y + i, color);
		if ((d & 0x10) != 0)
            layer_put_point(layer, x + 3, y + i, color);
		if ((d & 0x08) != 0)
            layer_put_point(layer, x + 4, y + i, color);
		if ((d & 0x04) != 0)
            layer_put_point(layer, x + 5, y + i, color);
		if ((d & 0x02) != 0)
            layer_put_point(layer, x + 6, y + i, color);
		if ((d & 0x01) != 0)
            layer_put_point(layer, x + 7, y + i, color);
	}	
}

void layer_draw_word_ex(
    layer_t *layer,
    int x,
    int y,
    char word,
    GUI_COLOR color,
    gui_font_t *font
) {
    if (!layer || !font)
        return;
    layer_draw_word_bit(layer, x, y, color, font->addr + word * font->height);
}

void layer_draw_text_ex(
    layer_t *layer,
    int x,
    int y,
    char *text,
    GUI_COLOR color,
    gui_font_t *font
) {
    if (!layer || !font)
        return;

    while (*text) {
        layer_draw_word_ex(layer, x, y, *text, color, font);
		x += font->width;
        text++;
	}
}

void layer_draw_word(
    layer_t *layer,
    int x,
    int y,
    char ch,
    GUI_COLOR color
) {
    layer_draw_word_ex(layer, x, y, ch, color, current_font);
}

void layer_draw_text(
    layer_t *layer,
    int x,
    int y,
    char *text,
    GUI_COLOR color
) {
    layer_draw_text_ex(layer, x, y, text, color, current_font);
}

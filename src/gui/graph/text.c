#include <gui/screen.h>
#include <gui/text.h>

static void gui_draw_word_bit(
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
            gui_screen.output_pixel(x + 0, y + i, color);
		if ((d & 0x40) != 0)
            gui_screen.output_pixel(x + 1, y + i, color);
		if ((d & 0x20) != 0)
             gui_screen.output_pixel(x + 2, y + i, color);
		if ((d & 0x10) != 0)
            gui_screen.output_pixel(x + 3, y + i, color);
		if ((d & 0x08) != 0)
            gui_screen.output_pixel(x + 4, y + i, color);
		if ((d & 0x04) != 0)
            gui_screen.output_pixel(x + 5, y + i, color);
		if ((d & 0x02) != 0)
            gui_screen.output_pixel(x + 6, y + i, color);
		if ((d & 0x01) != 0)
            gui_screen.output_pixel(x + 7, y + i, color);
	}	
}

void gui_draw_word_ex(
    int x,
    int y,
    char word,
    GUI_COLOR color,
    gui_font_t *font
) {
    if (!font)
        return;
    gui_draw_word_bit(x, y, color, font->addr + word * font->height);
}

void gui_draw_text_ex(
    int x,
    int y,
    char *text,
    GUI_COLOR color,
    gui_font_t *font
) {
    if (!font)
        return;

    while (*text) {
        gui_draw_word_ex(x, y, *text, color, font);
		x += font->width;
        text++;
	}
}

void gui_draw_word(
    int x,
    int y,
    char ch,
    GUI_COLOR color
) {
    gui_draw_word_ex(x, y, ch, color, current_font);
}

void gui_draw_text(
    int x,
    int y,
    char *text,
    GUI_COLOR color
) {
    gui_draw_text_ex(x, y, text, color, current_font);
}


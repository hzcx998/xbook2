/**
	This file is part of FORTMAX kernel.

	FORTMAX kernel is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	FORTMAX kernel is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with FORTMAX kernel.  If not, see <http://www.gnu.org/licenses/>.

	Copyright: Martin K. Schröder (info@fortmax.se) 2014
*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <gapi.h>


// states
enum {
	STATE_IDLE,
	STATE_ESCAPE,
	STATE_COMMAND
};

// events that are passed into states
enum {
	EV_CHAR = 1,
};

#define MAX_COMMAND_ARGS 4

#define KEY_ESC 0x1b
#define KEY_DEL 0x7f
#define KEY_BELL 0x07

struct vt100 {
	union flags {
		uint8_t val;
		struct {
			// 0 = cursor remains on last column when it gets there
			// 1 = lines wrap after last column to next line
			uint8_t cursor_wrap : 1;
			uint8_t scroll_mode : 1;
			uint8_t origin_mode : 1;
		};
	} flags;

	//uint16_t screen_width, screen_height;
	// cursor position on the screen (0, 0) = top left corner.
	int16_t cursor_x, cursor_y;
	int16_t saved_cursor_x, saved_cursor_y; // used for cursor save restore
	int16_t scroll_start_row, scroll_end_row;
	// character width and height
	int char_width, char_height;
	int font_width, font_height;
	int screen_width, screen_height;
	int width, height;
	// colors used for rendering current characters
	uint32_t back_color, front_color;
	// the starting y-position of the screen scroll
	uint16_t scroll_value;
	// command arguments that get parsed as they appear in the terminal
	uint8_t narg;
	uint16_t args[MAX_COMMAND_ARGS];
	// current arg pointer (we use it for parsing)
	uint8_t carg;

	void (*state)(struct vt100 *term, uint8_t ev, uint16_t arg);
	void (*send_response)(char *str);
	void (*ret_state)(struct vt100 *term, uint8_t ev, uint16_t arg);

	void (*draw_char)(struct vt100 *term, uint16_t x, uint16_t y, uint8_t ch);
	void (*fill_rect)(struct vt100 *term, uint16_t x, uint16_t y, uint16_t w, uint16_t h, int front);
	void (*scroll)(struct vt100 *term, int lines);
};

void vt100_init(struct vt100 *term);
void vt100_puts(struct vt100 *term, const char *str, size_t size);
#ifdef __cplusplus
}
#endif

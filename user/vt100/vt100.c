/**
	This file is part of FORTMAX.

	FORTMAX is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	FORTMAX is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with FORTMAX.  If not, see <http://www.gnu.org/licenses/>.

	Copyright: Martin K. Schröder (info@fortmax.se) 2014
*/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include "vt100.h"

void vt100_putc(struct vt100 *term, uint8_t c);

#define STATE(NAME, TERM, EV, ARG) void NAME(struct vt100 *TERM, uint8_t EV, uint16_t ARG)

STATE(_st_idle, term, ev, arg);
STATE(_st_esc_sq_bracket, term, ev, arg);
STATE(_st_esc_question, term, ev, arg);
STATE(_st_esc_hash, term, ev, arg);

void _vt100_reset(struct vt100 *term) {
	term->back_color = 0xff000000;
	term->front_color = 0xffffffff;
	term->cursor_x = term->cursor_y = term->saved_cursor_x = term->saved_cursor_y = 0;
	term->narg = 0;
	term->state = _st_idle;
	term->ret_state = 0;
	term->scroll_value = 0;
	term->scroll_start_row = 0;
	term->scroll_end_row = term->height - 1; // outside of screen = whole screen scrollable
	term->flags.cursor_wrap = 0;
	term->flags.origin_mode = 0;

    /* reset color */

	term->fill_rect(term, 0, 0, term->width, term->height, 0);
}

void _vt100_resetScroll(struct vt100 *term) {
	term->scroll_start_row = 0;
	term->scroll_end_row = term->height - 1;
	term->scroll_value = 0;
	term->scroll(term, 0);
}

void _vt100_clearLines(struct vt100 *term, uint16_t start_line, uint16_t end_line) {
	term->fill_rect(term, 0, start_line, term->width, end_line - start_line + 1, 0);
}


// moves the cursor relative to current cursor position and scrolls the screen
void _vt100_move(struct vt100 *term, int16_t right_left, int16_t bottom_top) {
	// calculate how many lines we need to move down or up if x movement goes outside screen
	int16_t new_x = right_left + term->cursor_x;
	if (new_x >= term->width) {
		if (term->flags.cursor_wrap) {
			bottom_top += new_x / term->width;
			term->cursor_x = new_x % term->width;
		} else {
			term->cursor_x = term->width - 1;
		}
	} else if (new_x < 0) {
		bottom_top += new_x / term->width - 1;
//#define abs(x) (((x)>0)?(x):(-(x)))
		term->cursor_x = term->width - (abs(new_x) % term->width) + 1;
	} else {
		term->cursor_x = new_x;
	}

	if (bottom_top) {
		int16_t new_y = term->cursor_y + bottom_top;
		int16_t to_scroll = 0;
		// bottom margin 39 marks last line as static on 40 line display
		// therefore, we would scroll when new cursor has moved to line 39
		// (or we could use new_y > term->height here
		// NOTE: new_y >= term->scroll_end_row ## to_scroll = (new_y - term->scroll_end_row) +1
		if (new_y >= term->scroll_end_row) {
			//scroll = new_y / term->height;
			//term->cursor_y = term->height;
			to_scroll = new_y - term->scroll_end_row;
			// place cursor back within the scroll region
			term->cursor_y = term->scroll_end_row; //new_y - to_scroll;
			//scroll = new_y - term->bottom_margin;
			//term->cursor_y = term->bottom_margin;
		} else if (new_y < term->scroll_start_row) {
			to_scroll = (new_y - term->scroll_start_row);
			term->cursor_y = term->scroll_start_row; //new_y - to_scroll;
			//scroll = new_y / (term->bottom_margin - term->top_margin) - 1;
			//term->cursor_y = term->top_margin;
		} else {
			// otherwise we move as normal inside the screen
			term->cursor_y = new_y;
		}
		term->scroll(term, to_scroll);
	}
}

void _vt100_drawCursor(struct vt100 *t) {
    t->fill_rect(t, t->cursor_x, t->cursor_y, 1, 1, 1);
}

// sends the character to the display and updates cursor position
void _vt100_putc(struct vt100 *term, uint8_t ch) {
	if (ch < 0x20 || ch > 0x7e) {
		static const char hex[] = "0123456789abcdef";
		_vt100_putc(term, '0');
		_vt100_putc(term, 'x');
		_vt100_putc(term, hex[((ch & 0xf0) >> 4)]);
		_vt100_putc(term, hex[(ch & 0x0f)]);
		return;
	}

	term->draw_char(term, term->cursor_x, term->cursor_y, ch);

	// move cursor right
	_vt100_move(term, 1, 0);
	//_vt100_drawCursor(term);
    _vt100_drawCursor(term);
}

void vt100_puts(struct vt100 *term, const char *str, size_t size) {
	for(size_t i = 0; *str && i < size; i++)
	{
		vt100_putc(term, *str++);
	}
}

STATE(_st_command_arg, term, ev, arg) {
	switch (ev) {
		case EV_CHAR: {
			if (isdigit(arg)) { // a digit argument
				term->args[term->narg] = term->args[term->narg] * 10 + (arg - '0');
			} else if (arg == ';') { // separator
				term->narg++;
			} else { // no more arguments
				// go back to command state
				term->narg++;
				if (term->ret_state) {
					term->state = term->ret_state;
				} else {
					term->state = _st_idle;
				}
				// execute next state as well because we have already consumed a char!
				term->state(term, ev, arg);
			}
			break;
		}
	}
}

STATE(_st_esc_sq_bracket, term, ev, arg) {
	switch (ev) {
		case EV_CHAR: {
			if (isdigit(arg)) { // start of an argument
				term->ret_state = _st_esc_sq_bracket;
				_st_command_arg(term, ev, arg);
				term->state = _st_command_arg;
			} else if (arg == ';') { // arg separator.
				// skip. And also stay in the command state
			} else { // otherwise we execute the command and go back to idle
				switch (arg) {
					case 'A': {// move cursor up (cursor stops at top margin)
						int n = (term->narg > 0) ? term->args[0] : 1;
						term->cursor_y -= n;
						if (term->cursor_y < 0) term->cursor_y = 0;
						term->state = _st_idle;
						break;
					}
					case 'B': { // cursor down (cursor stops at bottom margin)
						int n = (term->narg > 0) ? term->args[0] : 1;
						term->cursor_y += n;
						if (term->cursor_y >= term->height) term->cursor_y = term->height - 1;
						term->state = _st_idle;
						break;
					}
					case 'C': { // cursor right (cursor stops at right margin)
						int n = (term->narg > 0) ? term->args[0] : 1;
						term->cursor_x += n;
						if (term->cursor_x >= term->width) term->cursor_x = term->width - 1;
						term->state = _st_idle;
						break;
					}
					case 'D': { // cursor left
						int n = (term->narg > 0) ? term->args[0] : 1;
						term->cursor_x -= n;
						if (term->cursor_x < 0) term->cursor_x = 0;
						term->state = _st_idle;
						break;
					}
					case 'f':
					case 'H': { // move cursor to position (default 0;0)
						// cursor stops at respective margins
						term->cursor_x = (term->narg >= 1) ? (term->args[1] - 1) : 0;
						term->cursor_y = (term->narg == 2) ? (term->args[0] - 1) : 0;
						if (term->flags.origin_mode) {
							term->cursor_y += term->scroll_start_row;
							if (term->cursor_y > term->scroll_end_row)
								term->cursor_y = term->scroll_end_row;
						}
						if (term->cursor_x >= term->width) term->cursor_x = term->width - 1;
						if (term->cursor_y >= term->height) term->cursor_y = term->height - 1;
						term->state = _st_idle;
						break;
					}
					case 'J': { // clear screen from cursor up or down
						if (term->narg == 0 || (term->narg == 1 && term->args[0] == 0)) {
							// clear down to the bottom of screen (including cursor)
							_vt100_clearLines(term, term->cursor_y, term->height - 1);
						} else if (term->narg == 1 && term->args[0] == 1) {
							// clear top of screen to current line (including cursor)
							_vt100_clearLines(term, 0, term->cursor_y);
						} else if (term->narg == 1 && term->args[0] == 2) {
							// clear whole screen
							_vt100_clearLines(term, 0, term->height - 1);
							// reset scroll value
							_vt100_resetScroll(term);
						}
						term->state = _st_idle;
						break;
					}
					case 'K': { // clear line from cursor right/left
						if (term->narg == 0 || (term->narg == 1 && term->args[0] == 0)) {
							// clear to end of line (to \n or to edge?)
							// including cursor
							term->fill_rect(term, term->cursor_x, term->cursor_y, term->width - term->cursor_x, 1, 0);
						} else if (term->narg == 1 && term->args[0] == 1) {
							// clear from left to current cursor position
							term->fill_rect(term, 0, term->cursor_y, term->cursor_x + 1, 1, 0);
						} else if (term->narg == 1 && term->args[0] == 2) {
							// clear whole current line
							term->fill_rect(term, 0, term->cursor_y, term->width, 1, 0);
						}
						term->state = _st_idle;
						break;
					}

					case 'L': // insert lines (args[0] = number of lines)
					case 'M': // delete lines (args[0] = number of lines)
						term->state = _st_idle;
						break;
					case 'P': {// delete characters args[0] or 1 in front of cursor
						// TODO: this needs to correctly delete n chars
						int n = ((term->narg > 0) ? term->args[0] : 1);
						_vt100_move(term, -n, 0);
						for (int c = 0; c < n; c++) {
							_vt100_putc(term, ' ');
						}
						term->state = _st_idle;
						break;
					}
					case 'c': { // query device code
						term->send_response("\e[?1;0c");
						term->state = _st_idle;
						break;
					}
					case 'x': {
						term->state = _st_idle;
						break;
					}
					case 's': { // save cursor pos
						term->saved_cursor_x = term->cursor_x;
						term->saved_cursor_y = term->cursor_y;
						term->state = _st_idle;
						break;
					}
					case 'u': { // restore cursor pos
						term->cursor_x = term->saved_cursor_x;
						term->cursor_y = term->saved_cursor_y;
						//_vt100_moveCursor(term, term->saved_cursor_x, term->saved_cursor_y);
						term->state = _st_idle;
						break;
					}
					case 'h':
					case 'l': {
						term->state = _st_idle;
						break;
					}

					case 'g': {
						term->state = _st_idle;
						break;
					}
					case 'm': { // sets colors. Accepts up to 3 args
						// [m means reset the colors to default
						if (!term->narg) {
							term->front_color = 0xffffffff;
							term->back_color = 0xff000000;
						}
						while (term->narg) {
							term->narg--;
							int n = term->args[term->narg];
							static const uint32_t colors[] = {
								0xff000000, // black
								0xffff0000, // red
								0xff00ff00, // green
								0xffffff00, // yellow
								0xff0000ff, // blue
								0xffff00ff, // magenta
								0xff00ffff, // cyan
								0xffffffff, // white
							};
							if (n == 0) { // all attributes off
								term->front_color = 0xffffffff;
								term->back_color = 0xff000000;
							}
							if (n >= 30 && n < 38) { // fg colors
								term->front_color = colors[n - 30];
							} else if (n >= 40 && n < 48) {
								term->back_color = colors[n - 40];
							}
						}
						term->state = _st_idle;
						break;
					}
					case 'n':
					{
						if(term->narg==1 && term->args[0]==6)
						{
							char buf[50]={0};
							sprintf(buf,"\e%d;%dR",term->cursor_y,term->cursor_x);
							//TIOCSWINSZ ubuntu 
							term->send_response(buf);
						}
					}

						term->state = _st_idle;
					case '@': // Insert Characters
						term->state = _st_idle;
						break;
					case 'r': // Set scroll region (top and bottom margins)
						// the top value is first row of scroll region
						// the bottom value is the first row of static region after scroll
						if (term->narg == 2 && term->args[0] < term->args[1]) {
							// [1;40r means scroll region between 8 and 312
							// bottom margin is 320 - (40 - 1) * 8 = 8 pix
							term->scroll_start_row = term->args[0] - 1;
							term->scroll_end_row = term->args[1] - 1;
							//TODO:错误处理
							if(term->scroll_end_row >= term->height) term->scroll_end_row = term->height - 1;
							
							///term->scroll(term->scroll_start_row, term->scroll_end_row, term->scroll_start_row + term->scroll_value); // reset scroll
							//TEST:
							term->cursor_x = term->cursor_y = 0;
						}
						//  else {
						// 	_vt100_resetScroll(term);
						// }
						term->state = _st_idle;
						break;
					case 'i': // Printing
					case 'y': // self test modes..
					case '=': { // argument follows...
						//term->state = _st_screen_mode;
						term->state = _st_idle;
						break;
					}
					case '?': // '[?' escape mode
						term->state = _st_esc_question;
						break;
					default: { // unknown sequence

						term->state = _st_idle;
						break;
					}
				}
				//term->state = _st_idle;
			} // else
			break;
		}
		default: { // switch (ev)
			// for all other events restore normal mode
			term->state = _st_idle;
		}
	}
}

STATE(_st_esc_question, term, ev, arg) {
	// DEC mode commands
	switch (ev) {
		case EV_CHAR: {
			if (isdigit(arg)) { // start of an argument
				term->ret_state = _st_esc_question;
				_st_command_arg(term, ev, arg);
				term->state = _st_command_arg;
			} else if (arg == ';') { // arg separator.
				// skip. And also stay in the command state
			} else {
				switch (arg) {
					case 'l':
						// dec mode: OFF (arg[0] = function)
					case 'h': {
						// dec mode: ON (arg[0] = function)
						switch (term->args[0]) {
							case 1: { // cursor keys mode
								// h = esc 0 A for cursor up
								// l = cursor keys send ansi commands
								break;
							}
							case 2: { // ansi / vt52
								// h = ansi mode
								// l = vt52 mode
								break;
							}
							case 3: {
								// h = 132 chars per line
								// l = 80 chars per line
								break;
							}
							case 4: {
								// h = smooth scroll
								// l = jump scroll
								break;
							}
							case 5: {
								// h = black on white bg
								// l = white on black bg
								break;
							}
							case 6: {
								// h = cursor relative to scroll region
								// l = cursor independent of scroll region
								term->flags.origin_mode = (arg == 'h') ? 1 : 0;
								break;
							}
							case 7: {
								// h = new line after last column
								// l = cursor stays at the end of line
								term->flags.cursor_wrap = (arg == 'h') ? 1 : 0;
								break;
							}
							case 8: {
								// h = keys will auto repeat
								// l = keys do not auto repeat when held down
								break;
							}
							case 9: {
								// h = display interlaced
								// l = display not interlaced
								break;
							}
							// 10-38 - all quite DEC speciffic commands so omitted here
						}
						term->state = _st_idle;
						break;
					}
					case 'i': /* Printing */
					case 'n': /* Request printer status */
					default:
						term->state = _st_idle;
						break;
				}
				term->state = _st_idle;
			}
		}
	}
}

STATE(_st_esc_left_br, term, ev, arg) {
	switch (ev) {
		case EV_CHAR: {
			switch (arg) {
				case 'A':
				case 'B':
					// translation map command?
				case '0':
				case 'O':
					// another translation map command?
					term->state = _st_idle;
					break;
				default:
					term->state = _st_idle;
			}
			//term->state = _st_idle;
		}
	}
}

STATE(_st_esc_right_br, term, ev, arg) {
	switch (ev) {
		case EV_CHAR: {
			switch (arg) {
				case 'A':
				case 'B':
					// translation map command?
				case '0':
				case 'O':
					// another translation map command?
					term->state = _st_idle;
					break;
				default:
					term->state = _st_idle;
			}
			//term->state = _st_idle;
		}
	}
}

STATE(_st_esc_hash, term, ev, arg) {
	switch (ev) {
		case EV_CHAR: {
			switch (arg) {
				case '8': {
					// self test: fill the screen with 'E'

					term->state = _st_idle;
					break;
				}
				default:
					term->state = _st_idle;
			}
		}
	}
}

STATE(_st_escape, term, ev, arg) {
	switch (ev) {
		case EV_CHAR: {
#define CLEAR_ARGS \
	{\
		term->narg = 0;\
		for(int c = 0; c < MAX_COMMAND_ARGS; c++)\
		term->args[c] = 0;\
	}
			switch (arg) {
				case '[': { // command
					// prepare command state and switch to it
					CLEAR_ARGS;
					term->state = _st_esc_sq_bracket;
					break;
				}
				case '(': /* ESC ( */
					CLEAR_ARGS;
					term->state = _st_esc_left_br;
					break;
				case ')': /* ESC ) */
					CLEAR_ARGS;
					term->state = _st_esc_right_br;
					break;
				case '#': // ESC #
					CLEAR_ARGS;
					term->state = _st_esc_hash;
					break;
				case 'P': //ESC P (DCS, Device Control String)
					term->state = _st_idle;
					break;
				case 'D': // moves cursor down one line and scrolls if necessary
					// move cursor down one line and scroll window if at bottom line
					_vt100_move(term, 0, 1);
					term->state = _st_idle;
					break;
				case 'M': // Cursor up
					// move cursor up one line and scroll window if at top line
					_vt100_move(term, 0, -1);
					term->state = _st_idle;
					break;
				case 'E': // next line
					// same as '\r\n'
					_vt100_move(term, 0, 1);
					term->cursor_x = 0;
					term->state = _st_idle;
					break;
				case '7': // Save attributes and cursor position
				case 's':
					term->saved_cursor_x = term->cursor_x;
					term->saved_cursor_y = term->cursor_y;
					term->state = _st_idle;
					break;
				case '8': // Restore them
				case 'u':
					term->cursor_x = term->saved_cursor_x;
					term->cursor_y = term->saved_cursor_y;
					term->state = _st_idle;
					break;
				case '=': // Keypad into applications mode
					term->state = _st_idle;
					break;
				case '>': // Keypad into numeric mode
					term->state = _st_idle;
					break;
				case 'Z': // Report terminal type
					// vt 100 response
					term->send_response("\033[?1;0c");
					// unknown terminal
					//out("\033[?c");
					term->state = _st_idle;
					break;
				case 'c': // Reset terminal to initial state
					_vt100_reset(term);
					term->state = _st_idle;
					break;
				case 'H': // Set tab in current position
				case 'N': // G2 character set for next character only
				case 'O': // G3 "               "
				case '<': // Exit vt52 mode
					// ignore
					term->state = _st_idle;
					break;
				case KEY_ESC: { // marks start of next escape sequence
					// stay in escape state
					break;
				}
				default: { // unknown sequence - return to normal mode
					term->state = _st_idle;
					break;
				}
			}
#undef CLEAR_ARGS
			break;
		}
		default: {
			// for all other events restore normal mode
			term->state = _st_idle;
		}
	}
}

STATE(_st_idle, term, ev, arg) {
	switch (ev) {
		case EV_CHAR: {
			switch (arg) {

				case 5: // AnswerBack for vt100's
					term->send_response("X"); // should send SCCS_ID?
					break;
				case '\n': { // new line
					_vt100_move(term, 0, 1);
					term->cursor_x = 0;
					//_vt100_moveCursor(term, 0, term->cursor_y + 1);
					// do scrolling here!
					break;
				}
				case '\r': { // carrage return (0x0d)
					term->cursor_x = 0;
					_vt100_move(term, 0, 1);
					//_vt100_moveCursor(term, 0, term->cursor_y);
					break;
				}
				case '\b': { // backspace 0x08
					_vt100_move(term, -1, 0);
					// backspace does not delete the character! Only moves cursor!
					term->draw_char(term, term->cursor_x * term->char_width,
						term->cursor_y * term->char_height, ' ');
                    //ili9340_drawChar(term->cursor_x * term->char_width,
						//term->cursor_y * term->char_height, ' ');
					break;
				}
				case KEY_DEL: { // del - delete character under cursor
					// Problem: with current implementation, we can't move the rest of line
					// to the left as is the proper behavior of the delete character
					// fill the current position with background color
					_vt100_putc(term, ' ');
					_vt100_move(term, -1, 0);
					//_vt100_clearChar(term, term->cursor_x, term->cursor_y);
					break;
				}
				case '\t': { // tab
					// tab fills characters on the line until we reach a multiple of tab_stop
					int tab_stop = 4;
					int to_put = tab_stop - (term->cursor_x % tab_stop);
					while (to_put--) _vt100_putc(term, ' ');
					break;
				}
				case KEY_BELL: { // bell is sent by bash for ex. when doing tab completion
					// sound the speaker bell?
					// skip
					break;
				}
				case KEY_ESC: {// escape
					term->state = _st_escape;
					break;
				}
				default: {
					_vt100_putc(term, arg);
					break;
				}
			}
			break;
		}
		default: {
		}
	}
}

void vt100_init(struct vt100 *term) {
	_vt100_reset(term);
}

void vt100_putc(struct vt100 *term, uint8_t c) {
	//putc(c);
	char *buffer = 0;
	switch(c){
		case GK_UP:         buffer="\e[A";    break;
		case GK_DOWN:       buffer="\e[B";    break;
		case GK_RIGHT:      buffer="\e[C";    break;
		case GK_LEFT:       buffer="\e[D";    break;
		case GK_BACKSPACE:  buffer="\b";      break;
		/*case KEY_IC:         buffer="\e[2~";   break;
		case KEY_DC:         buffer="\e[3~";   break;*/
		case GK_HOME:       buffer="\e[7~";   break;
		case GK_END:        buffer="\e[8~";   break;
		/*case KEY_PPAGE:      buffer="\e[5~";   break;
		case KEY_NPAGE:      buffer="\e[6~";   break;
		case KEY_SUSPEND:    buffer="\x1A";    break;      // ctrl-z*/
		case GK_F1:       buffer="\e[[A";   break;
		case GK_F2:       buffer="\e[[B";   break;
		case GK_F3:       buffer="\e[[C";   break;
		case GK_F4:       buffer="\e[[D";   break;
		case GK_F5:       buffer="\e[[E";   break;
		case GK_F7:       buffer="\e[18~";  break;
		case GK_F6:       buffer="\e[17~";  break;
		case GK_F8:       buffer="\e[19~";  break;
		case GK_F9:       buffer="\e[20~";  break;
		case GK_F10:      buffer="\e[21~";  break;
	}
	if(buffer){
		while(*buffer){
			term->state(term, EV_CHAR, *buffer++);
		}
	} else {
		term->state(term, EV_CHAR, 0x0000 | c);
	}
	// char *s=" ";
	// if(c>=32 && c<=126)
	// {
	// 	printf("%c",c&0xff);
	// }
	// else
	// {
	// 	if(c==27)
	// 	{
	// 		printf("\\e");
	// 	} 
	// 	else
	// 	{
	// 		printf("[%#x:%d]",c&0xff,c&0xff);
	// 	}
		
	// }
    /*
	if(term->state)
		term->state(term, EV_CHAR, 0x0000 | c);*/
	//putchar(c);
}

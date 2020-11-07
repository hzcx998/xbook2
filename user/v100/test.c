#include <stdio.h>
#include <stdlib.h>
/**
	This is a demo for the vt100 functionality. 

	To do the tests, you may have to comment out some of them because all
	the strings take up a lot of ram. :)
	
	Copyright: Martin K. Schröder (info@fortmax.se) 2014/10/27
*/

#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <sys/time.h>

#include "vt100.h"
#include "vt.h"

/**
  Tests following commands:

  Cursor movement (<n>=how many chars or lines), cursor stop at margin.
  Up
    Esc  [ <n> A
    033 133   101
  Down
    Esc  [ <n> B
    033 133   102
  Right
    Esc  [ <n> C
    033 133   103
  Left
    Esc  [  n  D
    033 133   104
  Cursor position  (<n1>=y,<n2>=x, from top of screen or scroll region)
       Esc  [ <n1> ; <n2> H
       033 133    073    110
    Or Esc  [ <n1> ; <n2> f
       033 133    073    146
  Index (cursor down with scroll up when at margin)
    Esc  D
    033 104
  Reverse index (cursor up with scroll down when at margin)
    Esc  M
    033 115
  Next line (CR+Index)
    Esc  E
    033 105
  Save cursor and attribute
    Esc  7
    033 067
  Restore cursor and attribute
    Esc  8
    033 070
*/

void test_cursor(){
	char buf[160]; 
	//FIXME: \e[c 和 \e[2J连用，xterm无法清屏，但是单独echo可以，原因未知
	// clear screen
	_puts("\e[2J\e[m\e[r\e[?6l\e[1;1H");

	// draw a line of "*"
	for(int c = 0; c < vt_term->width; c++){
		_puts("*"); 
	}
	// draw left and right border
	for(int c = 0; c < vt_term->height; c++){
		sprintf(buf, "\e[%d;1H*\e[%d;%dH*", c + 1, c + 1, vt_term->width);
		_puts(buf);
	}
	// draw bottom line
	sprintf(buf, "\e[%d;1H", vt_term->height);
	_puts(buf); 
	for(int c = 0; c < vt_term->width; c++){
		_putc('*');
	}
	// draw inner border of +
	_puts("\e[2;2H");
	// draw a line of "*"
	for(int c = 0; c < vt_term->width - 2; c++){
		_putc('+'); 
	}
	// draw left and right border
	for(int c = 1; c < vt_term->height - 1; c++){
		sprintf(buf, "\e[%d;2H+\e[%d;%dH+", c + 1, c + 1, vt_term->width - 1);
		_puts(buf);
	}
	// draw bottom line
	sprintf(buf, "\e[%d;2H", vt_term->height - 1);
	_puts(buf); 
	for(int c = 0; c < vt_term->width - 2; c++){
		_putc('+');
	}

	// draw middle window
	// EEEEEEEEEEEEEEEEEEEEEEEEEEEEEE
	// E                            E
	// E This must be an unbroken a E
	// E rea of text with 1 free bo E
	// E rder around the text.      E
	// E                            E
	// EEEEEEEEEEEEEEEEEEEEEEEEEEEEEE
	_puts("\e[10;6H");
	for(int c = 0; c < 30; c++){
		_putc('E');
	}
	// test normal movement
	_puts("\e[11;6H");
	// test cursor store and restore...
	_puts("\e7\e[35;10H\e8");
	_puts("E\e[11;35HE");
	// goto 12;6, print E, move cursor 29 (already moved +1) to right and print E
	_puts("\e[12;6HE\e[28CE");
	// move cursor 31 to left, 1 down, print E, move 30 right, print E
	_puts("\e[30D\e[BE\e[28CE");
	_puts("\e[15;6H\e[AE\e[28CE");
	_puts("\e[15;6HE\e[15;35HE"); 
	
	_puts("\e[16;6H");
	for(int c = 0; c < 30; c++){
		_putc('E');
	}

	const char *text[] = {"This must be an unbroken a", "rea of text with 1 free bo", "rder around the text.     "};
	for(int c = 0; c < 3; c++){
		sprintf(buf, "\e[%d;8H", c + 12);
		_puts(buf);
		_puts((char *)text[c]);
	}

	// now lets draw two parallel columns of Es
	_puts("\e[20;19H"); 
	for(int c = 0; c < 10; c++){
		// draw E (cursor moves right), step one right, draw F, step 3 left and 1 down
		_puts("E\e[1CF\e[3D\e[B");
	}
	
	// Test index (escD - down with scroll)
	// Test reverse index (escM)
	// next line (escE) - cr + index
	// save and restore cursor

	// move to last line and scroll down 8 lines
	_puts("\e[40;1H");
	for(int c = 0; c < 7; c++){
		_puts("\eD");
	}
	mdelay(100); 
	// now scroll same number of lines back and then back again (to test up scroll)
	_puts("\e[1;1H");
	for(int c = 0; c < 7; c++){
		_puts("\eM");
	}
	mdelay(100); 
	_puts("\e[40;1H");
	for(int c = 0; c < 7; c++){
		_puts("\eD");
	}
	
	// we now have the Es at the third line (or we SHOULD have)
	// refill the top border and clear bottom borders
	for(int c = 1; c < vt_term->width - 1; c++){
		// we print * then move down and left, print + and go back right and top
		// (good way to test cursor navigation keys)
		sprintf(buf, "\e[1;%dH*\e[B\e[D+\e[A", c + 1); 
		_puts(buf);
	}
	// clear the border that scrolled up
	for(int c = 2; c < vt_term->width - 2; c++){
		// space, down, left, space, up
		sprintf(buf, "\e[32;%dH \e[B\e[D \e[A", c + 1); 
		_puts(buf);
	}
	
	// redraw left and right border
	for(int c = 1; c < vt_term->height; c++){
		sprintf(buf, "\e[%d;1H*+\e[%d;%dH+*", c + 1, c + 1, vt_term->width - 1);
		_puts(buf);
	}
	
	// fill border at the bottom
	for(int c = 1; c < vt_term->width - 1; c++){
		sprintf(buf, "\e[39;%dH+\e[B\e[D*\e[A", c + 1); 
		_puts(buf);
	}
	// draw the explanation string
	_puts("\e[30;6HShould see two columns of E F"); 
	_puts("\e[31;6HText box must start at line 3"); 
}

/**
	Tests setting scroll region and moving the cursor inside the scroll region.

*/

void test_scroll(){
	char buf[160]; 
	// reset terminal and clear screen. Cursor at 1;1. 
	_puts("\e[2J\e[m\e[r\e[?6l\e[1;1H");

	// set top margin 3 lines, bottom margin 5 lines
	_puts("\e[4;29r");

	// draw top and bottom windows
	_puts("\e[1;1H#\e[2;1H#\e[3;1H#\e[1;80H#\e[2;80H#\e[3;80H#");
	_puts("\e[30;1H#\e[31;1H#\e[32;1H#\e[33;1H#\e[34;1H#");
	_puts("\e[30;80H#\e[31;80H#\e[32;80H#\e[33;80H#\e[34;80H#");
	_puts("\e[1;1H"); for(int x = 0; x < vt_term->width; x++) _putc('#'),mdelay(1); 
	_puts("\e[3;1H"); for(int x = 0; x < vt_term->width; x++) _putc('#'),mdelay(1); 
	_puts("\e[30;1H"); for(int x = 0; x < vt_term->width; x++) _putc('#'),mdelay(1); 
	_puts("\e[34;1H"); for(int x = 0; x < vt_term->width; x++) _putc('#'),mdelay(1);

	// print some text that should not move
	_puts("\e[2;4HThis is top text (should not move)"); 
	_puts("\e[32;3HThis is bottom text (should not move)");

	// set origin mode and print border around the scroll region
	_puts("\e[?6h");
	_puts("\e[1;1H"); for(int x = 0; x < vt_term->width; x++) _putc('!'),mdelay(1);
	// origin mode should snap 99 to last line in scroll region
	_puts("\e[99;1H"); for(int x = 0; x < vt_term->width; x++) _putc('!'),mdelay(1);
	for(int y = 0; y < vt_term->height; y++){
		//sprintf(buf, "\e[%d;1H!\e[%d;%dH!", y+1, y+1, vt_term->width);
		sprintf(buf, "\e[%d;1H", y+1);
		_puts(buf);
		for(int c = 0; c < vt_term->width; c++){
			_putc('!');mdelay(1);
		}
	}

	// scroll the scroll region
	_puts("\e[99;1H\eD\eD");
	_puts("\e[1;1H\eM\eM");
	_puts("\e[99;1H\eD");

	// clear out an area in the middle and draw text
	for(int y = 0; y < 5; y++){
		sprintf(buf, "\e[%d;6H", y+10);
		_puts(buf);
	
		for(int c = 0; c < 30; c++){
			_putc(' ');mdelay(1);
		}
	}
	_puts("\e[11;10HMust be ! filled with 2");
	_puts("\e[12;10H    empty lines at");
	_puts("\e[13;10H    top and bottom! ");
	
}

/**

Erasing
  Erase in line
    End of line (including cursor position)
         Esc  [   K
         033 133 113
      Or Esc  [   0   K
         033 133 060 113
    Beginning of line (including cursor position)
      Esc  [   1   K
      033 133 061 113
    Complete line
      Esc  [   2   K
      033 133 062 113
  Erase in display
    End of screen (including cursor position)
         Esc  [   J
         033 133 112
      Or Esc  [   0   J
         033 133 060 112
    Beginning of screen (including cursor position)
      Esc  [   1   J
      033 133 061 112
    Complete display
      Esc  [   2   J
      033 133 062 112
 
 
Computer editing
  Delete characters (<n> characters right from cursor
    Esc  [ <n> P
    033 133   120
  Insert line (<n> lines)
    Esc  [ <n> L
    033 133   114
  Delete line (<n> lines)
    Esc  [ <n> M
    033 133   115
*/

void test_edit(){
	char buf[160]; 

	// clear screen
	_puts("\e[2J\e[m\e[r\e[?6l\e[1;1H");

	// enable auto wrap mode
	_puts("\e[?7h");
	
	// fill entire screen with 'x'
	int size = (vt_term->width * vt_term->height); 
	for(int c = 0; c < size; c++){
		_putc('x');mdelay(1);
	}

	// clear bottom and top halves (will remove all 'x' s)
	_puts("\e[20;1H"); 
	_puts("\e[J");
	mdelay(1000);
	_puts("\e[1J");
	_puts("\e[?7l");
	mdelay(1000); 
	
	// draw left and right borders using erase function
	for(int c = 23; c < vt_term->height; c+=2){
		sprintf(buf, "\e[%d;1H", c + 1);
		_puts(buf);
		// draw two lines of *
		for(int j = 0; j < vt_term->width; j++){
			_puts("*\e[B\e[D*\e[A");mdelay(1);
		}
		// erase end of first line and beginning of second line
		// goto c;3, erase end, goto c;(w-1), write **
		sprintf(buf, "\e[%d;3H\e[0K\e[%d;%dH**",
			c + 1, c + 1, vt_term->width - 1);
		_puts(buf);
		// goto (c+1);(width-2), erase beginning of line, goto (c+2);1, write **
		sprintf(buf, "\e[%d;%dH\e[1K\e[%d;1H**",
			c + 2, vt_term->width - 2, c + 2);
		_puts(buf); 
	}
	
	// fill border at the bottom
	for(int c = 2; c < vt_term->width - 2; c++){
		sprintf(buf, "\e[24;%dH*\e[B\e[D*\e[A", c + 1);mdelay(1);
		_puts(buf);
	}
	// fill border at the bottom
	for(int c = 2; c < vt_term->width - 2; c++){
		sprintf(buf, "\e[33;%dH*\e[B\e[D*\e[A", c + 1);mdelay(1);
		_puts(buf);
	}
	// draw text
	_puts("\e[30;4HYou should see border and NO x:s"); 
}
/**
	Tests terminal colors

	(i just copy the scroll example here and add some colors)
	
	Esc[Value;...;Valuem 	Set Graphics Mode:
		Calls the graphics functions specified by the following values. These specified functions remain active until the next occurrence of this escape sequence. Graphics mode changes the colors and attributes of text (such as bold and underline) displayed on the screen.
		 
		Text attributes
		0	All attributes off 
		1	Bold on (we currently don't support bold on this limited display)
		4	Underscore (not supported)
		5	Blink on (not supported)
		7	Reverse video on (not supported)
		8	Concealed on (not supported)
		 
		Foreground colors
		30	Black
		31	Red
		32	Green
		33	Yellow
		34	Blue
		35	Magenta
		36	Cyan
		37	White
		 
		Background colors
		40	Black
		41	Red
		42	Green
		43	Yellow
		44	Blue
		45	Magenta
		46	Cyan
		47	White
		 
		Parameters 30 through 47 meet the ISO 6429 standard.
*/

void test_colors(){
	char buf[160]; 
	
	// reset terminal and clear screen. Cursor at 1;1.
	// reset all modes
	_puts("\e[2J\e[m\e[r\e[?6l\e[1;1H");

	// set top margin 3 lines, bottom margin 5 lines
	_puts("\e[4;29r");

	// set bg color to red and text to white
	_puts("\e[41;37m");
	
	// draw top and bottom windows
	_puts("\e[1;1H#\e[2;1H#\e[3;1H#\e[1;80H#\e[2;80H#\e[3;80H#");
	_puts("\e[1;1H"); for(int x = 0; x < vt_term->width; x++) _putc('#'),mdelay(1);
	_puts("\e[3;1H"); for(int x = 0; x < vt_term->width; x++) _putc('#'),mdelay(1);

	// background blue
	_puts("\e[44;37m");

	_puts("\e[30;1H#\e[31;1H#\e[32;1H#\e[33;1H#\e[34;1H#");
	_puts("\e[30;80H#\e[31;80H#\e[32;80H#\e[33;80H#\e[34;80H#");
	_puts("\e[30;1H"); for(int x = 0; x < vt_term->width; x++) _putc('#'),mdelay(1);
	_puts("\e[34;1H"); for(int x = 0; x < vt_term->width; x++) _putc('#'),mdelay(1);

	// foreground white
	_puts("\e[37;40m");
	
	// print some text that should not move
	_puts("\e[2;4HThis is top text (should not move)"); 
	_puts("\e[32;3HThis is bottom text (should not move)");

	// green background, black text
	_puts("\e[42;30m");

	// set origin mode and print border around the scroll region
	_puts("\e[?6h");
	_puts("\e[1;1H"); for(int x = 0; x < vt_term->width; x++) _putc('!'),mdelay(1);
	// origin mode should snap 99 to last line in scroll region
	_puts("\e[99;1H"); for(int x = 0; x < vt_term->width; x++) _putc('!'),mdelay(1);
	for(int y = 0; y < vt_term->height; y++){
		//sprintf(buf, "\e[%d;1H!\e[%d;%dH!", y+1, y+1, vt_term->width);
		sprintf(buf, "\e[%d;1H", y+1);mdelay(1);
		_puts(buf);
		for(int c = 0; c < vt_term->width; c++){
			_putc('!');mdelay(1);
		}
	}

	// scroll the scroll region
	_puts("\e[99;1H\eD\eD");
	_puts("\e[1;1H\eM\eM");
	_puts("\e[99;1H\eD");

	// black background, yellow text
	_puts("\e[33;40m");
	
	// clear out an area in the middle and draw text
	for(int y = 0; y < 5; y++){
		sprintf(buf, "\e[%d;6H", y+10);
		_puts(buf);
	
		for(int c = 0; c < 30; c++){
			_putc(' ');mdelay(1);
		}
	}
	_puts("\e[11;10HMust be ! filled with 2");
	_puts("\e[12;10H    empty lines at");
	_puts("\e[13;10H    top and bottom! ");

}

unsigned long vt100_test(unsigned long arg)
{
	test_cursor();
	test_scroll();
	test_edit();
	test_colors();
	return 0;
}

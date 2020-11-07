This is a vt100 emulator written for devices with under 4kb of ram (for example the ATMega microcontroller). The emulator also uses ili9340 display by default, but can be compiled to use another display with only a few changes to the source code.

Note that because of the small size of the available memory and the slow speed of the microcontroller, it is a little tricky to implement all the escape sequences that are supported in vt100.

![VT100 Emulator Demo](https://raw.githubusercontent.com/mkschreder/avr-vt100/master/vt100_emu.jpg)

Examples
--------

* see demo.cpp for code that tests the terminal
* Note: since vt100 is 80x24 lines, and our terminal only supports 40x40 lines, you need to run "stty cols 40 rows 40" command to tell terminal programs that only 40 columns are available. 

Compiling
---------

Don't be put off by cmake. The CMakeLists.txt file is provided for convenience only. You can basically just compile using:
* avr-gcc -O3 -std=c99 -mmcu=atmega328p -DF_CPU=16000000UL -c ili9340.c uart.c vt100.c
* avr-g++ -O3 -std=c++11 -mmcu=atmega328p -DF_CPU=16000000UL -o demo.elf demo.cpp ili9340.o uart.o vt100.o

Compatibility
-------------

The aim of this project is to be a vt100 compliant terminal. However, at the moment it can not really be called a vt100 terminal because it is far from complete in terms of supporting all of the vt100 terminal sequences. But I do support a few and doing normal shell stuff works fine. But some functions require some more hacking to implement with only 2kb of ram (such as cursor blink and character deletion, to name a few). 

Nonetheless, I'm proud to have come this far with it. 

The driver currently supports the following escape sequences:

					 /======================================================\
					 |  VT100 Escape sequences supported by the this driver |
					 \======================================================/

	- ESC is the escape character "\e" or "\x1b"
	- (yes) - the driver supports the sequence
	- (no) - the driver currently does not support the sequence
	- (?) - do we really need this function? Or: what does this do exactly?

	VT52 Compatable Mode
	--------------------

	- (yes) ESC A           Cursor up with scroll
	- (yes) ESC B           Cursor down with scroll
	- (yes) ESC C           Cursor right 
	- (yes) ESC D           Cursor left
	- (?) 	ESC F           Special graphics character set
	- (?) 	ESC G           Select ASCII character set
	- (yes) ESC H           Cursor to home (but margins currently not supported!)
	- (?) ESC I          		Reverse line feed
	- (yes) ESC J           Erase to end of screen
	- (yes) ESC K           Erase to end of line
	- (no) ESC Ylc          Direct cursor address (See note 1)
	- (no) ESC Z            Identify (See note 2)
	- (?) ESC =             Enter alternate keypad mode
	- (?) ESC >             Exit alternate keypad mode
	- (?) ESC 1             Graphics processor on (See note 3)
	- (?) ESC 2             Graphics processor off (See note 3)
	- (?) ESC <             Enter ANSI mode

	ANSI Compatable Mode
	--------------------

	- (yes) ESC [ Pn A    Cursor up Pn lines (without scroll)
	- (yes) ESC [ Pn B    Cursor down Pn lines
	- (yes) ESC [ Pn C    Cursor forward Pn characters (right)
	- (yes) ESC [ Pn D    Cursor backward Pn characters (left)
	- (yes) ESC [ Pl;PcH  Direct cursor addressing, where Pl is line#, Pc is column#
	- NOTE: proper action is to move relative to margins. We don't support margins yet so the moves are absolute screen based character coordinates. 
	- (yes) ESC [ Pl;Pcf  Same as above 
	- (yes) ESC D         Index - moves cursor one line down and performs scroll if at bottom
	- (yes) ESC M         Reverse index - moves cursor up (with eventual scroll)
	- (yes) ESC 7         Save cursor and attributes
	- (yes) ESC 8         Restore cursor and attributes

	- (?) ESC #3          Change this line to double-height top half
	- (?) ESC #4          Change this line to double-height bottom half
	- (?) ESC #5          Change this line to single-width single-height
	- (?) ESC #6          Change this line to double-width single-height

	- (yes) ESC [ Ps..Ps m
									Ps refers to selective parameter. Multiple parameters are

									Text attributes
									0	All attributes off
									1	Bold on
									4	Underscore (on monochrome display adapter only)
									5	Blink on
									7	Reverse video on
									8	Concealed on
									 
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
									 
	- (yes) ESC [ K   Erase from cursor to end of line
	- (yes) ESC [ 0K        Same
	- (yes) ESC [ 1K        Erase from beginning of line to cursor
	- (yes) ESC [ 2K        Erase line containing cursor
	- (yes) ESC [ J         Erase from cursor to end of screen
	- (yes) ESC [ 0J        Same
	- (yes) ESC [ 2J        Erase entire screen

	- (?) ESC [ Ps..Ps q  Programmable LEDs: Ps are selective parameters separated by
									semicolons (073 octal) and executed in order, as follows:

									0 or None               All LEDs off
									1                       L1 On
									2                       L2 On
									3                       L3 On
									4                       L4 On

	G0 designator   G1 designator           Character set

	- (?) ESC ( A         ESC ) A                 United Kingdom (UK)
	- (?) ESC ( B         ESC ) B                 United States (USASCII)
	- (?) ESC ( 0         ESC ) 0                 Special graphics/line drawing set
	- (?) ESC ( 1         ESC ) 1                 Alternative character ROM
	- (?) ESC ( 2         ESC ) 2                 Alternative graphic ROM

	- (?) ESC K Pt;Pb r   Set top scrolling window (Pt) and bottom scrolling window
									(Pb). Pb must be greater than Pb.

	- (no) ESC H           Set tab at current column
	- (no) ESC [ g         Clear tab at current column
	- (no) ESC [ 0g        Same
	- (no) ESC [ 3g        Clear all tabs

	Modes
	-----

	The following commands are currently not supported.

	Mode Name       Mode            To set seq      Mode            To reset seq

	- (no)  Line feed/new   New line        ESC [20h        Line feed       ESC [20l
	- (no)  Cursor key      Application     ESC [?1h        Cursor          ESC [?1l
	- (no)  ANSI/VT52       ANSI            n/a             VT52            ESC [?2l
	- (no)  Column mode     132 col         ESC [?3h        80 col          ESC [?3l
	- (no)  Scrolling       Smooth          ESC [?4h        Jump            ESC [?4l
	- (no)  Screen mode     Reverse         ESC [?5h        Normal          ESC [?5l
	- (yes) Origin mode     Relative        ESC [?6h        Absolute        ESC [?6l
	- (yes) Wraparound      On              ESC [?7h        Off             ESC [?7l
	- (?)   Autorepeat      On              ESC [?8h        Off             ESC [?8l
	- (?)   Interface       On              ESC [?9h        Off             ESC [?9l

	Reports
	-------

	- (no) ESC [ 6n        Cursor position report
	- (no) ESC [ Pl;PcR            (response; Pl=line#; Pc=column#)
	- (no) ESC [ 5n        Status report
	- (no) ESC [ c                 (response; terminal Ok)
	- (no) ESC [ 0c                (response; teminal not Ok)
	- (no) ESC [ c         What are you?
	- (no) ESC [ 0c        Same
	- (no) ESC [?1;Ps c            response; where Ps is option present:

													0               Base VT100, no options
													1               Preprocessor option (STP)
													2               Advanced video option (AVO)
													3               AVO and STP
													4               Graphics processor option (GO)
													5               GO and STP
													6               GO and AVO
													7               GO, STP, and AVO

	- (no) ESC c           Causes power-up reset routine to be executed
	- (no) ESC #8          Fill screen with "E"
	- (no) ESC [ 2;Ps y    Invoke Test(s), where Ps is a decimal computed by adding the
									numbers of the desired tests to be executed:

													1               Power up test
													2               Data loop back
													4               EIA modem control signal test
													8               Repeat test(s) indefinitely


	 

	TERMINAL COMMANDS
	----------------

	- (yes) ESC c		Reset
	- (no)  [ ! p		Soft Reset
	- (yes) # 8		Fill Screen with E's
	- (no)  } 1 * 		Fill screen with * test
	- (no)  } 2		Video attribute test display
	- (no)  } 3		Character sets display test

	KEYBOARD COMMANDS
	-----------------

	- (?) [ 2 h		Keyboard locked
	- (?) [ 2 l		Keyboard unlocked
	- (?) [ ? 8 h		Autorepeat ON
	- (?) [ ? 8 l		Autorepeat OFF
	- (?) [ 0 q		Lights all off on keyboard
	- (?) [ * q		Light * on

	PROGRAMMABLE KEY COMMANDS
	-------------------------

	- (?) ! pk		Program a programmable key (local)
	- (?) @ pk		Program a programmable key (on-line)
	- (?) % pk		Transmit programmable key contents

	SCREEN FORMAT (not supported)
	-------------

	- (?) [ ? 3h		132 Characters on
	- (?) [ ? 3l		80 Characters on
	- (?) [ ? 4h		Smooth Scroll on
	- (?) [ ? 4l		Jump Scroll on
	- (?) [ *t ; *b r	Scrolling region selected, line *t to *b
	- (?) [ ? 5 h		Inverse video on
	- (?) [ ? 5 l		Normal video off
	- (?) [ ? 7 h		Wraparound ON
	- (?) [ ? 7 l		Wraparound OFF
	- (?) [ ? 75 h	Screen display ON
	- (?) [ ? 75 l	Screen display OFF

	CHARACTER SETS AND LABELS
	-------------------------

	- (?) ( A		British 
	- (?) ( B		North American ASCII set
	- (?) ( C		Finnish
	- (?) ( E		Danish or Norwegian
	- (?) ( H		Swedish
	- (?) ( K		German
	- (?) ( Q		French Canadian
	- (?) ( R		Flemish or French/Belgian
	- (?) ( Y		Italian
	- (?) ( Z		Spanish
	- (?) ( 0		Line Drawing
	- (?) ( 1		Alternative Character
	- (?) ( 2		Alternative Line drawing
	- (?) ( 4		Dutch
	- (?) ( 5		Finnish
	- (?) ( 6		Danish or Norwegian
	- (?) ( 7		Swedish
	- (?) ( =		Swiss (French or German)

	[Note all ( may be replaced with )]


	ATTRIBUTES AND FIELDS
	-------

	- (?) [ 0 m		Clear all character attributes
	- (?) [ 1 m		Alternate Intensity ON
	- (?) [ 4 m		Underline ON
	- (?) [ 5 m		Blink ON
	- (?) [ 7 m		Inverse video ON
	- (?) [ 22 m		Alternate Intensity OFF
	- (?) [ 24 m		Underline OFF
	- (?) [ 25 m		Blink OFF
	- (?) [ 27 m		Inverse Video OFF
	- (?) [ 0 }		Protected fields OFF
	- (?) [ 1 } 		Protected = Alternate Intensity
	- (?) [ 4 }		Protected = Underline
	- (?) [ 5 }		Protected = Blinking
	- (?) [ 7 }		Protected = Inverse
	- (?) [ 254 }		Protected = All attributes OFF

	CURSOR COMMANDS
	-------

	- (?) [ ? 25 l	Cursor OFF
	- (?) [ ? 25 h	Cursor ON
	- (?) [ ? 50 l	Cursor OFF
	- (?) [ ? 50 h	Cursor ON
	- (yes) 7		Save cursor position and character attributes
	- (yes) 8		Restore cursor position and character attributes
	- (yes) D		Line feed
	- (yes) E		Carriage return and line feed
	- (yes) M		Reverse Line feed
	- (yes) [ A		Cursor up one line
	- (yes) [ B		Cursor down one line
	- (yes) [ C		Cursor right one column
	- (yes) [ D		Cursor left one column
	- (yes) [ * A		Cursor up * lines
	- (yes) [ * B		Cursor down * lines
	- (yes) [ * C		Cursor right * columns
	- (yes) [ * D		Cursor left * columns
	- (yes) [ H		Cursor home
	- (yes) [ *l ; *c H	Move cursor to line *l, column *c
	- (yes) [ *l ; *c f	Move curosr to line *l, column *c
	- (no) Y nl nc 	Direct cursor addressing (line/column number)
	- (no) H		Tab set at present cursor position
	- (no) [ 0 g		Clear tab at present cursor position
	- (no) [ 3 g		Clear all tabs

	EDIT COMMANDS
	-------

	- (no) [ 4 h		Insert mode selected
	- (no) [ 4 l		Replacement mode selected
	- (no) [ ? 14 h	Immediate operation of ENTER key
	- (no) [ ? 14 l	Deferred operation of ENTER key
	- (no) [ ? 16 h	Edit selection immediate
	- (no) [ ? 16 l	Edit selection deffered
	- (no) [ P		Delete character from cursor position
	- (no) [ * P		Delete * chars from curosr right
	- (no) [ M		Delete 1 char from cursor position
	- (no) [ * M		Delete * lines from cursor line down
	- (yes) [ J		Erase screen from cursor to end
	- (yes) [ 1 J		Erase beginning of screen to cursor
	- (yes) [ 2 J		Erase entire screen but do not move cursor
	- (yes) [ K		Erase line from cursor to end
	- (yes) [ 1 K		Erase from beginning of line to cursor
	- (yes) [ 2 K		Erase entire line but do not move cursor
	- (no) [ L		Insert 1 line from cursor position
	- (no) [ * L		Insert * lines from cursor position

	LICENSE
	-------

	- Multilicense: GNU GPL free for all, by default.
		For anything else, drop me an email. :)

	 .............................................................................
	 : INTERNET: info@fortmax.se :: Tel: 0733-387694 Int: +46-733-387694         :
	 : AUTHOR: Martin K. Schröder COPYRIGHT: 2014 SWEDEN 												 :
	 :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::



